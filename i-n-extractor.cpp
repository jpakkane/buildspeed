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


#include<gio/gio.h>

#include<QCoreApplication>
#include"urfkillroot.h"
#include"urfkillswitch.h"
#include"nmroot.h"
#include"nmsettings.h"
#include"nmconnsettings.h"
#include"nmactiveconnection.h"
#include"nmwirelessdevice.h"
#include"nmaccesspoint.h"
#include"nmdevice.h"
#include"nmmodem.h"
#include"ofonoroot.h"
#include"ofonomodemsimmanager.h"
#include"ofonomodemnetworkregistration.h"
#include"ofononetworkoperator.h"
#include"ofonomodemconnectionmanager.h"
#include"eventprinter.h"
#include"systemstate.h"
#include"dbusdata.h"
#include"indicatorexporter.h"

static std::map<const int, const char*> nm_active_connection_strings = {{0, "Unknown"}, {1, "Activating"},
        {2, "Activated"}, {3, "Deactivating"}, {4, "Deactivated"}};
static std::map<const int, const char*> nm_connectivity_strings = {{0, "Unknown"}, {1, "None"}, {2, "Portal"},
        {3, "Limited"}, {4, "Full"}};
static std::map<const int, const char*> nm_state_strings = {{0, "Unknown"}, {10, "Asleep"}, {20, "Disconnected"},
    {30, "Disconnecting"}, {40, "Connecting"}, {50, "Connected local"}, {60, "Connected site"}, {70, "Connected global"}};
static std::map<const int, const char*> nm_ap_mode_strings = {{0, "Unknown"}, {1, "Adhoc"}, {2, "Infra"}, {3, "Access point"}};

/*
 * Bearer to icon type:
 * "none", "gprs" ( 2G ), "edge" ( E ), "umts" ( 3G ), "hsupa"/"hsdpa"/"hspa" ( H ), and "lte" ( L )
 */

void print_nm_wlans(NetworkManagerRoot *nmroot) {
    auto paths = nmroot->activeConnections();
    for(const auto &path : paths) {
        NetworkManagerActiveConnection ac(NM_SERVICE, path.path(), QDBusConnection::systemBus(), nullptr);
        for(const auto &devpath : ac.devices()) {
            NetworkManagerDevice curdev(NM_SERVICE, devpath.path(), QDBusConnection::systemBus(), nullptr);
            auto curdev_type = curdev.deviceType();

            if(curdev_type == 1) {
                printf("Device %s is wired ethernet.\n", devpath.path().toUtf8().data());
            } else if(curdev_type == 2) {
                NetworkManagerWirelessDevice dev(NM_SERVICE, devpath.path(), QDBusConnection::systemBus(), nullptr);
                auto reply = dev.GetAccessPoints();
                reply.waitForFinished();
                if(reply.isValid()) {
                    // FIXME and the same here.
                    // FIXME also that active access point might not exist
                    NetworkManagerAccessPoint ap(NM_SERVICE, dev.activeAccessPoint().path(), QDBusConnection::systemBus(), nullptr);
                    printf("Connection %s is wlan:\n", path.path().toUtf8().data());
                    auto raw_ssid = ap.ssid();
                    std::string ssid(raw_ssid.cbegin(), raw_ssid.cend());
                    printf("  ssid: %s\n", ssid.c_str());
                    printf("  mode: %d (%s)\n", ap.mode(), nm_ap_mode_strings[ap.mode()]);
                    printf("  visible networks:\n");
                    for(const auto &c : reply.value()) {
                        NetworkManagerAccessPoint i(NM_SERVICE, c.path(), QDBusConnection::systemBus(), nullptr);
                        auto i_raw_ssid = i.ssid();
                        std::string i_ssid(i_raw_ssid.cbegin(), i_raw_ssid.cend());
                        std::string encrypts(" ");
                        auto rsnflags = i.rsnFlags();
                        if(rsnflags == 0) {
                            rsnflags = i.wpaFlags(); // NM seems to fill either of these at random.
                        }
                        if(rsnflags & 0x4) {
                            encrypts += "WPA ";
                        }
                        if(rsnflags & 0x8) {
                            encrypts += "WPA2 ";
                        }
                        if(rsnflags & 0x200) {
                            encrypts += "WPA2_ENT ";
                        }
                        if(rsnflags & 0x1) {
                            encrypts += "WEP40 ";
                        }
                        if(rsnflags & 0x2) {
                            encrypts += "WEP104 ";
                        }
                    printf("    %s%s%d (encryption flags %u)\n", i_ssid.c_str(), encrypts.c_str(), (int)i.strength(),
                            rsnflags);
                    }
                }
            } else if(curdev_type == 8) {
                printf("Device %s is a modem.\n", devpath.path().toUtf8().data());
                NetworkManagerModem m(NM_SERVICE, devpath.path(), QDBusConnection::systemBus(), nullptr);
                printf(" current capabilities: %d\n", m.currentCapabilities());
                printf(" modem capabilities: %d\n", m.modemCapabilities());
            } else {
                printf("Device %s is of unknown type.\n", devpath.path().toUtf8().data());
            }
            // Interface is not in the introspection data for some reason so do this the
            // hard way.
            QDBusInterface iface(NM_SERVICE, devpath.path(), "org.freedesktop.DBus.Properties",
                            QDBusConnection::systemBus());
            QDBusReply<QDBusVariant> ifaceReply = iface.call("Get",
                    "org.freedesktop.NetworkManager.Device", "Interface");
            if(ifaceReply.isValid()) {
                auto dev_iface = ifaceReply.value().variant().toString();
                printf(" interface: %s\n", dev_iface.toUtf8().data());
            }
        }
    }
}

