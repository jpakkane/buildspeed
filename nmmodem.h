/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -N -p nmmodem.h -c NetworkManagerModem nmmodem.xml org.freedesktop.NetworkManager.Device.Modem
 *
 * qdbusxml2cpp is Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef NMMODEM_H_1417013159
#define NMMODEM_H_1417013159

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface org.freedesktop.NetworkManager.Device.Modem
 */
class NetworkManagerModem: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.freedesktop.NetworkManager.Device.Modem"; }

public:
    NetworkManagerModem(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~NetworkManagerModem();

    Q_PROPERTY(uint CurrentCapabilities READ currentCapabilities)
    inline uint currentCapabilities() const
    { return qvariant_cast< uint >(property("CurrentCapabilities")); }

    Q_PROPERTY(uint ModemCapabilities READ modemCapabilities)
    inline uint modemCapabilities() const
    { return qvariant_cast< uint >(property("ModemCapabilities")); }

public Q_SLOTS: // METHODS
Q_SIGNALS: // SIGNALS
    void PropertiesChanged(const QVariantMap &in0);
};

#endif

