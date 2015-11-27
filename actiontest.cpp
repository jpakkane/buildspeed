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

#include<glib.h>
#include<gio/gio.h>
#include<cstdio>
#include<cstdlib>

#define DBUS_PATH "com.example.action"
#define DBUS_OBJECT "/com/example/action"

static void activate_cb(GSimpleAction *simple,
        GVariant      *parameter,
        gpointer       user_data) {
    printf("Activate!\n");
}

static void
on_bus_acquired (GDBusConnection *connection,
                 const gchar     */*name*/,
                 gpointer         user_data)
{
}

static void
on_name_acquired (GDBusConnection *connection,
                  const gchar     */*name*/,
                  gpointer         /*user_data*/
                  )
{
    GSimpleAction *a = g_simple_action_new("test", G_VARIANT_TYPE_STRING);
    GSimpleActionGroup *gr = g_simple_action_group_new();
    GError *err = nullptr;

    g_signal_connect(a, "activate", G_CALLBACK(activate_cb), nullptr);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    g_simple_action_group_insert(gr, G_ACTION(a));
#pragma GCC diagnostic pop
    g_dbus_connection_export_action_group(connection, DBUS_OBJECT, G_ACTION_GROUP(gr), &err);
    if(err) {
        printf("Fail: %s", err->message);
        g_error_free(err);
    }
}

static void
on_name_lost (GDBusConnection */*connection*/,
              const gchar     */*name*/,
              gpointer         /*user_data*/
              )
{
  exit(1);
}

int main(int argc, char **argv) {
    g_bus_own_name (G_BUS_TYPE_SESSION,
                DBUS_PATH,
                G_BUS_NAME_OWNER_FLAGS_NONE,
                on_bus_acquired,
                on_name_acquired,
                on_name_lost,
                nullptr,
                nullptr);
    g_main_loop_run(g_main_loop_new(nullptr, FALSE));
    return 0;
}
