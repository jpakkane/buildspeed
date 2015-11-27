/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Jussi Pakkanen <jussi.pakkanen@canonical.com>
 */

#include"systemstate.h"
#include"indicatorexporter.h"
#include"indicatoradaptor.h"
#include"urfkillroot.h"
#include"urfkillswitch.h"
#include"nmroot.h"
#include"dbusdata.h"
#include"ofonomodemmodem.h"
#include"ofonoroot.h"
#include"ofonomodemsimmanager.h"
#include"ofonomodemnetworkregistration.h"
#include"ofononetworkoperator.h"
#include"ofonomodemconnectionmanager.h"
#include"numberadder.h"
#include"nmactiveconnection.h"
#include"nmaccesspoint.h"
#include"nmdevice.h"
#include"nmwirelessdevice.h"
#include<algorithm>

namespace {

void replace_or_append(std::vector<WifiAccessPoint> &aps, WifiAccessPoint &new_ap) {
    for(auto &ap : aps) {
        if(ap.ssid == new_ap.ssid && ap.securityFlags == new_ap.securityFlags) {
            if(new_ap.connected || (!ap.connected && new_ap.strength > ap.strength)) {
                ap = new_ap;
            }
            return;
        }
    }
    aps.push_back(new_ap);
}

// Connected first, then by descending signal strength.
bool accesspointSorter(const WifiAccessPoint &ap1, const WifiAccessPoint &ap2) {
    if(ap1.connected && !ap2.connected) {
        return true;
    }
    if(ap2.connected && !ap1.connected) {
        return false;
    }
    if(ap1.strength > ap2.strength) {
        return true;
    }
    if(ap2.strength > ap1.strength) {
        return false;
    }
    return false;
}

OfonoModem grabOfonoData(QDBusObjectPath &path) {
    OfonoModem md;
    OfonoModemModem modem(OFONO_SERVICE, path.path(), QDBusConnection::systemBus(), nullptr);
    OfonoModemSimManager man(OFONO_SERVICE, path.path(), QDBusConnection::systemBus(), nullptr);
    OfonoModemNetworkRegistration netreg(OFONO_SERVICE, path.path(), QDBusConnection::systemBus(), nullptr);
    OfonoModemConnectionManager connmgr(OFONO_SERVICE, path.path(), QDBusConnection::systemBus(), nullptr);
    auto mprops = man.GetProperties().value();
    auto regprops = netreg.GetProperties().value();
    const auto &props = modem.GetProperties().value();
    const auto operators = netreg.GetOperators().value();
    auto connprops = connmgr.GetProperties().value();

    md.online = props["Online"].toBool();
    md.technology = connprops["Bearer"].toString();
    md.simlock =  mprops["PinRequired"].toString();
    md.status = regprops["Status"].toString();
    md.strength = regprops["Strength"].toInt();
    // Fixme, should check all operators.
    if(!operators.empty()) {
        md.opname = regprops["Name"].toString();
    } else {
        md.opname = "none";
    }
    return md;
}

}

