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
    Q_PROPERTY(AddressType addressType READ addressType WRITE setAddressType)
    Q_PROPERTY(bool alive READ alive NOTIFY aliveChanged)
    Q_PROPERTY(float frontVoltage READ frontVoltage NOTIFY frontVoltageChanged);
    Q_PROPERTY(float backVoltage READ backVoltage NOTIFY backVoltageChanged);
    Q_PROPERTY(float frontTemperature READ frontTemperature NOTIFY frontTemperatureChanged);
    Q_PROPERTY(float backTemperature READ backTemperature NOTIFY backTemperatureChanged);
    Q_PROPERTY(int frontLeftError READ frontLeftError NOTIFY frontLeftErrorChanged);
    Q_PROPERTY(int frontRightError READ frontRightError NOTIFY frontRightErrorChanged);
    Q_PROPERTY(int backLeftError READ backLeftError NOTIFY backLeftErrorChanged);
    Q_PROPERTY(int backRightError READ backRightError NOTIFY backRightErrorChanged);
    Q_PROPERTY(float frontLeftSpeed READ frontLeftSpeed NOTIFY frontLeftSpeedChanged);
    Q_PROPERTY(float frontRightSpeed READ frontRightSpeed NOTIFY frontRightSpeedChanged);
    Q_PROPERTY(float backLeftSpeed READ backLeftSpeed NOTIFY backLeftSpeedChanged);
    Q_PROPERTY(float backRightSpeed READ backRightSpeed NOTIFY backRightSpeedChanged);
    Q_PROPERTY(float frontLeftDcLink READ frontLeftDcLink NOTIFY frontLeftDcLinkChanged);
    Q_PROPERTY(float frontRightDcLink READ frontRightDcLink NOTIFY frontRightDcLinkChanged);
    Q_PROPERTY(float backLeftDcLink READ backLeftDcLink NOTIFY backLeftDcLinkChanged);
    Q_PROPERTY(float backRightDcLink READ backRightDcLink NOTIFY backRightDcLinkChanged);

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

    bool alive() const;

    float frontVoltage() const { return m_frontVoltage; }
    float backVoltage() const { return m_backVoltage; }
    float frontTemperature() const { return m_frontTemperature; }
    float backTemperature() const { return m_backTemperature; }
    int frontLeftError() const { return m_frontLeftError; }
    int frontRightError() const { return m_frontRightError; }
    int backLeftError() const { return m_backLeftError; }
    int backRightError() const { return m_backRightError; }
    float frontLeftSpeed() const { return m_frontLeftSpeed; }
    float frontRightSpeed() const { return m_frontRightSpeed; }
    float backLeftSpeed() const { return m_backLeftSpeed; }
    float backRightSpeed() const { return m_backRightSpeed; }
    float frontLeftDcLink() const { return m_frontLeftDcLink; }
    float frontRightDcLink() const { return m_frontRightDcLink; }
    float backLeftDcLink() const { return m_backLeftDcLink; }
    float backRightDcLink() const { return m_backRightDcLink; }

signals:
    void aliveChanged();

    void frontVoltageChanged();
    void backVoltageChanged();
    void frontTemperatureChanged();
    void backTemperatureChanged();
    void frontLeftErrorChanged();
    void frontRightErrorChanged();
    void backLeftErrorChanged();
    void backRightErrorChanged();
    void frontLeftSpeedChanged();
    void frontRightSpeedChanged();
    void backLeftSpeedChanged();
    void backRightSpeedChanged();
    void frontLeftDcLinkChanged();
    void frontRightDcLinkChanged();
    void backLeftDcLinkChanged();
    void backRightDcLinkChanged();

public slots:
    void disconnectService();

private:
    void disconnectInternal();

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
    QLowEnergyController::RemoteAddressType m_addressType = QLowEnergyController::PublicAddress;
    QLowEnergyController *m_control = nullptr;
    QLowEnergyService *m_service = nullptr;
    QLowEnergyDescriptor m_notificationDescFrontVoltage;
    QLowEnergyDescriptor m_notificationDescBackVoltage;
    QLowEnergyDescriptor m_notificationDescFrontTemperature;
    QLowEnergyDescriptor m_notificationDescBackTemperature;
    QLowEnergyDescriptor m_notificationDescFrontLeftError;
    QLowEnergyDescriptor m_notificationDescFrontRightError;
    QLowEnergyDescriptor m_notificationDescBackLeftError;
    QLowEnergyDescriptor m_notificationDescBackRightError;
    QLowEnergyDescriptor m_notificationDescFrontLeftSpeed;
    QLowEnergyDescriptor m_notificationDescFrontRightSpeed;
    QLowEnergyDescriptor m_notificationDescBackLeftSpeed;
    QLowEnergyDescriptor m_notificationDescBackRightSpeed;
    QLowEnergyDescriptor m_notificationDescFrontLeftDcLink;
    QLowEnergyDescriptor m_notificationDescFrontRightDcLink;
    QLowEnergyDescriptor m_notificationDescBackLeftDcLink;
    QLowEnergyDescriptor m_notificationDescBackRightDcLink;
    DeviceInfo *m_currentDevice{};

    bool m_foundBobbycarService{};

    float m_frontVoltage{};
    float m_backVoltage{};
    float m_frontTemperature{};
    float m_backTemperature{};
    uint8_t m_frontLeftError{};
    uint8_t m_frontRightError{};
    uint8_t m_backLeftError{};
    uint8_t m_backRightError{};
    float m_frontLeftSpeed{};
    float m_frontRightSpeed{};
    float m_backLeftSpeed{};
    float m_backRightSpeed;
    float m_frontLeftDcLink{};
    float m_frontRightDcLink{};
    float m_backLeftDcLink{};
    float m_backRightDcLink{};
};

#endif // DEVICEHANDLER_H
