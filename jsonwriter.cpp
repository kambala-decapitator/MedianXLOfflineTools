#include "jsonwriter.h"

#if IS_QT5
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#else
#include <qjson-backport/qjsonarray.h>
#include <qjson-backport/qjsondocument.h>
#include <qjson-backport/qjsonobject.h>
#endif

JSONWriter::JSONWriter() : IKeyValueWriter(), _json(new QJsonObject) {}

JSONWriter::~JSONWriter()
{
    delete _json;
}

void JSONWriter::addDataFromMap(const QString &key, const QVariantMap &map)
{
    _json->insert(key, QJsonObject::fromVariantMap(map));
}

void JSONWriter::addDataFromArray(const QString &key, const QString &elementKey, const QList<QVariantMap> &array)
{
    Q_UNUSED(elementKey);
    QJsonArray jsonArray;
    foreach (const QVariantMap &map, array)
        jsonArray.append(QJsonObject::fromVariantMap(map));
    _json->insert(key, jsonArray);
}

QByteArray JSONWriter::write()
{
    return QJsonDocument(*_json).toJson();
}
