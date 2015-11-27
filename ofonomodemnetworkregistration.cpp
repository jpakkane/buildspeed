#include"ofonomodemnetworkregistration.h"

/*
 * Implementation of interface class OfonoModemNetworkRegistration
 */

OfonoModemNetworkRegistration::OfonoModemNetworkRegistration(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

OfonoModemNetworkRegistration::~OfonoModemNetworkRegistration()
{
}
