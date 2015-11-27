#include"urfkillroot.h"

/*
 * Implementation of interface class UrfkillRoot
 */

UrfkillRoot::UrfkillRoot(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

UrfkillRoot::~UrfkillRoot()
{
}
