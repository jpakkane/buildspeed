/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -N -p ofonoroot.h -c OfonoRoot ofonoroot.xml org.ofono.Manager
 *
 * qdbusxml2cpp is Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef OFONOROOT_H_1416325045
#define OFONOROOT_H_1416325045

#include "qdbusdatatypes.h"
#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

#include <QDBusMetaType>

/*
 * Proxy class for interface org.ofono.Manager
 */
class OfonoRoot: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.ofono.Manager"; }

public:
    OfonoRoot(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~OfonoRoot();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<ModemPropertyList> GetModems()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("GetModems"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void ModemAdded(const QDBusObjectPath &path, const QVariantMap &properties);
    void ModemRemoved(const QDBusObjectPath &path);
};

#endif

