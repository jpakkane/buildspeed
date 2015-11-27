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

#include"menuexporter.h"
#include<gio/gio.h>
#include<cstdlib>
#include<cstdio>

MenuExporter *e=nullptr;

void
on_bus_acquired (GDBusConnection *connection,
                 const gchar     */*name*/,
                 gpointer         user_data) {
    e = new MenuExporter(connection);
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
              gpointer         /*user_data*/
              ) {
    printf("Lost dbus name. Exiting.\n");
    exit(1);
}


int main(int argc, char **argv) {
    g_bus_own_name (G_BUS_TYPE_SESSION,
                "com.canonical.indicator.network",
                G_BUS_NAME_OWNER_FLAGS_NONE,
                on_bus_acquired,
                on_name_acquired,
                on_name_lost,
                nullptr,
                nullptr);
    g_main_loop_run(g_main_loop_new(nullptr, FALSE));
    return 0;
}
