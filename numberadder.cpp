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

#include"numberadder.h"

NumberAdder::NumberAdder(int number, QObject *parent) : QObject(parent), number(number) {
}

NumberAdder::~NumberAdder() {
}

void NumberAdder::modemPropertyChanged(const QString name, const QDBusVariant value) {
    Q_EMIT modemPropertyChanged(number, name, value);
}

void NumberAdder::simmanPropertyChanged(const QString name, const QDBusVariant value) {
    Q_EMIT simmanPropertyChanged(number, name, value);
}

void NumberAdder::netregPropertyChanged(const QString name, const QDBusVariant value) {
    Q_EMIT netregPropertyChanged(number, name, value);
}

void NumberAdder::operatorPropertyChanged(const QString name, const QDBusVariant value) {
    Q_EMIT operatorPropertyChanged(number, name, value);
}

void NumberAdder::connmgrPropertyChanged(const QString name, const QDBusVariant value) {
    Q_EMIT connmgrPropertyChanged(number, name, value);
}
