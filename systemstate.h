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

#pragma once

#include"coredefs.h"
#include<QObject>
#include<QDBusObjectPath>
#include<QDBusPendingCallWatcher>
#include<QVector>
#include<QDBusObjectPath>
#include<QByteArray>

class UrfkillRoot;
class UrfkillSwitch;
class NetworkManagerRoot;
class NetworkManagerAccessPoint;
class NetworkManagerWirelessDevice;
class OfonoModemModem;
class OfonoModemSimManager;
class OfonoModemNetworkRegistration;
class OfonoNetworkOperator;
class OfonoModemConnectionManager;
class NumberAdder;
class IndicatorExporter;

/*
 * This class takes in all events that happen and maintains the global state
 * of the system. It's a bit big, but unfortunately that is the nature
 * of this particular beast.
 */

enum PendingChange {
    PENDING_OFF,
    PENDING_ON,
    PENDING_TO_OFF,
    PENDING_TO_ON,
    PENDING_UNKNOWN,
};

class SystemState : public QObject {
    Q_OBJECT

public:
    explicit SystemState(IndicatorExporter *exporter, QObject *parent = nullptr, bool verbose=false);
    ~SystemState();

public Q_SLOTS:
    // These come from ofono, networkmanager etc
    void flightModeChanged(bool new_mode);
    void btKillswitchChanged();
    void wlanKillswitchChanged();
    void nmStateChanged(uint new_state);
    void nmPropertiesChanged(const QVariantMap &props);

    void modemPropertyChanged(const int modem, const QString &name, const QDBusVariant &value);
    void simmanPropertyChanged(const int simman, const QString &name, const QDBusVariant &value);
    void netregPropertyChanged(const int modem, const QString &name, const QDBusVariant &value);
    void operatorPropertyChanged(const int op, const QString &name, const QDBusVariant &value);
    void connmgrPropertyChanged(const int mgr, const QString &name, const QDBusVariant &value);

    void nmApPropertiesChanged(const QVariantMap &);

    // These come from the system and are commands to do something.
    void connect_wlan(int value);
    QStringList get_modems();
    QStringList get_wlans();
    void set_bt_killswitch(int value);
    void set_flightmode(int value);
    void set_wlan_killswitch(int value);
    void unlock_sim(int value);

private Q_SLOTS:

    void flightModeCallFinished(QDBusPendingCallWatcher *call);
    void activeApListChanged(const QDBusObjectPath &);
    void activeWirelessDevicePropertyChanged(const QVariantMap &);

private:

    void activeConnectionsChanged(const QVariant &list);
    void primaryConnectionTypeChanged(const QString &type);

    void rebuildApList();
    void printApList();
    void reconnectNmApSignals();
    OfonoNetworkOperator* createOperator(const QDBusObjectPath &p, NumberAdder *relayer);

    void reexport();

    bool verbose;
    UrfkillRoot *urfkill;
    UrfkillSwitch *btkill;
    UrfkillSwitch *wlankill;
    NetworkManagerRoot *nmroot;
    QVector<OfonoModemModem*> modems;
    QVector<OfonoModemSimManager*> simmanagers;
    QVector<OfonoModemNetworkRegistration*> netregs;
    QVector<OfonoNetworkOperator*> operators;
    QVector<OfonoModemConnectionManager*> connmgrs;
    QVector<NumberAdder*> relayers;
    QVector<NetworkManagerAccessPoint*> apObjects;
    NetworkManagerWirelessDevice *activeWirelessDevice;

    IndicatorExporter *exporter;

    // These are the cached bits of system state that
    // we need to store ourselves. They should not be stored
    // in Qt's container classes because we want to pass them
    // out to non-Qt code.
    PendingChange flightModeStatus;
    std::vector<WifiAccessPoint> aps;
    std::vector<OfonoModem> modemStates;
    PendingChange queuedChange; // For storing state changes that come during transitions (e.g. user furiosly tapping on block button)
};
