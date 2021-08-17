#include "bluetoothbaseclass.h"

BluetoothBaseClass::BluetoothBaseClass(QObject *parent) :
    QObject{parent}
{
}

void BluetoothBaseClass::setError(const QString &error)
{
    if (m_error == error)
        return;

    m_error = error;
    emit errorChanged();
}

void BluetoothBaseClass::setInfo(const QString &info)
{
    if (m_info == info)
        return;

    m_info = info;
    emit infoChanged();
}

void BluetoothBaseClass::clearMessages()
{
    setInfo("");
    setError("");
}