void print_info(QCoreApplication &app) {
    UrfkillRoot *urfkill = new UrfkillRoot(URFKILL_SERVICE, URFKILL_OBJECT,
            QDBusConnection::systemBus(), &app);
    UrfkillSwitch *urfkillwlan = new UrfkillSwitch(URFKILL_SERVICE, URFKILL_WLAN_OBJECT,
            QDBusConnection::systemBus(), &app);
    UrfkillSwitch *urfkillbt = new UrfkillSwitch(URFKILL_SERVICE, URFKILL_BLUETOOTH_OBJECT,
            QDBusConnection::systemBus(), &app);
    UrfkillSwitch *urfkillgps = new UrfkillSwitch(URFKILL_SERVICE, URFKILL_GPS_OBJECT,
            QDBusConnection::systemBus(), &app);
    NetworkManagerRoot *nmroot = new NetworkManagerRoot(NM_SERVICE, NM_OBJECT,
            QDBusConnection::systemBus(), &app);
    OfonoRoot *ofonoroot = new OfonoRoot(OFONO_SERVICE, OFONO_OBJECT,
            QDBusConnection::systemBus(), &app);

    printf("Urfkill flightmode: %d\n", urfkill->IsFlightMode().value());
    printf("Urfkill WLAN killswitch: %d\n", urfkillwlan->state());
    printf("Urfkill BLUETOOTH killswitch: %d\n", urfkillbt->state());
    printf("Urfkill GPS killswitch: %d\n", urfkillgps->state());
    printf("\n");
    auto nm_connectivity = nmroot->connectivity();
    printf("NetworkManager connectivity: %d (%s)\n", nm_connectivity, nm_connectivity_strings[nm_connectivity]);
    auto nm_state = nmroot->state().value();
    printf("NetworkManager primary connection: %s\n", nmroot->primaryConnection().path().toUtf8().data());
    printf("NetworkManager state: %d (%s).\n", nm_state, nm_state_strings[nm_state]);
    printf("NetworkManager networking enabled: %d\n", nmroot->networkingEnabled());
    printf("NetworkManager wireless enabled: %d\n", nmroot->wirelessEnabled());
    printf("NetworkManager wireless hardware enabled: %d\n", nmroot->wirelessHardwareEnabled());
    print_nm_wlans(nmroot);
    printf("\n");
    auto modems = ofonoroot->GetModems().value();

    printf("Ofono modem count: %d\n", modems.size());
    for(const auto &m : modems) {
        OfonoModemSimManager man(OFONO_SERVICE, m.first.path(), QDBusConnection::systemBus(), nullptr);
        OfonoModemNetworkRegistration netreg(OFONO_SERVICE, m.first.path(), QDBusConnection::systemBus(), nullptr);
        OfonoModemConnectionManager connmgr(OFONO_SERVICE, m.first.path(), QDBusConnection::systemBus(), nullptr);
        auto mprops = man.GetProperties().value();
        auto regprops = netreg.GetProperties().value();
        const auto &props = m.second;
        const auto operators = netreg.GetOperators().value();
        auto connprops = connmgr.GetProperties().value();

        printf("Modem %s:\n", m.first.path().toUtf8().data());
        // For proper usage should check for existance before indexing.
        printf(" Features: ");
        for(const auto &f : props["Features"].toStringList()) {
            printf("%s ", f.toUtf8().data());
        }
        printf("\n");
        printf(" Powered: %d\n", props["Powered"].toBool());
        printf(" Online: %d\n", props["Online"].toBool());
        printf(" Model: %s\n", props["Model"].toString().toUtf8().data());
        printf(" Bearer: %s\n", connprops["Bearer"].toString().toUtf8().data());
        printf(" Manufacturer: %s\n", props["Manufacturer"].toString().toUtf8().data());
        printf(" Pin required: %s\n", mprops["PinRequired"].toString().toUtf8().data());
        printf(" Status: %s\n", regprops["Status"].toString().toUtf8().data());
        printf(" Strength: %d\n", regprops["Strength"].toInt());
        // Fixme, should check all operators.
        if(!operators.empty()) {
            printf(" Operator: %s\n", regprops["Name"].toString().toUtf8().data());
            OfonoNetworkOperator op(OFONO_SERVICE, operators[0].first.path(), QDBusConnection::systemBus(), nullptr);
            auto opprops = op.GetProperties().value();
            printf("  Technologies: ");
            for(const auto &f : opprops["Technologies"].toStringList()) {
                printf("%s ", f.toUtf8().data());
            }
            printf("\n");
        } else {
            printf(" Operator: none\n");
        }
    }

}

