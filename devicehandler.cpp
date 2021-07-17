#include "devicehandler.h"
#include "deviceinfo.h"
#include <QtEndian>
#include <QRandomGenerator>

namespace {
const QBluetoothUuid bobbycarServiceUuid{QUuid::fromString(QStringLiteral("0335e46c-f355-4ce6-8076-017de08cee98"))};
const QBluetoothUuid frontLeftSpeedCharacUuid{QUuid::fromString(QStringLiteral("81287506-8985-4cea-9a58-92fc5ad2c570"))};
const QBluetoothUuid frontRightSpeedCharacUuid{QUuid::fromString(QStringLiteral("2f326a23-a676-4f87-b5cb-37a8fd7fe466"))};
const QBluetoothUuid backLeftSpeedCharacUuid{QUuid::fromString(QStringLiteral("a7f951c0-e984-460d-98ed-0d54c64092d5"))};
const QBluetoothUuid backRightSpeedCharacUuid{QUuid::fromString(QStringLiteral("14efe73f-6e34-49b3-b2c7-b513f3f5aee2"))};
}

DeviceHandler::DeviceHandler(QObject *parent) :
    BluetoothBaseClass(parent),
    m_foundBobbycarService(false),
    m_measuring(false),
    m_currentValue(0),
    m_min(0), m_max(0), m_sum(0), m_avg(0), m_distance(0)
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

void DeviceHandler::startMeasurement()
{
    if (alive()) {
        m_start = QDateTime::currentDateTime();
        m_min = 0;
        m_max = 0;
        m_avg = 0;
        m_sum = 0;
        m_distance = 0;
        m_measuring = true;
        m_measurements.clear();
        emit measuringChanged();
    }
}

void DeviceHandler::stopMeasurement()
{
    m_measuring = false;
    emit measuringChanged();
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

void DeviceHandler::serviceStateChanged(QLowEnergyService::ServiceState s)
{
    switch (s) {
    case QLowEnergyService::DiscoveringServices:
        setInfo(tr("Discovering services..."));
        break;
    case QLowEnergyService::ServiceDiscovered:
    {
        setInfo(tr("Service discovered."));

        const QLowEnergyCharacteristic hrChar = m_service->characteristic(frontLeftSpeedCharacUuid);
        if (!hrChar.isValid()) {
            setError("Bobbycar Data not found.");
            break;
        }

        m_notificationDesc = hrChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
        if (m_notificationDesc.isValid())
            m_service->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0100"));

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
    // ignore any other characteristic change -> shouldn't really happen though
    if (c.uuid() != frontLeftSpeedCharacUuid)
        return;

    bool ok;
    float val = value.toFloat(&ok);
    if (ok)
        addMeasurement(val);
    else
        qWarning() << "could not parse float" << value;
}

void DeviceHandler::confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value)
{
    if (d.isValid() && d == m_notificationDesc && value == QByteArray::fromHex("0000")) {
        //disabled notifications -> assume disconnect intent
        m_control->disconnectFromDevice();
        delete m_service;
        m_service = nullptr;
    }
}

void DeviceHandler::disconnectService()
{
    m_foundBobbycarService = false;

    //disable notifications
    if (m_notificationDesc.isValid() && m_service
            && m_notificationDesc.value() == QByteArray::fromHex("0100")) {
        m_service->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0000"));
    } else {
        if (m_control)
            m_control->disconnectFromDevice();

        delete m_service;
        m_service = nullptr;
    }
}

bool DeviceHandler::measuring() const
{
    return m_measuring;
}

bool DeviceHandler::alive() const
{
    if (m_service)
        return m_service->state() == QLowEnergyService::ServiceDiscovered;

    return false;
}

int DeviceHandler::time() const
{
    return m_start.secsTo(m_stop);
}

void DeviceHandler::addMeasurement(float value)
{
    m_currentValue = value;

    // If measuring and value is appropriate
    if (m_measuring) {

        m_stop = QDateTime::currentDateTime();
        m_measurements << value;

        m_min = m_min == 0 ? value : qMin(value, m_min);
        m_max = qMax(value, m_max);
        m_sum += value;
        m_avg = (double)m_sum / m_measurements.size();
        m_distance += value * 1000.f * 3600;
    }

    emit statsChanged();
}
