#include "nmroot.h"

/*
 * Implementation of interface class NetworkManagerRoot
 */

NetworkManagerRoot::NetworkManagerRoot(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

NetworkManagerRoot::~NetworkManagerRoot()
{
}

