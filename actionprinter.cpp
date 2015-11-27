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

#include"actionprinter.h"

ActionPrinter::ActionPrinter(QObject *parent) {

}

ActionPrinter::~ActionPrinter() {

}

void ActionPrinter::connect_wlan(int value) {
    printf("Connect to wlan: %d\n", value);
}

QStringList ActionPrinter::get_modems() {
    QStringList result;
    printf("Wlan list requested.\n");
    result << "modem1" << "modem2";
    return result;
}

QStringList ActionPrinter::get_wlans() {
    QStringList result;
    printf("Modem list requested.\n");
    result << "wlan1" << "wlan2";
    return result;
}

void ActionPrinter::set_bt_killswitch(int value) {
    printf("Set bt killswitch to: %d\n", value);
}

void ActionPrinter::set_flightmode(int value) {
    printf("Set flightmode to %d\n", value);
}

void ActionPrinter::set_wlan_killswitch(int value) {
    printf("Set wlan killswitch to %d.\n", value);
}

void ActionPrinter::unlock_sim(int value) {
    printf("Unlock sim number %d.\n", value);
}
