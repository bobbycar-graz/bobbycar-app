#include "devicehandler.h"
#include "deviceinfo.h"
#include <QtEndian>
#include <QRandomGenerator>

namespace {
const QBluetoothUuid bobbycarServiceUuid{QUuid::fromString(QStringLiteral("0335e46c-f355-4ce6-8076-017de08cee98"))};

const QBluetoothUuid frontVoltageCharacUuid{QUuid::fromString(QStringLiteral("a48321ea-329f-4eab-a401-30e247211524"))};
const QBluetoothUuid backVoltageCharacUuid{QUuid::fromString(QStringLiteral("4201def0-a264-43e6-946b-6b2d9612dfed"))};

const QBluetoothUuid frontTemperatureCharacUuid{QUuid::fromString(QStringLiteral("4799e23f-6448-4786-900b-b5c3f3c17a9c"))};
const QBluetoothUuid backTemperatureCharacUuid{QUuid::fromString(QStringLiteral("3c32b7bb-8d9b-4055-8ea0-5b6764111024"))};

const QBluetoothUuid frontLeftErrorCharacUuid{QUuid::fromString(QStringLiteral("f84b3a9b-1b2c-4075-acbe-016a2166976c"))};
const QBluetoothUuid frontRightErrorCharacUuid{QUuid::fromString(QStringLiteral("eed4b709-5a65-4a5b-8e07-512f9661533d"))};
const QBluetoothUuid backLeftErrorCharacUuid{QUuid::fromString(QStringLiteral("89d143f5-9ae2-4f7e-9235-643a3a7e21df"))};
const QBluetoothUuid backRightErrorCharacUuid{QUuid::fromString(QStringLiteral("0fb377f1-7527-4966-aaf0-8bd56f2ddd3f"))};

const QBluetoothUuid frontLeftSpeedCharacUuid{QUuid::fromString(QStringLiteral("c6f959e8-0ec3-4bdd-88ad-6ad993fc81e9"))};
const QBluetoothUuid frontRightSpeedCharacUuid{QUuid::fromString(QStringLiteral("ce53f135-8f20-4b80-abb9-31da81d62716"))};
const QBluetoothUuid backLeftSpeedCharacUuid{QUuid::fromString(QStringLiteral("9a1dd1fe-3f14-4af1-bc5e-3f70edcae54b"))};
const QBluetoothUuid backRightSpeedCharacUuid{QUuid::fromString(QStringLiteral("7de1a823-682e-438f-9201-3a80c3911f1a"))};

const QBluetoothUuid frontLeftDcLinkCharacUuid{QUuid::fromString(QStringLiteral("f404416f-2a77-41c6-a35f-7d10ec38376d"))};
const QBluetoothUuid frontRightDcLinkCharacUuid{QUuid::fromString(QStringLiteral("452dd012-3f12-428c-8746-40c6b6c73c40"))};
const QBluetoothUuid backLeftDcLinkCharacUuid{QUuid::fromString(QStringLiteral("9dc455a3-718e-4d62-b0e7-1c0cb2a8bbd3"))};
const QBluetoothUuid backRightDcLinkCharacUuid{QUuid::fromString(QStringLiteral("90a66506-1d78-4ba2-b074-e1153fbf5216"))};
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
    if (m_control) {
        m_control->disconnectFromDevice();
        delete m_control;
        m_control = nullptr;
    }

