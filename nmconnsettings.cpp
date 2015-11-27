#include "nmconnsettings.h"

/*
 * Implementation of interface class NetworkManagerConnectionSettings
 */

NetworkManagerConnectionSettings::NetworkManagerConnectionSettings(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

NetworkManagerConnectionSettings::~NetworkManagerConnectionSettings()
{
}
