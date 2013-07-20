#ifndef ITEMDATABASE_H
#define ITEMDATABASE_H

#include "structs.h"
#include "languagemanager.hpp"


class QFile;

class ItemDataBase
{
    Q_DECLARE_TR_FUNCTIONS(ItemDataBase)

public:
    static const char *const kJewelType;
    static bool createUncompressedTempFile(const QString &compressedFilePath, const QString &errorMessage, QFile *uncompressedFile);

    static QHash<QByteArray, ItemBase *> *Items();
    static QHash<QByteArray, QList<QByteArray> > *ItemTypes();
    static QHash<uint, ItemPropertyTxt *> *Properties();
    static QHash<uint, SetItemInfo *> *Sets();
    static QList<SkillInfo *> *Skills();
    static QHash<uint, UniqueItemInfo *> *Uniques();
    static QHash<uint, MysticOrb *> *MysticOrbs();
    static QHash<uint, QString> *Monsters();
    static RunewordHash *RW();
    static QHash<QByteArray, SocketableItemInfo *> *Socketables();
    static QStringList *NonMagicItemQualities();

    static QStringList completeSetForName(const QString &setName) { return _sets.values(setName); }
    static QString completeItemName(ItemInfo *item, bool shouldUseColor, bool showQualityText = true);
    static QHash<int, ColorsManager::ColorIndex> *itemQualityColorsHash();
    static ColorsManager::ColorIndex colorOfItem(ItemInfo *item);
    static QString &removeColorCodesFromString(QString &s);

    static ItemInfo *loadItemFromFile(const QString &fileName);
    static ItemsList extractItemsFromPage(const ItemsList &items, quint32 page) { return extractItemsFromPageRange(items, page, page); }
    static ItemsList extractItemsFromPageRange(const ItemsList &items, quint32 firstPage, quint32 lastPage);
    static ItemsList itemsStoredIn(int storage, int location = Enums::ItemLocation::Stored, quint32 *pPlugyPage = 0, ItemsList *allItems = 0);
    static bool storeItemIn(ItemInfo *item, Enums::ItemStorage::ItemStorageEnum storage, quint8 rowsTotal, quint32 plugyPage = 0, quint8 colsTotal = 10);
    static bool canStoreItemAt(quint8 row, quint8 col, const QByteArray &storeItemType, const ItemsList &items, int rowsTotal, int colsTotal = 10);

    static bool isClassCharm(const QByteArray &itemType);
    static bool isClassCharm(ItemInfo *item);
    static bool isUberCharm(const QList<QByteArray> &itemBaseTypes);
    static bool isUberCharm(const QByteArray &itemBaseType);
    static bool isUberCharm(ItemInfo *item);
    static bool isGenericSocketable(ItemInfo *item); // checks if an item is a gem or a rune
    static bool isCube(ItemInfo *item);

    static bool doesItemGrantBonus(ItemInfo *item);

    static bool canDisenchantIntoArcaneShards(ItemInfo *item);
    static bool canDisenchantIntoSignetOfLearning(ItemInfo *item);

private:
    static QMultiHash<QString, QString> _sets;

    static QList<QByteArray> stringArrayOfCurrentLineInFile(QFile &f);
    static void expandMultilineString(QString *stringToExpand);

    static bool canDisenchant(ItemInfo *item);
};
#endif // ITEMDATABASE_H
