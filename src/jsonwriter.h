#ifndef JSONWRITER_H
#define JSONWRITER_H

#include "ikeyvaluewriter.h"

class QJsonObject;

class JSONWriter : public IKeyValueWriter
{
public:
    JSONWriter();
    virtual ~JSONWriter();

    virtual void addDataFromMap(const QString &key, const QVariantMap &map);
    virtual void addDataFromArray(const QString &key, const QString &elementKey, const QList<QVariantMap> &array);
    virtual QByteArray write();

private:
    QJsonObject *_json;
};

#endif // JSONWRITER_H
