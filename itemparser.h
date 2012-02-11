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
    static const QString enhancedDamageFormat;

    static ItemInfo *parseItem(QDataStream &inputDataStream, const QByteArray &bytes);
    static PropertiesMultiMap parseItemProperties(ReverseBitReader &bitReader, bool *ok);
    static bool itemTypeInheritsFromTypes(const QByteArray &itemType, const QList<QByteArray> &allowedItemTypes);

    static void writeItems(const ItemsList &items, QDataStream &ds);

    static ItemInfo *loadItemFromFile(const QString &filePath);
    static ItemsList itemsLocatedAt(int storage, ItemsList *allItems = 0, int location = Enums::ItemLocation::Stored);
    static bool storeItemIn(ItemInfo *item, Enums::ItemStorage::ItemStorageEnum storage, quint8 rows, quint8 cols, int plugyPage = 0);
    static bool canStoreItemAt(quint8 row, quint8 col, const QByteArray &storeItemType, const ItemsList &items, int rowsTotal, int colsTotal, int plugyPage = 0);

private:
    static bool itemTypesInheritFromTypes(const QList<QByteArray> &itemTypes, const QList<QByteArray> &allowedItemTypes);
};

#endif // ITEMPARSER_H
