#ifndef ITEMPARSER_H
#define ITEMPARSER_H

#include "structs.h"

#include <QCoreApplication>


class QDataStream;
class QString;
class ReverseBitReader;

class ItemParser
{
    Q_DECLARE_TR_FUNCTIONS(ItemParser)

public:
    static const QByteArray itemHeader, plugyPageHeader;
    static const QString &enhancedDamageFormat(); // stupid lupdate

    static ItemInfo *parseItem(QDataStream &inputDataStream, const QByteArray &bytes);
    static PropertiesMultiMap parseItemProperties(ReverseBitReader &bitReader, bool *ok);
    static bool itemTypeInheritsFromTypes(const QByteArray &itemType, const QList<QByteArray> &allowedItemTypes);

    static void writeItems(const ItemsList &items, QDataStream &ds);

private:
    static bool itemTypesInheritFromTypes(const QList<QByteArray> &itemTypes, const QList<QByteArray> &allowedItemTypes);
    static QString mysticOrbReadableProperty(const QString &fullDescription);
};

#endif // ITEMPARSER_H
