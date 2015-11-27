#include"ofonoroot.h"

/*
 * Implementation of interface class OfonoRoot
 */

OfonoRoot::OfonoRoot(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

OfonoRoot::~OfonoRoot()
{
}

