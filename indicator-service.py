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

from gi.repository import GObject
from gi.repository import GLib

import dbus
import dbus.service
import dbus.mainloop.glib

class Indicator(dbus.service.Object):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    @dbus.service.method("com.example.network",
                         in_signature='i', out_signature='')
    def set_flightmode(self, value):
        print("Flight mode set to", value)

    @dbus.service.method("com.example.network",
                         in_signature='', out_signature='as')
    def get_wlans(self):
        print('Wlan list requested')
        return ['wlan1', 'wlan2']

    @dbus.service.method('com.example.network',
                         in_signature='', out_signature='as')
    def get_modems(self):
        print('Modem list requested.')
        return ['modem1', 'modem2']

    @dbus.service.method("com.example.network",
                         in_signature='i', out_signature='')
    def connect_wlan(self, value):
        print('Connect to wlan', value)

    @dbus.service.method("com.example.network",
                         in_signature='i', out_signature='')
    def set_wlan_killswitch(self, value):
        print('Set wlan killswitch to', value)

    @dbus.service.method("com.example.network",
                         in_signature='i', out_signature='')
    def set_bt_killswitch(self, value):
        print('Set bt killswitch to', value)

    @dbus.service.method("com.example.network",
                         in_signature='i', out_signature='')
    def unlock_sim(self, value):
        print('Unlock SIM', value)

if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

    session_bus = dbus.SessionBus()
    name = dbus.service.BusName("com.example.network", session_bus)
    mainloop = GLib.MainLoop()
    indi = Indicator(session_bus, '/network')
    mainloop.run()
