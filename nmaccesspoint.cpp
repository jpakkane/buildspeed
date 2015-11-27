#include"nmaccesspoint.h"

/*
 * Implementation of interface class NetworkManagerAccessPoint
 */

NetworkManagerAccessPoint::NetworkManagerAccessPoint(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

NetworkManagerAccessPoint::~NetworkManagerAccessPoint()
{
}
