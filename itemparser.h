#ifndef ITEMPARSER_H
#define ITEMPARSER_H

#include "structs.h"

#include <QCoreApplication>


class QDataStream;
class ReverseBitReader;

class ItemParser
{
    Q_DECLARE_TR_FUNCTIONS(ItemParser)

public:
    static const QByteArray kItemHeader, kPlugyPageHeader;
    static const QString &kEnhancedDamageFormat(); // stupid lupdate

    static QString parseItemsToBuffer(quint16 itemsTotal, QDataStream &inputDataStream, const QByteArray &bytes, const QString &corruptedItemFormat, ItemsList *itemsBuffer);
    static ItemInfo *parseItem(QDataStream &inputDataStream, const QByteArray &bytes);
    static PropertiesMultiMap parseItemProperties(ReverseBitReader &bitReader, ItemInfo::ParsingStatus *status);
    static void fixMagicDamageString(ItemProperty *prop, ItemPropertyTxt *txtProp);

    static bool itemTypesInheritFromType(const QList<QByteArray> &itemTypes, const QByteArray &allowedItemType);
    static bool itemTypeInheritsFromTypes(const QByteArray &itemType, const QList<QByteArray> &allowedItemTypes);
    static bool itemTypesInheritFromTypes(const QList<QByteArray> &itemTypes, const QList<QByteArray> &allowedItemTypes);

    static void writeItems(const ItemsList &items, QDataStream &ds);
    static QString itemStorageAndCoordinatesString(const QString &text, ItemInfo *item);

private:
    static QString mysticOrbReadableProperty(const QString &fullDescription);
};

#endif // ITEMPARSER_H
