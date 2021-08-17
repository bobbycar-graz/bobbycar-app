#pragma once

// Qt includes
#include <QDateTime>
#include <QTimer>
#include <QVector>
#include <QLowEnergyController>
#include <QLowEnergyService>

// local includes
#include "bluetoothbaseclass.h"

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

    Q_PROPERTY(bool remoteControlActive READ remoteControlActive WRITE setRemoteControlActive NOTIFY remoteControlActiveChanged);
    Q_PROPERTY(int remoteControlFrontLeft WRITE setRemoteControlFrontLeft);
    Q_PROPERTY(int remoteControlFrontRight WRITE setRemoteControlFrontRight);
    Q_PROPERTY(int remoteControlBackLeft WRITE setRemoteControlBackLeft);
    Q_PROPERTY(int remoteControlBackRight WRITE setRemoteControlBackRight);

public:
    enum class AddressType {
        PublicAddress,
        RandomAddress
    };
    Q_ENUM(AddressType)

    DeviceHandler(QObject *parent = nullptr);

    void setDevice(const QBluetoothDeviceInfo &device);
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

    bool remoteControlActive() const { return m_timerId != -1; }
    void setRemoteControlActive(bool remoteControlActive);
    void setRemoteControlFrontLeft(int remoteControlFrontLeft) { m_remoteControlFrontLeft = remoteControlFrontLeft; }
    void setRemoteControlFrontRight(int remoteControlFrontRight) { m_remoteControlFrontRight = remoteControlFrontRight; }
    void setRemoteControlBackLeft(int remoteControlBackLeft) { m_remoteControlBackLeft = remoteControlBackLeft; }
    void setRemoteControlBackRight(int remoteControlBackRight) { m_remoteControlBackRight = remoteControlBackRight; }

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

    void remoteControlActiveChanged();

protected:
    void timerEvent(QTimerEvent *event) override;

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
    void confirmedCharacteristicWrite(const QLowEnergyCharacteristic &info,
                                      const QByteArray &value);

    void sendRemoteControl();

private:
    QLowEnergyController::RemoteAddressType m_addressType = QLowEnergyController::PublicAddress;
    QLowEnergyController *m_control = nullptr;
    QLowEnergyService *m_service = nullptr;
    QLowEnergyDescriptor m_notificationDescLivestats;
    QLowEnergyCharacteristic m_remotecontrolCharacteristic;
    QBluetoothDeviceInfo m_currentDevice;

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
    float m_backRightSpeed{};
    float m_frontLeftDcLink{};
    float m_frontRightDcLink{};
    float m_backLeftDcLink{};
    float m_backRightDcLink{};

    int m_timerId{-1};

    int m_remoteControlFrontLeft{};
    int m_remoteControlFrontRight{};
    int m_remoteControlBackLeft{};
    int m_remoteControlBackRight{};

    bool m_waitingForWrite{};
};
