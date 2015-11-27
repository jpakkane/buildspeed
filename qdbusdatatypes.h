#pragma once

#include <QDBusMetaType>
#include <QMap>

typedef QMap<QString, QVariantMap> QVariantDictMap;
Q_DECLARE_METATYPE(QVariantDictMap)

typedef QVector<QPair<QDBusObjectPath, QVariantMap>> ModemPropertyList;
Q_DECLARE_METATYPE(ModemPropertyList)

typedef ModemPropertyList OfonoObjectPropertyList;
