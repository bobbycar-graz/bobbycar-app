#include "devicefinder.h"

// system includes
#include <algorithm>

// local includes
#include "devicehandler.h"

DeviceFinder::DeviceFinder(QObject *parent):
    QAbstractItemModel{parent},
    m_deviceDiscoveryAgent{this}
{
    m_deviceDiscoveryAgent.setLowEnergyDiscoveryTimeout(5000);

    connect(&m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &DeviceFinder::addDevice);
    connect(&m_deviceDiscoveryAgent, qOverload<QBluetoothDeviceDiscoveryAgent::Error>(&QBluetoothDeviceDiscoveryAgent::error),
            this, &DeviceFinder::scanError);

    connect(&m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &DeviceFinder::scanFinished);
    connect(&m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled, this, &DeviceFinder::scanFinished);
}

QModelIndex DeviceFinder::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return createIndex(row, column);
}

QModelIndex DeviceFinder::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)
    return {};
}

int DeviceFinder::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_devices.size();
}

int DeviceFinder::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant DeviceFinder::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        qWarning() << "invalid index";
        return {};
    }

    if (index.row() < 0 ||
        index.row() >= m_devices.size() ||
        index.column() != 0)
    {
        qWarning() << "index out of bounds" << index;
        return {};
    }

    const auto &device = m_devices.at(index.row());

    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
    case Qt::UserRole + 1:
        return device.name();
    case Qt::UserRole + 2:
        return device.address().toString();
    }

    return {};
}

QMap<int, QVariant> DeviceFinder::itemData(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        qWarning() << "invalid index";
        return {};
    }

    if (index.row() < 0 ||
        index.row() >= m_devices.size() ||
        index.column() != 0)
    {
        qWarning() << "index out of bounds" << index;
        return {};
    }

    const auto &device = m_devices.at(index.row());

    return QMap<int, QVariant> {
        { Qt::UserRole + 1, device.name() },
        { Qt::UserRole + 2, device.address().toString() }
    };
}

QHash<int, QByteArray> DeviceFinder::roleNames() const
{
    return QHash<int, QByteArray> {
        { Qt::UserRole + 1, QByteArrayLiteral("deviceName") },
        { Qt::UserRole + 2, QByteArrayLiteral("deviceAddress") }
    };
}

void DeviceFinder::startSearch()
{
    clearMessages();

    beginResetModel();
    m_devices.clear();
    endResetModel();

    m_deviceDiscoveryAgent.start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);

    emit scanningChanged();
    setInfo(tr("Scanning for devices..."));
}

void DeviceFinder::addDevice(const QBluetoothDeviceInfo &device)
{
    // If device is LowEnergy-device, add it to the list
    if (!(device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration))
        return;

    //if (!device.name().contains("bobby"))
    //    return;

    beginInsertRows({}, m_devices.size(), m_devices.size());
    m_devices.push_back(device);
    endInsertRows();

    setInfo(tr("Low Energy device found. Scanning more..."));
}

void DeviceFinder::scanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    switch (error)
    {
    case QBluetoothDeviceDiscoveryAgent::PoweredOffError:
        setError(tr("The Bluetooth adaptor is powered off."));
        break;
    case QBluetoothDeviceDiscoveryAgent::InputOutputError:
        setError(tr("Writing or reading from the device resulted in an error."));
        break;
    default:
        setError(tr("An unknown error has occurred."));
    }
}

void DeviceFinder::scanFinished()
{
    if (m_devices.empty())
        setError(tr("No Low Energy devices found."));
    else
        setInfo(tr("Scanning done."));

    emit scanningChanged();
}

void DeviceFinder::connectToService(const QString &address)
{
    m_deviceDiscoveryAgent.stop();

    auto iter = std::find_if(std::cbegin(m_devices), std::cend(m_devices), [&address](const QBluetoothDeviceInfo &device){
        return device.address().toString() == address;
    });

    if (iter == std::cend(m_devices))
    {
        qWarning() << "could not find address" << address;
        setError(tr("could not find address %0").arg(address));
        return;
    }

    if (!m_handler)
    {
        qWarning() << "no valid handler!";
        setError(tr("no valid handler!"));
        return;
    }

    m_handler->setDevice(*iter);

    clearMessages();
}
