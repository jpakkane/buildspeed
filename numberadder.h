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

// This class relays signals and adds a number to identify which
// source object sent it.

class NumberAdder : public QObject {
    Q_OBJECT

public:
    explicit NumberAdder(int number, QObject *parent=nullptr);
    virtual ~NumberAdder();

public Q_SLOTS:

    void modemPropertyChanged(const QString name, const QDBusVariant value);
    void simmanPropertyChanged(const QString name, const QDBusVariant value);
    void netregPropertyChanged(const QString name, const QDBusVariant value);
    void operatorPropertyChanged(const QString name, const QDBusVariant value);
    void connmgrPropertyChanged(const QString name, const QDBusVariant value);

Q_SIGNALS:

    void modemPropertyChanged(int sourceNum, const QString &name, const QDBusVariant &value);
    void simmanPropertyChanged(int sourceNum, const QString &name, const QDBusVariant &value);
    void netregPropertyChanged(int sourceNum, const QString &name, const QDBusVariant &value);
    void operatorPropertyChanged(int sourceNum, const QString &name, const QDBusVariant &value);
    void connmgrPropertyChanged(int sourceNum, const QString &name, const QDBusVariant &value);

private:
    int number;
};
