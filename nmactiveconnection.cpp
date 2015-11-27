#include "nmactiveconnection.h"

/*
 * Implementation of interface class NetworkManagerActiveConnection
 */

NetworkManagerActiveConnection::NetworkManagerActiveConnection(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

NetworkManagerActiveConnection::~NetworkManagerActiveConnection()
{
}
