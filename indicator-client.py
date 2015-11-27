#!/usr/bin/python3

# Copyright (C) 2014 Canonical, Ltd.
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranties of
# MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
# PURPOSE.  See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Authors:
#     Jussi Pakkanen <jussi.pakkanen@canonical.com>

import sys

from gi.repository import GObject
from gi.repository import GLib

import dbus
import dbus.service
import dbus.mainloop.glib

from optparse import OptionParser

parser = OptionParser()
parser.add_option('--list-wlans', action='store_true', dest='listwlans', default=False)
parser.add_option('--list-modems', action='store_true', dest='listmodems', default=False)
parser.add_option('--connect-wlan', default=None, dest='connwlan')
parser.add_option('--flightmode', default=None, dest='flightmode')
parser.add_option('--wlankill', default=None, dest='wlankill')
parser.add_option('--btkill', default=None, dest='btkill')
parser.add_option('--unlock-sim', default=None, dest='unlocksim')


def list_wlans(dbusiface):
    for i, w in enumerate(iface.get_wlans()):
        print('%d:' % i, w)

def list_modems(dbusinterface):
    for w in iface.get_modems():
        print(w)

def connect_wlan(dbusobject, value):
    iface.connect_wlan(value)

def set_flightmode(dbusobject, value):
    iface.set_flightmode(value)

def set_wlankill(dbusobject, value):
    iface.set_wlan_killswitch(value)

def set_btkill(dbusobject, value):
    iface.set_bt_killswitch(value)

def unlock_sim(dbusobject, value):
    iface.unlock_sim(value)

if __name__ == '__main__':
    (options, args) = parser.parse_args(sys.argv)
    dbusobject = dbus.SessionBus().get_object('com.example.network', '/network')
    iface = dbus.Interface(dbusobject, "com.example.network")

    if options.listwlans:
        list_wlans(iface)
    if options.listmodems:
        list_modems(iface)
    if options.connwlan is not None:
        connect_wlan(iface, int(options.connwlan))
    if options.flightmode is not None:
        set_flightmode(iface, int(options.flightmode))
    if options.wlankill is not None:
        set_wlankill(iface, int(options.wlankill))
    if options.btkill is not None:
        set_btkill(iface, int(options.btkill))
    if options.unlocksim is not None:
        unlock_sim(iface, int(options.unlocksim))