int run_daemon(QCoreApplication &app, bool verbose) {
    new EventPrinter(&app, verbose);
    return app.exec();
}

void
on_bus_acquired (GDBusConnection *connection,
                 const gchar     */*name*/,
                 gpointer         user_data) {
    auto app = static_cast<QCoreApplication*>(user_data);
    auto exporter = new IndicatorExporter(connection); // FIXME, leaks.
    auto ss = new SystemState(exporter, app);
    // Get rid of these eventually.
    if(!QDBusConnection::sessionBus().registerService("com.example.network")) {
        printf("Service name already taken.\n");
        app->exit(1);
    }
    if(!QDBusConnection::sessionBus().registerObject("/network", ss)) {
        printf("Could not register to DBus session.\n");
        app->exit(1);
    }
}

static void
on_name_acquired (GDBusConnection */*connection*/,
                  const gchar     */*name*/,
                  gpointer         /*user_data*/
                  ) {
}

static void
on_name_lost (GDBusConnection */*connection*/,
              const gchar     */*name*/,
              gpointer         user_data
              ) {
    printf("Lost dbus name. Exiting.\n");
    static_cast<QCoreApplication*>(user_data)->exit(1);
}

int run_indicator(QCoreApplication &app) {
    g_bus_own_name (G_BUS_TYPE_SESSION,
                    "com.canonical.indicator.network",
                    G_BUS_NAME_OWNER_FLAGS_NONE,
                    on_bus_acquired,
                    on_name_acquired,
                    on_name_lost,
                    &app,
                    nullptr);
    return app.exec();
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    qRegisterMetaType<QVariantDictMap>("QVariantDictMap");
    qDBusRegisterMetaType<QVariantDictMap>();
    qRegisterMetaType<ModemPropertyList>("ModemPropertyList");
    qDBusRegisterMetaType<ModemPropertyList>();
    qDBusRegisterMetaType<QPair<QDBusObjectPath, QVariantMap>>();

    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption daemonMode("d", "Daemon mode, print events as they happen.");
    parser.addOption(daemonMode);
    QCommandLineOption verboseMode("v", "Verbose mode, print all events in daemon mode.");
    parser.addOption(verboseMode);
    QCommandLineOption indicatorMode("q", "Experimental indicator mode.");
    parser.addOption(indicatorMode);

    parser.process(app);
    if(parser.isSet(daemonMode)) {
        return run_daemon(app, parser.isSet(verboseMode));
    } else if(parser.isSet(indicatorMode)) {
        return run_indicator(app);
    } else {
        print_info(app);
    }
    return 0;
}
