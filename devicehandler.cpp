#include "devicehandler.h"

// Qt includes
#include <QtEndian>
#include <QRandomGenerator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

// local includes
#include "deviceinfo.h"

namespace {
const QBluetoothUuid bobbycarServiceUuid{QUuid::fromString(QStringLiteral("0335e46c-f355-4ce6-8076-017de08cee98"))};

const QBluetoothUuid livestatsCharacUuid{QUuid::fromString(QStringLiteral("a48321ea-329f-4eab-a401-30e247211524"))};
const QBluetoothUuid remotecontrolCharacUuid{QUuid::fromString(QStringLiteral("4201def0-a264-43e6-946b-6b2d9612dfed"))};
}

DeviceHandler::DeviceHandler(QObject *parent) :
    BluetoothBaseClass(parent),
    m_foundBobbycarService(false)
{
}

void DeviceHandler::setAddressType(AddressType type)
{
    switch (type) {
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

void DeviceHandler::setDevice(DeviceInfo *device)
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
    if (m_currentDevice)
    {
        // Make connections
        //! [Connect-Signals-1]
        m_control = QLowEnergyController::createCentral(m_currentDevice->getDevice(), this);
        //! [Connect-Signals-1]
        m_control->setRemoteAddressType(m_addressType);
        //! [Connect-Signals-2]
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
        //! [Connect-Signals-2]
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
        m_service->discoverDetails();
    }
    else
    {
        setError("Bobbycar Service not found.");
    }
}

namespace {
void logAddr(const QBluetoothUuid &uuid)
{
    if (uuid == bobbycarServiceUuid)
        qDebug() << "bobbycarServiceUuid";
    else if (uuid == livestatsCharacUuid)
        qDebug() << "livestatsCharacUuid";
    else if (uuid == remotecontrolCharacUuid)
        qDebug() << "remotecontrolCharacUuid";
    else
        qDebug() << "unknown uuid" << uuid;
}
}

void DeviceHandler::serviceStateChanged(QLowEnergyService::ServiceState s)
{
    switch (s) {
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

bool DeviceHandler::alive() const
{
    if (m_service)
        return m_service->state() == QLowEnergyService::ServiceDiscovered;

    return false;
}
