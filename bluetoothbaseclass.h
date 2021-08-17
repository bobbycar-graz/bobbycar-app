#pragma once

// Qt includes
#include <QObject>

class BluetoothBaseClass : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString error READ error WRITE setError NOTIFY errorChanged)
    Q_PROPERTY(QString info READ info WRITE setInfo NOTIFY infoChanged)

public:
    explicit BluetoothBaseClass(QObject *parent = nullptr);

    QString error() const { return m_error; }
    void setError(const QString& error);

    QString info() const { return m_info; }
    void setInfo(const QString& info);

    void clearMessages();

signals:
    void errorChanged();
    void infoChanged();

private:
    QString m_error;
    QString m_info;
};
