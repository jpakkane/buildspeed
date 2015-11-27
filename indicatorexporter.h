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

#include<vector>
#include"coredefs.h"

// This class exports the state of the system over dbus to the indicator gui

typedef struct _GDBusConnection GDBusConnection;

class IndicatorExporter {

public:
    explicit IndicatorExporter(GDBusConnection *conn);
    ~IndicatorExporter();

    void exportIndicator(const std::vector<OfonoModem> &modems, const std::vector<WifiAccessPoint> &aps);

private:

    struct Private;
    Private *p;
};
