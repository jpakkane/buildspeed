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

#include"eventprinter.h"
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

EventPrinter::EventPrinter(QObject *parent, bool verbose) : QObject(parent), verbose(verbose) {
    urfkill = new UrfkillRoot(URFKILL_SERVICE, URFKILL_OBJECT, QDBusConnection::systemBus(), this);
    btkill = new UrfkillSwitch(URFKILL_SERVICE, URFKILL_BLUETOOTH_OBJECT, QDBusConnection::systemBus(), this);
    wlankill = new UrfkillSwitch(URFKILL_SERVICE, URFKILL_WLAN_OBJECT, QDBusConnection::systemBus(), this);
    nmroot = new NetworkManagerRoot(NM_SERVICE, NM_OBJECT, QDBusConnection::systemBus(), this);

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
            auto oberator = new OfonoNetworkOperator(OFONO_SERVICE, ops[0].first.path(),
                    QDBusConnection::systemBus(), this);
            connect(oberator, SIGNAL(PropertyChanged(const QString, const QDBusVariant)), relayer,
                    SLOT(operatorPropertyChanged(const QString, const QDBusVariant)));
            connect(relayer, SIGNAL(operatorPropertyChanged(int, const QString&, const QDBusVariant&)), this,
                    SLOT(operatorPropertyChanged(int, const QString&, const QDBusVariant&)));
            operators.push_back(oberator);
        }

        auto connmgr = new OfonoModemConnectionManager(OFONO_SERVICE, curModems[0].first.path(), QDBusConnection::systemBus(), this);
        connect(connmgr, SIGNAL(PropertyChanged(const QString, const QDBusVariant)), relayer,
                SLOT(connmgrPropertyChanged(QString, QDBusVariant)));
        connect(relayer, SIGNAL(connmgrPropertyChanged(int, const QString&, const QDBusVariant&)), this,
                SLOT(connmgrPropertyChanged(int, const QString&, const QDBusVariant&)));

        modems.push_back(modem);
        simmanagers.push_back(simman);
        netregs.push_back(netreg);
        connmgrs.push_back(connmgr);
    }
}

EventPrinter::~EventPrinter() {
}

void EventPrinter::flightModeChanged(bool newMode) {
    printf("Flight mode changed to: %d\n", newMode);
}

void EventPrinter::btKillswitchChanged() {
    printf("Bluetooth killswitch changed to: %d\n", btkill->state());
}

void EventPrinter::wlanKillswitchChanged() {
    printf("Wlan killswitch changed to: %d\n", wlankill->state());
}

void EventPrinter::nmStateChanged(uint new_state) {
    printf("NetworkManager state changed to: %d\n", new_state);
}

void EventPrinter::nmPropertiesChanged(const QVariantMap &props) {
    const static QString activeConnections("ActiveConnections");
    const static QString primaryConnectionType("PrimaryConnectionType");
    printf("NetworkManager properties changed:\n");
    for(const auto &p : props.keys()) {
        if(p == activeConnections || p == primaryConnectionType)
            continue;
        printf("  %s %s\n", p.toUtf8().data(), props[p].toString().toUtf8().data());
    }

    auto i = props.find(activeConnections);
    if(i != props.end()) {
        activeConnectionsChanged(*i);
    }
    i = props.find(primaryConnectionType);
    if(i != props.end()) {
        primaryConnectionTypeChanged(i->toString());
    }
}

void EventPrinter::primaryConnectionTypeChanged(const QString &type) {
    printf("NetworkManager primary connection type changed to %s.\n", type.toUtf8().data());
}

void EventPrinter::activeConnectionsChanged(const QVariant &list) {
    // list.value<QList<QDBusObjectPath>>() does not work for some reason
    // so grab again over dbus. Yes it sucks.
    printf("  ActiveConnections (now have %d)\n", nmroot->activeConnections().size());
}

void EventPrinter::modemPropertyChanged(const int modem, const QString &name, const QDBusVariant &value) {
    auto str = value.variant().toString();
    if(str.length() > 0) {
        str = " to " + str;
    }
    printf("Ofono modem %d property %s changed%s.\n", modem, name.toUtf8().data(), str.toUtf8().data());
}

void EventPrinter::simmanPropertyChanged(const int simman, const QString &name, const QDBusVariant &value) {
    if(name == "PinRequired") {
        printf("Ofono modem %d pin required changed to %s.\n", simman, value.variant().toString().toUtf8().data());
    }
}

void EventPrinter::netregPropertyChanged(const int netreg, const QString &name, const QDBusVariant &value) {
    QString valuetext;
    if(name == "Strength") {
        if(!verbose) {
            return;
        }
        valuetext = QString::number(value.variant().toInt());
    } else {
        valuetext = value.variant().toString();
    }

    printf("Ofono network registration %d property %s changed to %s.\n", netreg, name.toUtf8().data(), valuetext.toUtf8().data());
}

void EventPrinter::operatorPropertyChanged(const int op, const QString &name, const QDBusVariant &value) {
    printf("Ofono operator %d property %s changed to %s.\n", op, name.toUtf8().data(), value.variant().toString().toUtf8().data());
}

void EventPrinter::connmgrPropertyChanged(const int cm, const QString &name, const QDBusVariant &value) {
    printf("Ofono connectionmanager %d property %s changed to %s.\n", cm, name.toUtf8().data(), value.variant().toString().toUtf8().data());
}
