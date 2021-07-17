#ifndef DEVICEHANDLER_H
#define DEVICEHANDLER_H

#include "bluetoothbaseclass.h"

#include <QDateTime>
#include <QTimer>
#include <QVector>

#include <QLowEnergyController>
#include <QLowEnergyService>

class DeviceInfo;

class DeviceHandler : public BluetoothBaseClass
{
    Q_OBJECT

    Q_PROPERTY(bool measuring READ measuring NOTIFY measuringChanged)
    Q_PROPERTY(bool alive READ alive NOTIFY aliveChanged)
    Q_PROPERTY(float speed READ speed NOTIFY statsChanged)
    Q_PROPERTY(float maxSpeed READ maxSpeed NOTIFY statsChanged)
    Q_PROPERTY(float minSpeed READ minSpeed NOTIFY statsChanged)
    Q_PROPERTY(float avgSpeed READ avgSpeed NOTIFY statsChanged)
    Q_PROPERTY(int time READ time NOTIFY statsChanged)
    Q_PROPERTY(float distance READ distance NOTIFY statsChanged)
    Q_PROPERTY(AddressType addressType READ addressType WRITE setAddressType)

public:
    enum class AddressType {
        PublicAddress,
        RandomAddress
    };
    Q_ENUM(AddressType)

    DeviceHandler(QObject *parent = nullptr);

    void setDevice(DeviceInfo *device);
    void setAddressType(AddressType type);
    AddressType addressType() const;

    bool measuring() const;
    bool alive() const;

    // Statistics
    float speed() const { return m_currentValue; }
    int time() const;
    float avgSpeed() const { return m_avg; }
    float maxSpeed() const { return m_max; }
    float minSpeed() const { return m_min; }
    float distance() const { return m_distance; }

signals:
    void measuringChanged();
    void aliveChanged();
    void statsChanged();

public slots:
    void startMeasurement();
    void stopMeasurement();
    void disconnectService();

private:
    //QLowEnergyController
    void serviceDiscovered(const QBluetoothUuid &);
    void serviceScanDone();

    //QLowEnergyService
    void serviceStateChanged(QLowEnergyService::ServiceState s);
    void updateBobbycarValue(const QLowEnergyCharacteristic &c,
                              const QByteArray &value);
    void confirmedDescriptorWrite(const QLowEnergyDescriptor &d,
                              const QByteArray &value);

private:
    void addMeasurement(float value);

    QLowEnergyController *m_control = nullptr;
    QLowEnergyService *m_service = nullptr;
    QLowEnergyDescriptor m_notificationDesc;
    DeviceInfo *m_currentDevice = nullptr;

    bool m_foundBobbycarService;
    bool m_measuring;
    float m_currentValue, m_min, m_max, m_sum;
    float m_avg, m_distance;

    // Statistics
    QDateTime m_start;
    QDateTime m_stop;

    QVector<float> m_measurements;
    QLowEnergyController::RemoteAddressType m_addressType = QLowEnergyController::PublicAddress;
};

#endif // DEVICEHANDLER_H
