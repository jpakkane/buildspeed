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

#include"indicatorexporter.h"
#include<gio/gio.h>
#include<string>
#include<stdexcept>

namespace {

void activateWifiSettings(GSimpleAction *simple, GVariant *parameter, gpointer user_data) {
    printf("Invoke wifi page in system-settings.\n");
}

void activateCellularSettings(GSimpleAction *simple, GVariant *parameter, gpointer user_data) {
    printf("Invoke cellular page in system-settings.\n");
}

void enableWifi(GSimpleAction *action, GVariant *value, gpointer user_data) {
    bool new_state = g_variant_get_boolean(value);
    g_simple_action_set_state(action, g_variant_new_boolean(new_state));
    printf("Set wifi enabled to: %d\n", new_state);
}

void enableFlightMode(GSimpleAction *action, GVariant *value, gpointer user_data) {
    bool new_state = g_variant_get_boolean(value);
    g_simple_action_set_state(action, g_variant_new_boolean(new_state));
    printf("Set cellular enabled to: %d\n", new_state);
}

void connectAP(GSimpleAction *action, GVariant *value, gpointer user_data) {
    printf("Connect to access point.\n");
}


GVariant* buildStatusItem(const char *value) {
    GVariantBuilder innerArrBuilder, builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_TUPLE);
    g_variant_builder_init(&innerArrBuilder, G_VARIANT_TYPE_ARRAY);
    g_variant_builder_add_value(&builder, g_variant_new_string("themed"));
    g_variant_builder_init(&innerArrBuilder, G_VARIANT_TYPE_ARRAY);
    g_variant_builder_add_value(&innerArrBuilder, g_variant_new_string(value));
    g_variant_builder_add_value(&builder, g_variant_builder_end(&innerArrBuilder));
    return g_variant_builder_end(&builder);
}

void warningSilencer(GSimpleActionGroup *simple, GAction *action) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    g_simple_action_group_insert(simple, action);
#pragma GCC diagnostic pop
}

}

struct IndicatorExporter::Private {
    GDBusConnection *conn;
    GSimpleActionGroup *group;
    guint group_num;
    GMenu *menu;
    guint menu_num;

    explicit Private(GDBusConnection *conn) : conn(conn), group(nullptr), group_num(0),
            menu(nullptr), menu_num(0) {
        g_object_ref(conn);
    }

    void exportIndicator(const std::vector<OfonoModem> &modems, const std::vector<WifiAccessPoint> &aps);
    void unexportActions();
    void unexportMenus();
    void buildActions(const std::vector<OfonoModem> &modems, const std::vector<WifiAccessPoint> &aps);
    void buildMenus(const std::vector<OfonoModem> &modems, const std::vector<WifiAccessPoint> &aps);
};

void IndicatorExporter::Private::exportIndicator(const std::vector<OfonoModem> &modems, const std::vector<WifiAccessPoint> &aps) {
    buildActions(modems, aps);
    buildMenus(modems, aps);
}

void IndicatorExporter::Private::unexportActions() {
    if(group) {
        g_dbus_connection_unexport_action_group(conn, group_num);
        g_object_unref(group);
        group = nullptr;
        group_num = 0;
    }
}

void IndicatorExporter::Private::unexportMenus() {
    if(menu) {
        g_dbus_connection_unexport_menu_model(conn, menu_num);
        g_object_unref(menu);
        menu = nullptr;
        menu_num = 0;
    }
}

