#include"nmsettings.h"

/*
 * Implementation of interface class NetworkManagerSettings
 */

NetworkManagerSettings::NetworkManagerSettings(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

NetworkManagerSettings::~NetworkManagerSettings()
{
}

