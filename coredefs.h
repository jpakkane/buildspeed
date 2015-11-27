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

#include<QString>
#include<QDBusObjectPath>
#include<QByteArray>

struct WifiAccessPoint {
    QDBusObjectPath path;
    QByteArray ssid;
    uchar strength;
    uint securityFlags;
    bool connected;
};

struct OfonoModem {
    QDBusObjectPath path;
    QString opname;
    QString simlock;
    QString technology;
    QString status; // "roaming" means roaming.
    bool online;
    char strength;

};
