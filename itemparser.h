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
	static ItemInfo *parseItem(QDataStream &inputDataStream, const QByteArray &bytes);
	static QMultiMap<int, ItemProperty> parseItemProperties(ReverseBitReader &bitReader, bool *ok);
	static void writeItems(const ItemsList &items, QDataStream &ds);

	static ItemInfo *loadItemFromFile(const QString &filePath);
	static ItemsList itemsLocatedAt(int storage, bool location = Enums::ItemLocation::Stored);
	static bool storeItemIn(Enums::ItemStorage::ItemStorageEnum storage, quint8 rows, quint8 cols, ItemInfo *item);
	static bool canStoreItemAt(quint8 row, quint8 col, const QByteArray &itemType, const ItemsList &items);

	static const QByteArray itemHeader, plugyPageHeader;
};

#endif // ITEMPARSER_H