SystemState::SystemState(IndicatorExporter *exporter, QObject *parent, bool verbose) : QObject(parent), verbose(verbose),
        activeWirelessDevice(nullptr), exporter(exporter), queuedChange(PENDING_UNKNOWN) {
    urfkill = new UrfkillRoot(URFKILL_SERVICE, URFKILL_OBJECT, QDBusConnection::systemBus(), this);
    btkill = new UrfkillSwitch(URFKILL_SERVICE, URFKILL_BLUETOOTH_OBJECT, QDBusConnection::systemBus(), this);
    wlankill = new UrfkillSwitch(URFKILL_SERVICE, URFKILL_WLAN_OBJECT, QDBusConnection::systemBus(), this);
    nmroot = new NetworkManagerRoot(NM_SERVICE, NM_OBJECT, QDBusConnection::systemBus(), this);

    flightModeStatus = urfkill->IsFlightMode().value() ? PENDING_ON : PENDING_OFF;

    // This needs to be our child so it gets exported to dbus properly.
    // It also relays all its signals to its parent (i.e. this object).
    new IndicatorAdaptor(this);

    connect(urfkill, SIGNAL(FlightModeChanged(bool)), this, SLOT(flightModeChanged(bool)));
    connect(btkill, SIGNAL(StateChanged()), this, SLOT(btKillswitchChanged()));
    connect(wlankill, SIGNAL(StateChanged()), this, SLOT(wlanKillswitchChanged()));
    connect(nmroot, SIGNAL(StateChanged(uint)), this, SLOT(nmStateChanged(uint)));
    connect(nmroot, SIGNAL(PropertiesChanged(const QVariantMap&)), this, SLOT(nmPropertiesChanged(const QVariantMap&)));

    OfonoRoot ofono(OFONO_SERVICE, OFONO_OBJECT, QDBusConnection::systemBus(), nullptr);
    auto curModems = ofono.GetModems().value();
    for(int i=0; i < curModems.length(); i++) {
        auto relayer = new NumberAdder(i, this);
        auto modem = new OfonoModemModem(OFONO_SERVICE, curModems[i].first.path(), QDBusConnection::systemBus(), this);

        connect(modem, SIGNAL(PropertyChanged(const QString, const QDBusVariant)), relayer,
                SLOT(modemPropertyChanged(const QString, const QDBusVariant)));
        connect(relayer, SIGNAL(modemPropertyChanged(int, const QString&, const QDBusVariant&)), this,
                SLOT(modemPropertyChanged(int, const QString&, const QDBusVariant&)));

        auto simman = new OfonoModemSimManager(OFONO_SERVICE, curModems[i].first.path(), QDBusConnection::systemBus(), this);
        connect(simman, SIGNAL(PropertyChanged(const QString, const QDBusVariant)), relayer,
                SLOT(simmanPropertyChanged(const QString, const QDBusVariant)));
        connect(relayer, SIGNAL(simmanPropertyChanged(int, const QString&, const QDBusVariant&)), this,
                SLOT(simmanPropertyChanged(int, const QString&, const QDBusVariant&)));

        auto netreg = new OfonoModemNetworkRegistration(OFONO_SERVICE, curModems[i].first.path(), QDBusConnection::systemBus(), this);
        connect(netreg, SIGNAL(PropertyChanged(const QString, const QDBusVariant)), relayer,
                SLOT(netregPropertyChanged(const QString, const QDBusVariant)));
        connect(relayer, SIGNAL(netregPropertyChanged(int, const QString&, const QDBusVariant&)), this,
                SLOT(netregPropertyChanged(int, const QString&, const QDBusVariant&)));
        auto ops = netreg->GetOperators().value();
        if(!ops.empty()) {
            operators.push_back(createOperator(ops[0].first, relayer));
        } else {
            operators.push_back(nullptr);
        }

        auto connmgr = new OfonoModemConnectionManager(OFONO_SERVICE, curModems[i].first.path(), QDBusConnection::systemBus(), this);
        connect(connmgr, SIGNAL(PropertyChanged(const QString, const QDBusVariant)), relayer,
                SLOT(connmgrPropertyChanged(QString, QDBusVariant)));
        connect(relayer, SIGNAL(connmgrPropertyChanged(int, const QString&, const QDBusVariant&)), this,
                SLOT(connmgrPropertyChanged(int, const QString&, const QDBusVariant&)));

        modems.push_back(modem);
        simmanagers.push_back(simman);
        netregs.push_back(netreg);
        connmgrs.push_back(connmgr);
        modemStates.push_back(grabOfonoData(curModems[i].first));
        relayers.push_back(relayer);
    }

    rebuildApList();
    printApList();
    reconnectNmApSignals();
    reexport();
}

SystemState::~SystemState() {
}

void SystemState::reconnectNmApSignals() {
    for(NetworkManagerAccessPoint *apo : apObjects) {
        delete apo;
    }
    for(const auto &ap : aps) {
        auto apo = new NetworkManagerAccessPoint(NM_SERVICE, ap.path.path(), QDBusConnection::systemBus(), this);
        connect(apo, SIGNAL(PropertiesChanged(const QVariantMap &)), this,
                SLOT(nmApPropertiesChanged(const QVariantMap &)));
    }
}
void SystemState::nmApPropertiesChanged(const QVariantMap &props) {
    NetworkManagerAccessPoint *senderObject = dynamic_cast<NetworkManagerAccessPoint*>(sender());
    auto path = senderObject->path();
    for(auto &ap : aps) {
        if(path == ap.path.path()) {
            auto strength = props.find("Strength");
            if(strength != props.end()) {
                int new_strength = strength->toInt();
                if(ap.strength != new_strength) {
                    ap.strength = new_strength;
                    printf("Network %s strength changed to %d\n", ap.ssid.data(), new_strength);
                }
            }
        }
    }
}