    // Create new controller and connect it if device available
    if (m_currentDevice) {

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
    if (gatt == bobbycarServiceUuid) {
        setInfo("Bobbycar service discovered. Waiting for service scan to be done...");
        m_foundBobbycarService = true;
    }
}

void DeviceHandler::serviceScanDone()
{
    setInfo("Service scan done.");

    // Delete old service if available
    if (m_service) {
        delete m_service;
        m_service = nullptr;
    }

    // If bobbycarService found, create new service
    if (m_foundBobbycarService)
        m_service = m_control->createServiceObject(bobbycarServiceUuid, this);

    if (m_service) {
        connect(m_service, &QLowEnergyService::stateChanged, this, &DeviceHandler::serviceStateChanged);
        connect(m_service, &QLowEnergyService::characteristicChanged, this, &DeviceHandler::updateBobbycarValue);
        connect(m_service, &QLowEnergyService::descriptorWritten, this, &DeviceHandler::confirmedDescriptorWrite);
        m_service->discoverDetails();
    } else {
        setError("Bobbycar Service not found.");
    }
}

namespace {
void logAddr(const QBluetoothUuid &uuid)
{
    if (uuid == bobbycarServiceUuid)
        qDebug() << "bobbycarServiceUuid";
    else if (uuid == frontVoltageCharacUuid)
        qDebug() << "frontVoltageCharacUuid";
    else if (uuid == backVoltageCharacUuid)
        qDebug() << "backVoltageCharacUuid";
    else if (uuid == frontTemperatureCharacUuid)
        qDebug() << "frontTemperatureCharacUuid";
    else if (uuid == backTemperatureCharacUuid)
        qDebug() << "backTemperatureCharacUuid";
    else if (uuid == frontLeftErrorCharacUuid)
        qDebug() << "frontLeftErrorCharacUuid";
    else if (uuid == frontRightErrorCharacUuid)
        qDebug() << "frontRightErrorCharacUuid";
    else if (uuid == backLeftErrorCharacUuid)
        qDebug() << "backLeftErrorCharacUuid";
    else if (uuid == backRightErrorCharacUuid)
        qDebug() << "backRightErrorCharacUuid";
    else if (uuid == frontLeftSpeedCharacUuid)
        qDebug() << "frontLeftSpeedCharacUuid";
    else if (uuid == frontRightSpeedCharacUuid)
        qDebug() << "frontRightSpeedCharacUuid";
    else if (uuid == backLeftSpeedCharacUuid)
        qDebug() << "backLeftSpeedCharacUuid";
    else if (uuid == backRightSpeedCharacUuid)
        qDebug() << "backRightSpeedCharacUuid";
    else if (uuid == frontLeftDcLinkCharacUuid)
        qDebug() << "frontLeftDcLinkCharacUuid";
    else if (uuid == frontRightDcLinkCharacUuid)
        qDebug() << "frontRightDcLinkCharacUuid";
    else if (uuid == backLeftDcLinkCharacUuid)
        qDebug() << "backLeftDcLinkCharacUuid";
    else if (uuid == backRightDcLinkCharacUuid)
        qDebug() << "backRightDcLinkCharacUuid";
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

        if (const QLowEnergyCharacteristic hrChar = m_service->characteristic(frontVoltageCharacUuid); hrChar.isValid())
        {
            m_notificationDescFrontVoltage = hrChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            if (m_notificationDescFrontVoltage.isValid())
                m_service->writeDescriptor(m_notificationDescFrontVoltage, QByteArray::fromHex("0100"));
        }
        else
        {
            setError("frontVoltageCharacUuid not found.");
            break;
        }

        if (const QLowEnergyCharacteristic hrChar = m_service->characteristic(backVoltageCharacUuid); hrChar.isValid())
        {
            m_notificationDescBackVoltage = hrChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            if (m_notificationDescBackVoltage.isValid())
                m_service->writeDescriptor(m_notificationDescBackVoltage, QByteArray::fromHex("0100"));
        }
        else
        {
            setError("backVoltageCharacUuid not found.");
            break;
        }

        if (const QLowEnergyCharacteristic hrChar = m_service->characteristic(frontTemperatureCharacUuid); hrChar.isValid())
        {
            m_notificationDescFrontTemperature = hrChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            if (m_notificationDescFrontTemperature.isValid())
                m_service->writeDescriptor(m_notificationDescFrontTemperature, QByteArray::fromHex("0100"));
        }
        else
        {
            setError("frontTemperatureCharacUuid not found.");
            break;
        }

        if (const QLowEnergyCharacteristic hrChar = m_service->characteristic(backTemperatureCharacUuid); hrChar.isValid())
        {
            m_notificationDescBackTemperature = hrChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            if (m_notificationDescBackTemperature.isValid())
                m_service->writeDescriptor(m_notificationDescBackTemperature, QByteArray::fromHex("0100"));
        }
        else
        {
            setError("backTemperatureCharacUuid not found.");
            break;
        }

        if (const QLowEnergyCharacteristic hrChar = m_service->characteristic(frontLeftErrorCharacUuid); hrChar.isValid())
        {
            m_notificationDescFrontLeftError = hrChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            if (m_notificationDescFrontLeftError.isValid())
                m_service->writeDescriptor(m_notificationDescFrontLeftError, QByteArray::fromHex("0100"));
        }
        else
        {
            setError("frontLeftErrorCharacUuid not found.");
            break;
        }

        if (const QLowEnergyCharacteristic hrChar = m_service->characteristic(frontRightErrorCharacUuid); hrChar.isValid())
        {
            m_notificationDescFrontRightError = hrChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            if (m_notificationDescFrontRightError.isValid())
                m_service->writeDescriptor(m_notificationDescFrontRightError, QByteArray::fromHex("0100"));
        }
        else
        {
            setError("frontRightErrorCharacUuid not found.");
            break;
        }

        if (const QLowEnergyCharacteristic hrChar = m_service->characteristic(backLeftErrorCharacUuid); hrChar.isValid())
        {
            m_notificationDescBackLeftError = hrChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            if (m_notificationDescBackLeftError.isValid())
                m_service->writeDescriptor(m_notificationDescBackLeftError, QByteArray::fromHex("0100"));
        }
        else
        {
            setError("backLeftErrorCharacUuid not found.");
            break;
        }

        if (const QLowEnergyCharacteristic hrChar = m_service->characteristic(backRightErrorCharacUuid); hrChar.isValid())
        {
            m_notificationDescBackRightError = hrChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            if (m_notificationDescBackRightError.isValid())
                m_service->writeDescriptor(m_notificationDescBackRightError, QByteArray::fromHex("0100"));
        }
        else
        {
            setError("backRightErrorCharacUuid not found.");
            break;
        }

        if (const QLowEnergyCharacteristic hrChar = m_service->characteristic(frontLeftSpeedCharacUuid); hrChar.isValid())
        {
            m_notificationDescFrontLeftSpeed = hrChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            if (m_notificationDescFrontLeftSpeed.isValid())
                m_service->writeDescriptor(m_notificationDescFrontLeftSpeed, QByteArray::fromHex("0100"));
        }
        else
        {
            setError("frontLeftSpeedCharacUuid not found.");
            break;
        }

        if (const QLowEnergyCharacteristic hrChar = m_service->characteristic(frontRightSpeedCharacUuid); hrChar.isValid())
        {
            m_notificationDescFrontRightSpeed = hrChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            if (m_notificationDescFrontRightSpeed.isValid())
                m_service->writeDescriptor(m_notificationDescFrontRightSpeed, QByteArray::fromHex("0100"));
        }
        else
        {
            setError("frontRightSpeedCharacUuid not found.");
            break;
        }

        if (const QLowEnergyCharacteristic hrChar = m_service->characteristic(backLeftSpeedCharacUuid); hrChar.isValid())
        {
            m_notificationDescBackLeftSpeed = hrChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            if (m_notificationDescBackLeftSpeed.isValid())
                m_service->writeDescriptor(m_notificationDescBackLeftSpeed, QByteArray::fromHex("0100"));
        }
        else
        {
            setError("backLeftSpeedCharacUuid not found.");
            break;
        }

        if (const QLowEnergyCharacteristic hrChar = m_service->characteristic(backRightSpeedCharacUuid); hrChar.isValid())
        {
            m_notificationDescBackRightSpeed = hrChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            if (m_notificationDescBackRightSpeed.isValid())
                m_service->writeDescriptor(m_notificationDescBackRightSpeed, QByteArray::fromHex("0100"));
        }
        else
        {
            setError("backRightSpeedCharacUuid not found.");
            break;
        }

        if (const QLowEnergyCharacteristic hrChar = m_service->characteristic(frontLeftDcLinkCharacUuid); hrChar.isValid())
        {
            m_notificationDescFrontLeftDcLink = hrChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            if (m_notificationDescFrontLeftDcLink.isValid())
                m_service->writeDescriptor(m_notificationDescFrontLeftDcLink, QByteArray::fromHex("0100"));
        }
        else
        {
            setError("frontLeftDcLinkCharacUuid not found.");
            break;
        }

        if (const QLowEnergyCharacteristic hrChar = m_service->characteristic(frontRightDcLinkCharacUuid); hrChar.isValid())
        {
            m_notificationDescFrontRightDcLink = hrChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            if (m_notificationDescFrontRightDcLink.isValid())
                m_service->writeDescriptor(m_notificationDescFrontRightDcLink, QByteArray::fromHex("0100"));
        }
        else
        {
            setError("frontRightDcLinkCharacUuid not found.");
            break;
        }

        if (const QLowEnergyCharacteristic hrChar = m_service->characteristic(backLeftDcLinkCharacUuid); hrChar.isValid())
        {
            m_notificationDescBackLeftDcLink = hrChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            if (m_notificationDescBackLeftDcLink.isValid())
                m_service->writeDescriptor(m_notificationDescBackLeftDcLink, QByteArray::fromHex("0100"));
        }
        else
        {
            setError("backLeftDcLinkCharacUuid not found.");
            break;
        }

        if (const QLowEnergyCharacteristic hrChar = m_service->characteristic(backRightDcLinkCharacUuid); hrChar.isValid())
        {
            m_notificationDescBackRightDcLink = hrChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            if (m_notificationDescBackRightDcLink.isValid())
                m_service->writeDescriptor(m_notificationDescBackRightDcLink, QByteArray::fromHex("0100"));
        }
        else
        {
            setError("backRightDcLinkCharacUuid not found.");
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

    if (c.uuid() == frontVoltageCharacUuid)
    {
        bool ok;
        float val = value.toFloat(&ok);
        if (ok)
        {
            m_frontVoltage = val;
            emit frontVoltageChanged();
        }
        else if (!value.isEmpty())
            qWarning() << "could not parse frontVoltage" << value;
    }
    else if (c.uuid() == backVoltageCharacUuid)
    {
        bool ok;
        float val = value.toFloat(&ok);
        if (ok)
        {
            m_backVoltage = val;
            emit backVoltageChanged();
        }
        else if (!value.isEmpty())
            qWarning() << "could not parse backVoltage" << value;
    }
    else if (c.uuid() == frontTemperatureCharacUuid)
    {
        bool ok;
        float val = value.toFloat(&ok);
        if (ok)
        {
            m_frontTemperature = val;
            emit frontTemperatureChanged();
        }
        else if (!value.isEmpty())
            qWarning() << "could not parse frontTemperature" << value;
    }
    else if (c.uuid() == backTemperatureCharacUuid)
    {
        bool ok;
        float val = value.toFloat(&ok);
        if (ok)
        {
            m_backTemperature = val;
            emit backTemperatureChanged();
        }
        else if (!value.isEmpty())
            qWarning() << "could not parse backTemperature" << value;
    }
    else if (c.uuid() == frontLeftErrorCharacUuid)
    {
        bool ok;
        uint8_t val = value.toShort(&ok);
        if (ok)
        {
            m_frontLeftError = val;
            emit frontLeftErrorChanged();
        }
        else if (!value.isEmpty())
            qWarning() << "could not parse frontLeftError" << value;
    }
    else if (c.uuid() == frontRightErrorCharacUuid)
    {
        bool ok;
        uint8_t val = value.toShort(&ok);
        if (ok)
        {
            m_frontRightError = val;
            emit frontRightErrorChanged();
        }
        else if (!value.isEmpty())
            qWarning() << "could not parse frontRightError" << value;
    }
    else if (c.uuid() == backLeftErrorCharacUuid)
    {
        bool ok;
        uint8_t val = value.toShort(&ok);
        if (ok)
        {
            m_backLeftError = val;
            emit backLeftErrorChanged();
        }
        else if (!value.isEmpty())
            qWarning() << "could not parse backLeftError" << value;
    }
    else if (c.uuid() == backRightErrorCharacUuid)
    {
        bool ok;
        uint8_t val = value.toShort(&ok);
        if (ok)
        {
            m_backRightError = val;
            emit backRightErrorChanged();
        }
        else if (!value.isEmpty())
            qWarning() << "could not parse backRightError" << value;
    }
    else if (c.uuid() == frontLeftSpeedCharacUuid)
    {
        bool ok;
        float val = value.toFloat(&ok);
        if (ok)
        {
            m_frontLeftSpeed = val;
            emit frontLeftSpeedChanged();
        }
        else if (!value.isEmpty())
            qWarning() << "could not parse frontLeftSpeed" << value;
    }
    else if (c.uuid() == frontRightSpeedCharacUuid)
    {
        bool ok;
        float val = value.toFloat(&ok);
        if (ok)
        {
            m_frontRightSpeed = val;
            emit frontRightSpeedChanged();
        }
        else if (!value.isEmpty())
            qWarning() << "could not parse frontRightSpeed" << value;
    }
    else if (c.uuid() == backLeftSpeedCharacUuid)
    {
        bool ok;
        float val = value.toFloat(&ok);
        if (ok)
        {
            m_backLeftSpeed = val;
            emit backLeftSpeedChanged();
        }
        else if (!value.isEmpty())
            qWarning() << "could not parse backLeftSpeed" << value;
    }
    else if (c.uuid() == backRightSpeedCharacUuid)
    {
        bool ok;
        float val = value.toFloat(&ok);
        if (ok)
        {
            m_backRightSpeed = val;
            emit backRightSpeedChanged();
        }
        else if (!value.isEmpty())
            qWarning() << "could not parse backRightSpeed" << value;
    }
    else if (c.uuid() == frontLeftDcLinkCharacUuid)
    {
        bool ok;
        float val = value.toFloat(&ok);
        if (ok)
        {
            m_frontLeftDcLink = val;
            emit frontLeftDcLinkChanged();
        }
        else if (!value.isEmpty())
            qWarning() << "could not parse frontLeftDcLink" << value;
    }
    else if (c.uuid() == frontRightDcLinkCharacUuid)
    {
        bool ok;
        float val = value.toFloat(&ok);
        if (ok)
        {
            m_frontRightDcLink = val;
            emit frontRightDcLinkChanged();
        }
        else if (!value.isEmpty())
            qWarning() << "could not parse frontRightDcLink" << value;
    }
    else if (c.uuid() == backLeftDcLinkCharacUuid)
    {
        bool ok;
        float val = value.toFloat(&ok);
        if (ok)
        {
            m_backLeftDcLink = val;
            emit backLeftDcLinkChanged();
        }
        else if (!value.isEmpty())
            qWarning() << "could not parse backLeftDcLink" << value;
    }
    else if (c.uuid() == backRightDcLinkCharacUuid)
    {
        bool ok;
        float val = value.toFloat(&ok);
        if (ok)
        {
            m_backRightDcLink = val;
            emit backRightDcLinkChanged();
        }
        else if (!value.isEmpty())
            qWarning() << "could not parse backRightDcLink" << value;
    }
}

void DeviceHandler::confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value)
{
    qDebug() << "confirmedDescriptorWrite" << d.uuid() << value;
    if (d.isValid() && value == QByteArray::fromHex("0000"))
    {
        if (d == m_notificationDescFrontVoltage)
            m_notificationDescFrontVoltage = {};
        if (d == m_notificationDescBackVoltage)
            m_notificationDescBackVoltage = {};
        if (d == m_notificationDescFrontTemperature)
            m_notificationDescFrontTemperature = {};
        if (d == m_notificationDescBackTemperature)
            m_notificationDescBackTemperature = {};
        if (d == m_notificationDescFrontLeftError)
            m_notificationDescFrontLeftError = {};
        if (d == m_notificationDescFrontRightError)
            m_notificationDescFrontRightError = {};
        if (d == m_notificationDescBackLeftError)
            m_notificationDescBackLeftError = {};
        if (d == m_notificationDescBackRightError)
            m_notificationDescBackRightError = {};
        if (d == m_notificationDescFrontLeftSpeed)
            m_notificationDescFrontLeftSpeed = {};
        if (d == m_notificationDescFrontRightSpeed)
            m_notificationDescFrontRightSpeed = {};
        if (d == m_notificationDescBackLeftSpeed)
            m_notificationDescBackLeftSpeed = {};
        if (d == m_notificationDescBackRightSpeed)
            m_notificationDescBackRightSpeed = {};
        if (d == m_notificationDescFrontLeftDcLink)
            m_notificationDescFrontLeftDcLink = {};
        if (d == m_notificationDescFrontRightDcLink)
            m_notificationDescFrontRightDcLink = {};
        if (d == m_notificationDescBackLeftDcLink)
            m_notificationDescBackLeftDcLink = {};
        if (d == m_notificationDescBackRightDcLink)
            m_notificationDescBackRightDcLink = {};

        disconnectInternal();
    }
}

void DeviceHandler::disconnectService()
{
    m_foundBobbycarService = false;

    //disable notifications
    if (m_service)
    {
        if (m_notificationDescFrontVoltage.isValid() && m_notificationDescFrontVoltage.value() == QByteArray::fromHex("0100"))
            m_service->writeDescriptor(m_notificationDescFrontVoltage, QByteArray::fromHex("0000"));
        if (m_notificationDescBackVoltage.isValid() && m_notificationDescBackVoltage.value() == QByteArray::fromHex("0100"))
            m_service->writeDescriptor(m_notificationDescBackVoltage, QByteArray::fromHex("0000"));
        if (m_notificationDescFrontTemperature.isValid() && m_notificationDescFrontTemperature.value() == QByteArray::fromHex("0100"))
            m_service->writeDescriptor(m_notificationDescFrontTemperature, QByteArray::fromHex("0000"));
        if (m_notificationDescBackTemperature.isValid() && m_notificationDescBackTemperature.value() == QByteArray::fromHex("0100"))
            m_service->writeDescriptor(m_notificationDescBackTemperature, QByteArray::fromHex("0000"));
        if (m_notificationDescFrontLeftError.isValid() && m_notificationDescFrontLeftError.value() == QByteArray::fromHex("0100"))
            m_service->writeDescriptor(m_notificationDescFrontLeftError, QByteArray::fromHex("0000"));
        if (m_notificationDescFrontRightError.isValid() && m_notificationDescFrontRightError.value() == QByteArray::fromHex("0100"))
            m_service->writeDescriptor(m_notificationDescFrontRightError, QByteArray::fromHex("0000"));
        if (m_notificationDescBackLeftError.isValid() && m_notificationDescBackLeftError.value() == QByteArray::fromHex("0100"))
            m_service->writeDescriptor(m_notificationDescBackLeftError, QByteArray::fromHex("0000"));
        if (m_notificationDescBackRightError.isValid() && m_notificationDescBackRightError.value() == QByteArray::fromHex("0100"))
            m_service->writeDescriptor(m_notificationDescBackRightError, QByteArray::fromHex("0000"));
        if (m_notificationDescFrontLeftSpeed.isValid() && m_notificationDescFrontLeftSpeed.value() == QByteArray::fromHex("0100"))
            m_service->writeDescriptor(m_notificationDescFrontLeftSpeed, QByteArray::fromHex("0000"));
        if (m_notificationDescFrontRightSpeed.isValid() && m_notificationDescFrontRightSpeed.value() == QByteArray::fromHex("0100"))
            m_service->writeDescriptor(m_notificationDescFrontRightSpeed, QByteArray::fromHex("0000"));
        if (m_notificationDescBackLeftSpeed.isValid() && m_notificationDescBackLeftSpeed.value() == QByteArray::fromHex("0100"))
            m_service->writeDescriptor(m_notificationDescBackLeftSpeed, QByteArray::fromHex("0000"));
        if (m_notificationDescBackRightSpeed.isValid() && m_notificationDescBackRightSpeed.value() == QByteArray::fromHex("0100"))
            m_service->writeDescriptor(m_notificationDescBackRightSpeed, QByteArray::fromHex("0000"));
        if (m_notificationDescFrontLeftDcLink.isValid() && m_notificationDescFrontLeftDcLink.value() == QByteArray::fromHex("0100"))
            m_service->writeDescriptor(m_notificationDescFrontLeftDcLink, QByteArray::fromHex("0000"));
        if (m_notificationDescFrontRightDcLink.isValid() && m_notificationDescFrontRightDcLink.value() == QByteArray::fromHex("0100"))
            m_service->writeDescriptor(m_notificationDescFrontRightDcLink, QByteArray::fromHex("0000"));
        if (m_notificationDescBackLeftDcLink.isValid() && m_notificationDescBackLeftDcLink.value() == QByteArray::fromHex("0100"))
            m_service->writeDescriptor(m_notificationDescBackLeftDcLink, QByteArray::fromHex("0000"));
        if (m_notificationDescBackRightDcLink.isValid() && m_notificationDescBackRightDcLink.value() == QByteArray::fromHex("0100"))
            m_service->writeDescriptor(m_notificationDescBackRightDcLink, QByteArray::fromHex("0000"));

        disconnectInternal();
    }
}

void DeviceHandler::disconnectInternal()
{
    if (!m_notificationDescFrontVoltage.isValid() &&
        !m_notificationDescBackVoltage.isValid() &&
        !m_notificationDescFrontTemperature.isValid() &&
        !m_notificationDescBackTemperature.isValid() &&
        !m_notificationDescFrontLeftError.isValid() &&
        !m_notificationDescFrontRightError.isValid() &&
        !m_notificationDescBackLeftError.isValid() &&
        !m_notificationDescBackRightError.isValid() &&
        !m_notificationDescFrontLeftSpeed.isValid() &&
        !m_notificationDescFrontRightSpeed.isValid() &&
        !m_notificationDescBackLeftSpeed.isValid() &&
        !m_notificationDescBackRightSpeed.isValid() &&
        !m_notificationDescFrontLeftDcLink.isValid() &&
        !m_notificationDescFrontRightDcLink.isValid() &&
        !m_notificationDescBackLeftDcLink.isValid() &&
        !m_notificationDescBackRightDcLink.isValid())
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
