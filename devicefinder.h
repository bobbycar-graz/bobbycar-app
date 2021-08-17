#pragma once

// system includes
#include <memory>
#include <vector>

// Qt includes
#include <QAbstractItemModel>
#include <QTimer>
#include <QVariant>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QtQml/qqml.h>

// forward declares
class DeviceHandler;

class DeviceFinder : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)
    Q_PROPERTY(DeviceHandler* handler READ handler WRITE setHandler NOTIFY handlerChanged)
    Q_PROPERTY(QString error READ error WRITE setError NOTIFY errorChanged)
    Q_PROPERTY(QString info READ info WRITE setInfo NOTIFY infoChanged)
    QML_ELEMENT

public:
    DeviceFinder(QObject *parent = nullptr);

    bool scanning() const { return m_deviceDiscoveryAgent.isActive(); }

    DeviceHandler* handler() { return m_handler; }
    const DeviceHandler* handler() const { return m_handler; }
    void setHandler(DeviceHandler* handler) { if (m_handler == handler) return; m_handler = handler; emit handlerChanged(); }

    QString error() const { return m_error; }
    void setError(const QString& error) { if (m_error == error) return; m_error = error; emit errorChanged(); }

    QString info() const { return m_info; }
    void setInfo(const QString& info) { if (m_info == info) return; m_info = info; emit infoChanged(); }

    void clearMessages() { setInfo(""); setError(""); }

    // QAbstractItemModel interface
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QMap<int, QVariant> itemData(const QModelIndex &index) const override;
    QHash<int, QByteArray> roleNames() const override;

signals:
    void scanningChanged();
    void handlerChanged();
    void errorChanged();
    void infoChanged();

public slots:
    void startSearch();
    void connectToService(const QString &address);

private slots:
    void addDevice(const QBluetoothDeviceInfo &);
    void scanError(QBluetoothDeviceDiscoveryAgent::Error error);
    void scanFinished();

private:
    DeviceHandler *m_handler{};
    QBluetoothDeviceDiscoveryAgent m_deviceDiscoveryAgent;
    std::vector<QBluetoothDeviceInfo> m_devices;
    QString m_error;
    QString m_info;
};
