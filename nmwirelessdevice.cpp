#include "nmwirelessdevice.h"

/*
 * Implementation of interface class NetworkManagerWirelessDevice
 */

NetworkManagerWirelessDevice::NetworkManagerWirelessDevice(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

NetworkManagerWirelessDevice::~NetworkManagerWirelessDevice()
{
}
