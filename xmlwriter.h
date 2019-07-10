#ifndef XMLWRITER_H
#define XMLWRITER_H

#include "ikeyvaluewriter.h"

class QXmlStreamWriter;

class XMLWriter : public IKeyValueWriter
{
public:
    XMLWriter(const QString &topElementName);
    virtual ~XMLWriter();

    virtual void addDataFromMap(const QString &key, const QVariantMap &map);
    virtual void addDataFromArray(const QString &key, const QString &elementKey, const QList<QVariantMap> &array);
    virtual QByteArray write();

private:
    QByteArray _buffer;
    QXmlStreamWriter *_xml;
};

#endif // XMLWRITER_H