void IndicatorExporter::Private::buildActions(const std::vector<OfonoModem> &modems, const std::vector<WifiAccessPoint> &aps) {
    GError *err = nullptr;
    unexportActions();
    group = g_simple_action_group_new();

    auto a = g_simple_action_new("wifi.settings", nullptr);
    g_signal_connect(G_OBJECT(a), "activate", G_CALLBACK(activateWifiSettings), this);
    warningSilencer(group, G_ACTION(a));

    a = g_simple_action_new("cellular.settings", nullptr);
    g_signal_connect(G_OBJECT(a), "activate", G_CALLBACK(activateCellularSettings), this);
    warningSilencer(group, G_ACTION(a));

    a = g_simple_action_new_stateful("wifi.enable", nullptr, g_variant_new_boolean(true));
    g_signal_connect(a, "change-state", G_CALLBACK(enableWifi), this);
    warningSilencer(group, G_ACTION(a));

    a = g_simple_action_new_stateful("airplane.enabled", nullptr, g_variant_new_boolean(true));
    g_signal_connect(a, "change-state", G_CALLBACK(enableFlightMode), this);
    warningSilencer(group, G_ACTION(a));


    GVariantDict dict;
    g_variant_dict_init(&dict, nullptr);
    GVariantBuilder builder, arrbuilder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_TUPLE);
    g_variant_builder_init(&arrbuilder, G_VARIANT_TYPE_ARRAY);
    g_variant_builder_add_value(&builder, g_variant_new_string("themed"));
    g_variant_builder_add_value(&arrbuilder, g_variant_new_string("nm-signal-75-secure"));
    g_variant_builder_add_value(&builder, g_variant_builder_end(&arrbuilder));
    g_variant_dict_insert_value(&dict, "icon", g_variant_builder_end(&builder));
    g_variant_builder_init(&arrbuilder, G_VARIANT_TYPE_ARRAY);
    g_variant_builder_add_value(&arrbuilder, buildStatusItem("gsm-3g-full"));
    g_variant_builder_add_value(&arrbuilder, buildStatusItem("simcard-locked"));
    g_variant_builder_add_value(&arrbuilder, buildStatusItem("nm-signal-75-secure"));
    g_variant_dict_insert_value(&dict, "icons", g_variant_builder_end(&arrbuilder));

    g_variant_dict_insert_value(&dict, "title", g_variant_new_string("Network"));
    g_variant_dict_insert_value(&dict, "visible", g_variant_new_boolean(TRUE));
    a = g_simple_action_new_stateful("phone.network-status", nullptr, g_variant_dict_end(&dict));
    warningSilencer(group, G_ACTION(a));

    // These don't need callbacks, because they are only for display.
    for(size_t i=0; i<modems.size(); i++) {
        std::string base("modem.");
        base += std::to_string(i+1);
        base += "::";
        bool is_locked = modems[i].simlock != "none";
        auto j = g_simple_action_new_stateful((base + "status-label").c_str(), nullptr,
                g_variant_new_string(modems[i].status.toUtf8().data()));
        warningSilencer(group, G_ACTION(j));

        j = g_simple_action_new_stateful((base + "connectivity-icon").c_str(), nullptr,
                g_variant_new_string(""));
        warningSilencer(group, G_ACTION(j));

        j = g_simple_action_new_stateful((base + "sim-identifier-label").c_str(), nullptr,
                g_variant_new_string(modems[i].opname.toUtf8().data()));
        warningSilencer(group, G_ACTION(j));

        j = g_simple_action_new_stateful((base + "locked").c_str(), nullptr,
                g_variant_new_boolean(is_locked));
        warningSilencer(group, G_ACTION(j));

        j = g_simple_action_new_stateful((base + "status-icon").c_str(), nullptr,
                g_variant_new_string(is_locked ? "simcard-locked" : "gsm-3g-full"));
        warningSilencer(group, G_ACTION(j));

        j = g_simple_action_new_stateful((base + "roaming").c_str(), nullptr,
                g_variant_new_boolean(modems[i].status == "roaming"));
        warningSilencer(group, G_ACTION(j));
    }

    // FIXME store pointers to map.
    for(size_t i=0; i<aps.size(); i++) {
        std::string base("accesspoint.");
        base += std::to_string(i);
        auto j = g_simple_action_new_stateful(base.c_str(), nullptr,
                g_variant_new_boolean(aps[i].connected)); // FIXME
        g_signal_connect(G_OBJECT(j), "activate", G_CALLBACK(connectAP), this);
        warningSilencer(group, G_ACTION(j));
        j = g_simple_action_new_stateful((base + "::strength").c_str(), nullptr,
                g_variant_new_byte(aps[i].strength));
        warningSilencer(group, G_ACTION(j));
    }
    if(err) {
        std::string m(err->message);
        g_error_free(err);
        throw std::runtime_error(m.c_str());
    }

    g_dbus_connection_export_action_group(conn, "/com/canonical/indicator/network", G_ACTION_GROUP(group), &err);
    if(err) {
        std::string m(err->message);
        g_error_free(err);
        throw std::runtime_error(m.c_str());
    }
}

