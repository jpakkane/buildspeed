/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -N -p ofonomodemnetworkregistration.h -c OfonoModemNetworkRegistration ofonomodem.xml org.ofono.NetworkRegistration
 *
 * qdbusxml2cpp is Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef OFONOMODEMNETWORKREGISTRATION_H_1416489111
#define OFONOMODEMNETWORKREGISTRATION_H_1416489111

#include "qdbusdatatypes.h"
#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface org.ofono.NetworkRegistration
 */
class OfonoModemNetworkRegistration: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.ofono.NetworkRegistration"; }

public:
    OfonoModemNetworkRegistration(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~OfonoModemNetworkRegistration();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<OfonoObjectPropertyList> GetOperators()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("GetOperators"), argumentList);
    }

    inline QDBusPendingReply<QVariantMap> GetProperties()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("GetProperties"), argumentList);
    }

    inline QDBusPendingReply<> Register()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("Register"), argumentList);
    }

    inline QDBusPendingReply<OfonoObjectPropertyList> Scan()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("Scan"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void PropertyChanged(const QString &name, const QDBusVariant &value);
};

#endif