OfonoNetworkOperator* SystemState::createOperator(const QDBusObjectPath &p, NumberAdder *relayer) {
    auto oberator = new OfonoNetworkOperator(OFONO_SERVICE, p.path(),
            QDBusConnection::systemBus(), this);
    connect(oberator, SIGNAL(PropertyChanged(const QString, const QDBusVariant)), relayer,
            SLOT(operatorPropertyChanged(const QString, const QDBusVariant)));
    connect(relayer, SIGNAL(operatorPropertyChanged(int, const QString&, const QDBusVariant&)), this,
            SLOT(operatorPropertyChanged(int, const QString&, const QDBusVariant&)));
    return oberator;
}

void SystemState::printApList() {
    for(const auto &ap : aps) {
        printf("%s", ap.ssid.data());
        if(ap.connected) {
            printf(" (connected)");
        }
        printf("\n");
    }
    if(aps.size() == 0) {
        printf("No visible access points.\n");
    }
}

void SystemState::rebuildApList() {
    aps.clear();

    // Fixme, this only works if we have an active connection. Make it scan all devices eventually.
    for(const auto &path : nmroot->activeConnections()) {
        NetworkManagerActiveConnection ac(NM_SERVICE, path.path(), QDBusConnection::systemBus(), nullptr);
        for(const auto &devpath : ac.devices()) {
            NetworkManagerDevice curdev(NM_SERVICE, devpath.path(), QDBusConnection::systemBus(), nullptr);
            auto curdev_type = curdev.deviceType();

            if(curdev_type == 2) {
                NetworkManagerWirelessDevice dev(NM_SERVICE, devpath.path(), QDBusConnection::systemBus(), nullptr);
                auto activeAccesspoint = dev.activeAccessPoint();
                auto reply = dev.GetAccessPoints();
                reply.waitForFinished();
                if(reply.isValid()) {
                    NetworkManagerAccessPoint ap(NM_SERVICE, dev.activeAccessPoint().path(), QDBusConnection::systemBus(), nullptr);
                    for(const auto &c : reply.value()) {
                        NetworkManagerAccessPoint i(NM_SERVICE, c.path(), QDBusConnection::systemBus(), nullptr);
                        auto rsnflags = i.rsnFlags();
                        if(rsnflags == 0) {
                            rsnflags = i.wpaFlags(); // NM seems to fill either of these at random.
                        }
                        WifiAccessPoint current_ap;
                        current_ap.path = c;
                        current_ap.ssid = i.ssid();
                        current_ap.securityFlags = rsnflags;
                        current_ap.connected = c == activeAccesspoint;
                        current_ap.strength = i.strength();
                        replace_or_append(aps, current_ap);
                    }
                }
                std::sort(aps.begin(), aps.end(), accesspointSorter);
                break; // Assumes only 1 wifi device.
            }
        }
    }
}

void SystemState::flightModeChanged(bool newMode) {
    printf("Flight mode change finalised: %d\n", newMode);
    if(flightModeStatus != PENDING_TO_OFF && flightModeStatus != PENDING_TO_ON) {
        printf("Strange: got a change for flightmode without the corresponding pending change. Continuing anyway.\n");
        // FIXME, send status change signal if there are no pending changes.
    }
    flightModeStatus = newMode ? PENDING_ON : PENDING_OFF;
    // If there was a pending change execute it.
    if(queuedChange != PENDING_UNKNOWN) {
        auto newval = queuedChange;
        queuedChange = PENDING_UNKNOWN;
        set_flightmode(newval);
    }
}

void SystemState::btKillswitchChanged() {
    printf("Bluetooth killswitch changed to: %d\n", btkill->state());
}

void SystemState::wlanKillswitchChanged() {
    printf("Wlan killswitch changed to: %d\n", wlankill->state());
}

void SystemState::nmStateChanged(uint new_state) {
    if(verbose) {
        printf("NetworkManager state changed to: %d\n", new_state);
    }
}

