#include "devicehandler.h"

// Qt includes
#include <QtEndian>
#include <QRandomGenerator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimerEvent>

// local includes
#include "deviceinfo.h"

namespace {
const QBluetoothUuid bobbycarServiceUuid{QUuid::fromString(QStringLiteral("0335e46c-f355-4ce6-8076-017de08cee98"))};

const QBluetoothUuid livestatsCharacUuid{QUuid::fromString(QStringLiteral("a48321ea-329f-4eab-a401-30e247211524"))};
const QBluetoothUuid remotecontrolCharacUuid{QUuid::fromString(QStringLiteral("4201def0-a264-43e6-946b-6b2d9612dfed"))};

const QBluetoothUuid settingsSetterUuid{QUuid::fromString(QStringLiteral("4201def1-a264-43e6-946b-6b2d9612dfed"))};
const QBluetoothUuid wifiListUuid{QUuid::fromString(QStringLiteral("4201def2-a264-43e6-946b-6b2d9612dfed"))};
}

DeviceHandler::DeviceHandler(QObject *parent) :
    BluetoothBaseClass(parent),
    m_foundBobbycarService(false)
{
}

void DeviceHandler::setDevice(const QBluetoothDeviceInfo &device)
{
    clearMessages();
    m_currentDevice = device;

    // Disconnect and delete old connection
    if (m_control)
    {
        m_control->disconnectFromDevice();
        delete m_control;
        m_control = nullptr;
    }

    // Create new controller and connect it if device available
    if (m_currentDevice.isValid())
    {
        // Make connections
        m_control = QLowEnergyController::createCentral(m_currentDevice, this);
        m_control->setRemoteAddressType(m_addressType);

        connect(m_control, &QLowEnergyController::serviceDiscovered,
                this, &DeviceHandler::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished,
                this, &DeviceHandler::serviceScanDone);

        connect(m_control, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, [this](QLowEnergyController::Error error) {
            Q_UNUSED(error);
            setError("Cannot connect to remote device.");
        });
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            setInfo("Controller connected. Search services...");
            m_control->discoverServices();
        });
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            setError("LowEnergy controller disconnected");
        });

        // Connect
        m_control->connectToDevice();
    }
}

void DeviceHandler::setAddressType(AddressType type)
{
    switch (type)
    {
    case DeviceHandler::AddressType::PublicAddress:
        m_addressType = QLowEnergyController::PublicAddress;
        break;
    case DeviceHandler::AddressType::RandomAddress:
        m_addressType = QLowEnergyController::RandomAddress;
        break;
    }
}

DeviceHandler::AddressType DeviceHandler::addressType() const
{
    if (m_addressType == QLowEnergyController::RandomAddress)
        return DeviceHandler::AddressType::RandomAddress;

    return DeviceHandler::AddressType::PublicAddress;
}

bool DeviceHandler::alive() const
{
    if (m_service)
        return m_service->state() == QLowEnergyService::ServiceDiscovered;

    return false;
}

void DeviceHandler::setRemoteControlActive(bool remoteControlActive)
{
    if (!remoteControlActive && m_timerId != -1)
    {
        killTimer(m_timerId);
        m_timerId = -1;
        emit remoteControlActiveChanged();

        if (m_service && m_remotecontrolCharacteristic.isValid())
        {
            m_remoteControlFrontLeft = 0;
            m_remoteControlFrontRight = 0;
            m_remoteControlBackLeft = 0;
            m_remoteControlBackRight = 0;

            sendRemoteControl();
        }
    }
    else if (remoteControlActive && m_timerId == -1 && m_service && m_remotecontrolCharacteristic.isValid())
    {
        m_timerId = startTimer(100);
        emit remoteControlActiveChanged();
    }
}

void DeviceHandler::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timerId)
    {
        if (!m_service || !m_remotecontrolCharacteristic.isValid())
        {
            killTimer(m_timerId);
            m_timerId = -1;
            emit remoteControlActiveChanged();
            return;
        }

        if (m_waitingForWrite)
            qWarning() << "still pending";
        else
            sendRemoteControl();
    }
    else
        BluetoothBaseClass::timerEvent(event);
}

void DeviceHandler::disconnectService()
{
    m_foundBobbycarService = false;

    //disable notifications
    if (m_service)
    {
        if (m_notificationDescLivestats.isValid() && m_notificationDescLivestats.value() == QByteArray::fromHex("0100"))
            m_service->writeDescriptor(m_notificationDescLivestats, QByteArray::fromHex("0000"));

        disconnectInternal();
    }
}

void DeviceHandler::disconnectInternal()
{
    if (!m_notificationDescLivestats.isValid())
    {
        //disabled notifications -> assume disconnect intent
        if (m_control)
            m_control->disconnectFromDevice();

        if (m_service)
        {
            delete m_service;
            m_service = nullptr;
        }
    }
}

void DeviceHandler::serviceDiscovered(const QBluetoothUuid &gatt)
{
    if (gatt == bobbycarServiceUuid)
    {
        setInfo("Bobbycar service discovered. Waiting for service scan to be done...");
        m_foundBobbycarService = true;
    }
}