void IndicatorExporter::Private::buildMenus(const std::vector<OfonoModem> &modems, const std::vector<WifiAccessPoint> &aps) {
    GError *err = nullptr;
    unexportMenus();
    menu = g_menu_new();
    auto indMenu = g_menu_new();
    auto i = g_menu_item_new("Flight mode", "indicator.airplane.enabled");
    g_menu_item_set_attribute_value(i, "x-canonical-type", g_variant_new_string("com.canonical.indicator.switch"));
    g_menu_append_item(indMenu, i);

    // Modem section.
    auto modemmenu = g_menu_new();
    for(size_t i=0; i<modems.size(); i++) {
        std::string base("indicator.modem.");
        base += std::to_string(i+1) + "::";
        auto mi = g_menu_item_new(nullptr, nullptr);
        g_menu_item_set_attribute_value(mi, "x-canonical-modem-connectivity-icon-action",
                g_variant_new_string((base + "connectivity-icon").c_str()));
        g_menu_item_set_attribute_value(mi, "x-canonical-modem-sim-identifier-label-action",
                g_variant_new_string((base + "sim-identifier-label").c_str()));
        g_menu_item_set_attribute_value(mi, "x-canonical-modem-status-icon-action",
                g_variant_new_string((base + "status-icon").c_str()));
        g_menu_item_set_attribute_value(mi, "x-canonical-modem-locked-action",
                g_variant_new_string((base + "locked").c_str()));
        g_menu_item_set_attribute_value(mi, "x-canonical-modem-connectivity-status-label-action",
                g_variant_new_string((base + "status-label").c_str()));
        g_menu_item_set_attribute_value(mi, "x-canonical-type",
                g_variant_new_string("com.canonical.indicator.network.modeminfoitem"));
        g_menu_item_set_attribute_value(mi, "x-canonical-modem-roaming-action",
                g_variant_new_string((base + "roaming").c_str()));
        g_menu_append_item(modemmenu, mi);
    }
    i = g_menu_item_new("Cellular settings", "indicator.cellular.settings");
    g_menu_append_item(modemmenu, i);
    g_menu_append_section(indMenu, nullptr, G_MENU_MODEL(modemmenu));

    i = g_menu_item_new("Wifi", "indicator.wifi.enable");
    g_menu_item_set_attribute_value(i, "x-canonical-type", g_variant_new_string("com.canonical.indicator.switch"));
    g_menu_append_item(indMenu, i);

    // Access points.
    auto apmenu = g_menu_new();
    for(size_t i=0; i<aps.size(); i++) {
        std::string visible_name(aps[i].ssid.begin(), aps[i].ssid.end());
        std::string base("indicator.accesspoint." + std::to_string(i));

        auto ai = g_menu_item_new(visible_name.c_str(), base.c_str());
        g_menu_item_set_attribute_value(ai, "x-canonical-wifi-ap-is-secure",
                g_variant_new_boolean(aps[i].securityFlags != 0));
        g_menu_item_set_attribute_value(ai, "x-canonical-wifi-ap-strength-action",
                g_variant_new_string((base + "::strength").c_str()));
        g_menu_item_set_attribute_value(ai, "x-canonical-wifi-is-adhoc",
                g_variant_new_boolean(FALSE));
        g_menu_item_set_attribute_value(ai, "x-canonical-type",
                g_variant_new_string("unity.widgets.systemsettings.tablet.accesspoint"));
        g_menu_append_item(apmenu, ai);
    }
    g_menu_append_section(indMenu, nullptr, G_MENU_MODEL(apmenu));

    i = g_menu_item_new_submenu(nullptr, G_MENU_MODEL(indMenu));
    g_menu_item_set_detailed_action(i, "indicator.phone.network-status");
    g_menu_item_set_attribute_value(i, "x-canonical-type", g_variant_new_string("com.canonical.indicator.root"));
    g_menu_append_item(menu, i);
    menu_num = g_dbus_connection_export_menu_model(conn, "/com/canonical/indicator/network/phone",
            G_MENU_MODEL(menu), &err);
    if(err) {
        std::string m(err->message);
        g_error_free(err);
        throw std::runtime_error(m.c_str());
    }
}

IndicatorExporter::IndicatorExporter(GDBusConnection *conn) : p(new IndicatorExporter::Private(conn)) {

}

IndicatorExporter::~IndicatorExporter() {
    delete p;
}
void IndicatorExporter::exportIndicator(const std::vector<OfonoModem> &modems, const std::vector<WifiAccessPoint> &aps) {
    p->exportIndicator(modems, aps);
}