void SystemState::nmPropertiesChanged(const QVariantMap &props) {
    const static QString activeConnections("ActiveConnections");
    const static QString primaryConnectionType("PrimaryConnectionType");

    auto i = props.find(activeConnections);
    if(i != props.end()) {
        activeConnectionsChanged(*i);
    }
    i = props.find(primaryConnectionType);
    if(i != props.end()) {
        primaryConnectionTypeChanged(i->toString());
    }
    if(verbose) {
        printf("NetworkManager properties changed:\n");
        for(const auto &p : props.keys()) {
            if(p == activeConnections || p == primaryConnectionType)
                continue;
            printf("  %s %s\n", p.toUtf8().data(), props[p].toString().toUtf8().data());
        }
    }
}

void SystemState::primaryConnectionTypeChanged(const QString &type) {
    if(type.isEmpty()) {
        printf("Primary connection is empty. No connectivity.\n");
    } else if(type == "802-11-wireless") {
        printf("Primary wonnection is wireless.\n");
    } else if(type == "gsm") {
        printf("Primary connection is modem.\n");
        auto paths = nmroot->activeConnections();
        for(const auto &path : paths) {
            NetworkManagerActiveConnection ac(NM_SERVICE, path.path(), QDBusConnection::systemBus(), nullptr);
            for(const auto &devpath : ac.devices()) {
                NetworkManagerDevice curdev(NM_SERVICE, devpath.path(), QDBusConnection::systemBus(), nullptr);
                if(curdev.deviceType() == 8) {
                    // Interface is not in the introspection data for some reason so do this the
                    // hard way.
                    QDBusInterface iface(NM_SERVICE, devpath.path(), "org.freedesktop.DBus.Properties",
                                    QDBusConnection::systemBus());
                    QDBusReply<QDBusVariant> ifaceReply = iface.call("Get",
                            "org.freedesktop.NetworkManager.Device", "Interface");
                    auto ofonodev = ifaceReply.value().variant().toString();
                    OfonoModemSimManager man(OFONO_SERVICE, ofonodev, QDBusConnection::systemBus(), nullptr);
                    OfonoModemNetworkRegistration netreg(OFONO_SERVICE, ofonodev, QDBusConnection::systemBus(), nullptr);
                    OfonoModemConnectionManager connmgr(OFONO_SERVICE, ofonodev, QDBusConnection::systemBus(), nullptr);
                    auto connprops = connmgr.GetProperties().value();
                    auto regprops = netreg.GetProperties().value();
                    auto bearer = connprops["Bearer"].toString();
                    if(bearer.isEmpty()) {
                        bearer = "unknown";
                    }
                    QString roaming = regprops["Status"].toString() == "roaming" ? "(roaming)" : "";
                    QString op = regprops["Name"].toString();
                    printf(" technology: %s\n", bearer.toStdString().c_str());
                    printf(" operator: %s %s\n", op.toStdString().c_str(), roaming.toStdString().c_str());
                    return; // Again assume only one modem connection.
                }
            }
        }
    } else {
        printf("Primary connection is of unknown type.\n");
    }
}

void SystemState::activeConnectionsChanged(const QVariant &list) {
    // list.value<QList<QDBusObjectPath>>() does not work for some reason
    // so grab again over dbus. Yes it sucks.
    auto activeConnections = nmroot->activeConnections();
    bool wireless_found = false;
    for(const auto &path : activeConnections) {
        NetworkManagerActiveConnection ac(NM_SERVICE, path.path(), QDBusConnection::systemBus(), nullptr);
        for(const auto &devpath : ac.devices()) {
            NetworkManagerDevice curdev(NM_SERVICE, devpath.path(), QDBusConnection::systemBus(), nullptr);
            auto curdev_type = curdev.deviceType();

            if(curdev_type == 2) {
                if(!activeWirelessDevice || activeWirelessDevice->path() != devpath.path()) {
                    delete activeWirelessDevice;
                    activeWirelessDevice = new NetworkManagerWirelessDevice(NM_SERVICE, devpath.path(),
                            QDBusConnection::systemBus(), this);
                    connect(activeWirelessDevice, SIGNAL(AccessPointAdded(const QDBusObjectPath &)),
                            this, SLOT(activeApListChanged(const QDBusObjectPath &)));
                    connect(activeWirelessDevice, SIGNAL(AccessPointRemoved(const QDBusObjectPath &)),
                            this, SLOT(activeApListChanged(const QDBusObjectPath &)));
                    connect(activeWirelessDevice, SIGNAL(PropertiesChanged(const QVariantMap&)),
                            this, SLOT(activeWirelessDevicePropertyChanged(const QVariantMap&)));
                    return;
                }
                wireless_found = true;
                break;
            }
        }
    }

    if(!wireless_found && activeWirelessDevice) {
        delete activeWirelessDevice;
        activeWirelessDevice = nullptr;
    }
    if(verbose) {
        printf("ActiveConnections changed (now have %d)\n", activeConnections.size());
    }
}

