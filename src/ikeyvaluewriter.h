#ifndef IDOCUMENTWRITER_H
#define IDOCUMENTWRITER_H

#include <QVariant>

class IKeyValueWriter
{
public:
//    IDocumentWriter() {}

    virtual void addDataFromMap(const QString &key, const QVariantMap &map) = 0;
    virtual void addDataFromArray(const QString &key, const QString &elementKey, const QList<QVariantMap> &array) = 0;
    virtual QByteArray write() = 0;
};

#endif // IDOCUMENTWRITER_H
