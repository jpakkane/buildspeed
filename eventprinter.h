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

#include<QObject>
#include<QDBusVariant>
#include<QVector>

class UrfkillRoot;
class UrfkillSwitch;
class NetworkManagerRoot;
class OfonoModemModem;
class OfonoModemSimManager;
class OfonoModemNetworkRegistration;
class OfonoNetworkOperator;
class OfonoModemConnectionManager;

class EventPrinter : QObject {
    Q_OBJECT

public:
    explicit EventPrinter(QObject *parent = nullptr, bool verbose=false);
    ~EventPrinter();

public Q_SLOTS:
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

private:

    void activeConnectionsChanged(const QVariant &list);
    void primaryConnectionTypeChanged(const QString &type);

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
};