void SystemState::activeApListChanged(const QDBusObjectPath &) {
    printf("An access point appeared or disappeared.\n");
    rebuildApList();
    printApList();
    reconnectNmApSignals();
}

void SystemState::activeWirelessDevicePropertyChanged(const QVariantMap &props) {
    if(props.find("ActiveAccessPoint") != props.end()) {
        printf("Active access point changed.\n");
        rebuildApList();
        printApList();
        reconnectNmApSignals();
    } else if(verbose) {
        for(const auto &p : props.keys()) {
            printf("Active wireless device property %s changed to %s.\n", p.toUtf8().data(),
                    props[p].toString().toUtf8().data());
        }
    }
}


void SystemState::modemPropertyChanged(const int modem, const QString &name, const QDBusVariant &value) {
    auto str = value.variant().toString();
    if(str.length() > 0) {
        str = " to " + str;
    }
    if(verbose) {
        printf("Ofono modem %d property %s changed%s.\n", modem, name.toUtf8().data(), str.toUtf8().data());
    }
}

void SystemState::simmanPropertyChanged(const int simman, const QString &name, const QDBusVariant &value) {
    if(name == "PinRequired") {
        auto newval = value.variant().toString();
        modemStates[simman].simlock = newval;
        printf("Ofono modem %d pin required changed to %s.\n", simman, newval.toUtf8().data());
    }
    if(verbose) {
        printf("Ofono simmanger %d property %s changed to %s.\n", simman,
                name.toUtf8().data(), value.variant().toString().toUtf8().data());
    }
}

void SystemState::netregPropertyChanged(const int netreg, const QString &name, const QDBusVariant &value) {
    QString valuetext;
    if(name == "Strength") {
        auto newstrength = value.variant().toInt();
        modemStates[netreg].strength = newstrength;
        printf("Modem %d strength changed to %d.\n", netreg, newstrength);
    } else if(name == "Name") {
        auto newName = value.variant().toString();
        modemStates[netreg].opname = newName;
        printf("Modem %d operator changed to %s.\n", netreg, newName.toUtf8().data());
    } else if(name == "Status") {
        auto newName = value.variant().toString();
        modemStates[netreg].status = newName;
        printf("Modem %d status changed to %s.\n", netreg, newName.toUtf8().data());
    } else if(verbose) {
        printf("Ofono network registration %d property %s changed to %s.\n",
                netreg, name.toUtf8().data(), valuetext.toUtf8().data());
    }
}

void SystemState::operatorPropertyChanged(const int op, const QString &name, const QDBusVariant &value) {
    printf("Ofono operator %d property %s changed to %s.\n", op, name.toUtf8().data(), value.variant().toString().toUtf8().data());
}

void SystemState::connmgrPropertyChanged(const int cm, const QString &name, const QDBusVariant &value) {
    if(name == "Bearer") {
        auto newBearer = value.variant().toString();
        if(newBearer.isEmpty()) {
            newBearer = "unknown";
        }
        modemStates[cm].technology = newBearer;
        printf("Modem %d technology changed to %s.\n", cm, newBearer.toUtf8().data());
    } else if(verbose) {
        printf("Ofono connectionmanager %d property %s changed to %s.\n",
                cm, name.toUtf8().data(), value.variant().toString().toUtf8().data());
    }
}

