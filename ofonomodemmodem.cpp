#include"ofonomodemmodem.h"

/*
 * Implementation of interface class OfonoModemModem
 */

OfonoModemModem::OfonoModemModem(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

OfonoModemModem::~OfonoModemModem()
{
}

