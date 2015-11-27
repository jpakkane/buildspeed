#include"ofonomodemsimmanager.h"

/*
 * Implementation of interface class OfonoModemSimManager
 */

OfonoModemSimManager::OfonoModemSimManager(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

OfonoModemSimManager::~OfonoModemSimManager()
{
}

