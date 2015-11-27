#include"urfkillswitch.h"

/*
 * Implementation of interface class UrfkillWlan
 */

UrfkillSwitch::UrfkillSwitch(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

UrfkillSwitch::~UrfkillSwitch()
{
}

