#include "xmlwriter.h"

#include <QXmlStreamWriter>

XMLWriter::XMLWriter(const QString &topElementName) : IKeyValueWriter(), _xml(new QXmlStreamWriter(&_buffer))
{
    _xml->setAutoFormatting(true);
    _xml->writeStartDocument();
    _xml->writeStartElement(topElementName);
}

XMLWriter::~XMLWriter()
{
    delete _xml;
}

void XMLWriter::addDataFromMap(const QString &key, const QVariantMap &map)
{
    _xml->writeStartElement(key);
    for (QVariantMap::const_iterator i = map.constBegin(); i != map.constEnd(); ++i)
    {
        const QString &k = i.key();
        const QVariant &v = i.value();
        if (v.canConvert<QVariantList>())
        {
            QList<QVariantMap> list;
            foreach (const QVariant &element, v.toList())
                list += element.toMap();
            addDataFromArray(k, QLatin1String("item"), list);
        }
        else if (v.canConvert<QVariantMap>())
            addDataFromMap(k, v.toMap());
        else
            _xml->writeTextElement(k, v.toString());
    }
    _xml->writeEndElement();
}

void XMLWriter::addDataFromArray(const QString &key, const QString &elementKey, const QList<QVariantMap> &array)
{
    _xml->writeStartElement(key);
    foreach (const QVariantMap &map, array)
        addDataFromMap(elementKey, map);
    _xml->writeEndElement();
}

QByteArray XMLWriter::write()
{
    _xml->writeEndElement();
    _xml->writeEndDocument();
    return _buffer;
}