void SystemState::connect_wlan(int value) {
    if(value < 0 || value >= (int)aps.size()) {
        fprintf(stderr, "Tried to activate an invalid network.\n");
    }
    for(const auto &acpath : nmroot->activeConnections()) {
        NetworkManagerActiveConnection ac(NM_SERVICE, acpath.path(), QDBusConnection::systemBus(), nullptr);
        auto connpath = ac.connection();
        for(const auto &devpath : ac.devices()) {
            NetworkManagerDevice curdev(NM_SERVICE, devpath.path(), QDBusConnection::systemBus(), nullptr);
            auto curdev_type = curdev.deviceType();

            if(curdev_type == 2) {
                auto res = nmroot->ActivateConnection(connpath, devpath, aps[value].path);
                res.waitForFinished();
                if(!res.isValid()) {
                    fprintf(stderr, "Activating WLAN (%s, %s, %s) failed: %s\n",
                            connpath.path().toStdString().c_str(),
                            devpath.path().toStdString().c_str(),
                            aps[value].path.path().toStdString().c_str(),
                            res.error().message().toUtf8().data());
                }
                return;
            }
        }
    }
}

QStringList SystemState::get_wlans() {
    QStringList result;
    for(const auto &i : aps) {
        std::string tmp(i.ssid.begin(), i.ssid.end()); // TERRIBLE. Fix properly.
        QString repr(tmp.c_str());
        repr += " ";
        repr += std::to_string(i.strength).c_str();
        repr += " ";
        repr += std::to_string(i.securityFlags).c_str();
        if(i.connected) {
            repr += " connected";
        }
        result << repr;
    }
    return result;
}

QStringList SystemState::get_modems() {
    QStringList result;
    for(auto const & m : modemStates) {
        QString repr(m.path.path());
        repr += " " + m.opname;
        repr += " " + m.simlock;
        repr += " " + m.status;
        repr += " " + m.technology;
        repr += " " + QString(std::to_string(m.strength).c_str());
        result << repr;
    }
    return result;
}

void SystemState::set_bt_killswitch(int value) {
    if(value < 0 || value > 1) {
        fprintf(stderr, "Tried to set bt killswitch to bad value.\n");
        return;
    }
    auto res = urfkill->Block(2, value);
    res.waitForFinished();
    if(!res.isValid()) {
        fprintf(stderr, "Error setting bt killswitch: %s\n", res.error().message().toUtf8().data());
    }
    if(!res.value()) {
        fprintf(stderr, "Setting bt killswitch failed for an unknown reason.\n");
    }
}

void SystemState::flightModeCallFinished(QDBusPendingCallWatcher *call) {
    if(!call->isValid()) {
        fprintf(stderr, "Toggling flight mode failed: %s.\n", call->error().message().toUtf8().data());
        flightModeStatus = PENDING_TO_ON ? PENDING_OFF : PENDING_ON;
        // FIXME, emit signal here.
    }
    // Note that we don't set flight mode if the call succeeded. Urfkill will send us a signal when the
    // change actually takes place so do it then.
    call->deleteLater();
}

// Switching to flight mode takes forever in the background, so
// we need to fake it.
void SystemState::set_flightmode(int value) {
    bool newArgumentValue;

    if(value < 0 || value > 1) {
        fprintf(stderr, "Invalid value of flightmode: %d\n", value);
        return;
    }
    if(queuedChange != PENDING_UNKNOWN) {
        queuedChange = (PendingChange)value;
        return;
    }
    if(value == flightModeStatus) {
        return;
    }
    if(value == 1) {
        if(flightModeStatus == PENDING_TO_ON) {
            return;
        }
        newArgumentValue = true;
        printf("Starting flight mode activation.\n");
    } else {
        if(flightModeStatus == PENDING_TO_OFF) {
            return;
        }
        newArgumentValue = false;
        printf("Starting flight mode deactivation.\n");
    }
    auto reply = urfkill->FlightMode(newArgumentValue);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(flightModeCallFinished(QDBusPendingCallWatcher*)));
}


void SystemState::set_wlan_killswitch(int value) {
    if(value < 0 || value > 1) {
        fprintf(stderr, "Tried to set wlan killswitch to bad value.\n");
        return;
    }
    auto res = urfkill->Block(1, value);
    res.waitForFinished();
    if(!res.isValid()) {
        fprintf(stderr, "Error setting wlan killswitch: %s\n", res.error().message().toUtf8().data());
    }
    if(!res.value()) {
        fprintf(stderr, "Setting wlan killswitch failed for an unknown reason.\n");
    }
}

void SystemState::unlock_sim(int value) {
    printf("Should poke whatever it is that pops up the SIM unlock dialog now.\n");
}

void SystemState::reexport() {
    exporter->exportIndicator(modemStates, aps);
}