void DeviceHandler::serviceScanDone()
{
    setInfo("Service scan done.");

    // Delete old service if available
    if (m_service)
    {
        delete m_service;
        m_service = nullptr;
    }

    // If bobbycarService found, create new service
    if (m_foundBobbycarService)
        m_service = m_control->createServiceObject(bobbycarServiceUuid, this);

    if (m_service)
    {
        connect(m_service, &QLowEnergyService::stateChanged, this, &DeviceHandler::serviceStateChanged);
        connect(m_service, &QLowEnergyService::characteristicChanged, this, &DeviceHandler::updateBobbycarValue);
        connect(m_service, &QLowEnergyService::descriptorWritten, this, &DeviceHandler::confirmedDescriptorWrite);
        connect(m_service, &QLowEnergyService::characteristicWritten, this, &DeviceHandler::confirmedCharacteristicWrite);
        m_service->discoverDetails();
    }
    else
    {
        setError("Bobbycar Service not found.");
    }
}

void DeviceHandler::serviceStateChanged(QLowEnergyService::ServiceState s)
{
    qDebug() << "serviceStateChanged()" << s;

    if (m_timerId != -1)
    {
        killTimer(m_timerId);
        m_timerId = -1;
        emit remoteControlActiveChanged();
    }

    m_waitingForWrite = false;

    switch (s)
    {
    case QLowEnergyService::DiscoveringServices:
        setInfo(tr("Discovering services..."));
        break;
    case QLowEnergyService::ServiceDiscovered:
    {
        setInfo(tr("Service discovered."));

        if (const QLowEnergyCharacteristic hrChar = m_service->characteristic(livestatsCharacUuid); hrChar.isValid())
        {
            m_notificationDescLivestats = hrChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            if (m_notificationDescLivestats.isValid())
                m_service->writeDescriptor(m_notificationDescLivestats, QByteArray::fromHex("0100"));
        }
        else
        {
            setError("livestatsCharacUuid not found.");
            break;
        }

        m_remotecontrolCharacteristic = m_service->characteristic(remotecontrolCharacUuid);
        if (!m_remotecontrolCharacteristic.isValid())
        {
            setError("remotecontrolCharacUuid not found.");
            break;
        }

        break;
    }
    default:
        //nothing for now
        break;
    }

    emit aliveChanged();
}

void DeviceHandler::updateBobbycarValue(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    //qDebug() << "updateBobbycarValue";
    //logAddr(c.uuid());

    if (c.uuid() == livestatsCharacUuid)
    {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(value, &error);
        if (error.error != QJsonParseError::NoError)
        {
            qWarning() << "could not parse livestats" << error.errorString();
            return;
        }

        clearMessages();

        const QJsonObject &obj = doc.object();

        {
            const QJsonArray &arr = obj.value("v").toArray();
            m_frontVoltage = arr.at(0).toDouble();
            emit frontVoltageChanged();
            m_backVoltage = arr.at(1).toDouble();
            emit backVoltageChanged();
        }
        {
            const QJsonArray &arr = obj.value("t").toArray();
            m_frontTemperature = arr.at(0).toDouble();
            emit frontTemperatureChanged();
            m_backTemperature = arr.at(1).toDouble();
            emit backTemperatureChanged();
        }
        {
            const QJsonArray &arr = obj.value("e").toArray();
            m_frontLeftError = arr.at(0).toInt();
            emit frontLeftErrorChanged();
            m_frontRightError = arr.at(1).toInt();
            emit frontRightErrorChanged();
            m_backLeftError = arr.at(2).toInt();
            emit backLeftErrorChanged();
            m_backRightError = arr.at(3).toInt();
            emit backRightErrorChanged();
        }
        {
            const QJsonArray &arr = obj.value("s").toArray();
            m_frontLeftSpeed = arr.at(0).toDouble();
            emit frontLeftSpeedChanged();
            m_frontRightSpeed = arr.at(1).toDouble();
            emit frontRightSpeedChanged();
            m_backLeftSpeed = arr.at(2).toDouble();
            emit backLeftSpeedChanged();
            m_backRightSpeed = arr.at(3).toDouble();
            emit backRightSpeedChanged();
        }
        {
            const QJsonArray &arr = obj.value("a").toArray();
            m_frontLeftDcLink = arr.at(0).toDouble();
            emit frontLeftDcLinkChanged();
            m_frontRightDcLink = arr.at(1).toDouble();
            emit frontRightDcLinkChanged();
            m_backLeftDcLink = arr.at(2).toDouble();
            emit backLeftDcLinkChanged();
            m_backRightDcLink = arr.at(3).toDouble();
            emit backRightDcLinkChanged();
        }
    }
    else
        qWarning() << "unknown uuid" << c.uuid();
}

void DeviceHandler::confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value)
{
    qDebug() << "confirmedDescriptorWrite" << d.uuid() << value;
    if (d.isValid() && value == QByteArray::fromHex("0000"))
    {
        if (d == m_notificationDescLivestats)
            m_notificationDescLivestats = {};

        disconnectInternal();
    }
}

void DeviceHandler::confirmedCharacteristicWrite(const QLowEnergyCharacteristic &info, const QByteArray &value)
{
    Q_UNUSED(value)
    qDebug() << "confirmedCharacteristicWrite";

    if (info == m_remotecontrolCharacteristic)
        m_waitingForWrite = false;
}

void DeviceHandler::sendRemoteControl()
{
    m_waitingForWrite = true;

    qDebug() << "writeCharacteristic()";
    m_service->writeCharacteristic(m_remotecontrolCharacteristic, QJsonDocument{QJsonObject {
        {"fl", m_remoteControlFrontLeft},
        {"fr", m_remoteControlFrontRight},
        {"bl", m_remoteControlBackLeft},
        {"br", m_remoteControlBackRight}
    }}.toJson(QJsonDocument::Compact));
}
