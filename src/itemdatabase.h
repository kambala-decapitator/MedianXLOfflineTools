#ifndef ITEMDATABASE_H
#define ITEMDATABASE_H

#include "structs.h"
#include "languagemanager.hpp"

#include <QSet>


struct FullSetInfo
{
    QList<QString> itemNames;
    QList<SetFixedProperty> partialSetProperties;
    QList<SetFixedProperty> fullSetProperties;
};

class QFile;

class ItemDataBase
{
    Q_DECLARE_TR_FUNCTIONS(ItemDataBase)

public:
    static const double EtherealMultiplier;
    static const char *const kJewelType;
    static QByteArray decompressedFileData(const QString &compressedFilePath, const QString &errorMessage);

    static QHash<QByteArray, ItemBase *> *Items();
    static QHash<QByteArray, ItemType> *ItemTypes();
    static QHash<uint, ItemPropertyTxt *> *Properties();
    static QHash<uint, SetItemInfo *> *Sets();
    static QList<SkillInfo *> *Skills();
    static QHash<uint, UniqueItemInfo *> *Uniques();
    static QHash<uint, MysticOrb *> *MysticOrbs();
    static QHash<uint, QString> *Monsters();
    static RunewordHash *RW();
    static QHash<QByteArray, SocketableItemInfo *> *Socketables();
    static QStringList *NonMagicItemQualities();

    static QHash<QString, quint32> tblIndexLookup;
    static QHash<quint32, QString> *StringTable();
    static QString stringFromTblKey(const QString &key);

    static FullSetInfo fullSetInfoForKey(const QByteArray &setKey) { return _sets.value(setKey); }
    static QString completeItemName(ItemInfo *item, bool shouldUseColor, bool showQualityText = true);
    static QHash<int, ColorsManager::ColorIndex> *itemQualityColorsHash();
    static ColorsManager::ColorIndex colorOfItem(ItemInfo *item);
    static QString &removeColorCodesFromString(QString &s);

    static ItemInfo *loadItemFromFile(const QString &fileName);
    static ItemsList extractItemsFromPage(const ItemsList &items, quint32 page) { return extractItemsFromPageRange(items, page, page); }
    static ItemsList extractItemsFromPageRange(const ItemsList &items, quint32 firstPage, quint32 lastPage);
    static ItemsList itemsStoredIn(int storage, int location = Enums::ItemLocation::Stored, quint32 *pPlugyPage = 0, ItemsList *allItems = 0);
    static bool storeItemIn(ItemInfo *item, Enums::ItemStorage::ItemStorageEnum storage, quint8 rowsTotal, quint8 colsTotal, quint32 plugyPage = 0, bool shouldChangeCoordinatesBits = true);
    static bool storeItemIn(ItemInfo *item, Enums::ItemStorage::ItemStorageEnum storage, quint8 rowsTotal, quint8 colsTotal, Enums::ItemLocation::ItemLocationEnum location, ItemsList *pItems = 0, quint32 plugyPage = 0, bool shouldChangeCoordinatesBits = true);
    static bool canStoreItemAt(quint8 row, quint8 col, const QByteArray &storeItemType, const ItemsList &items, int rowsTotal, int colsTotal);

    static bool isClassCharm(const QByteArray &itemType);
    static bool isClassCharm(ItemInfo *item);
    static bool isUberCharm(const QList<QByteArray> &itemBaseTypes);
    static bool isUberCharm(const QByteArray &itemBaseType);
    static bool isUberCharm(ItemInfo *item);
    static bool isGenericSocketable(ItemInfo *item); // checks if an item is a gem or a rune
    static bool isCube(ItemInfo *item);
    static bool isTomeWithScrolls(ItemInfo *item);

    static bool doesItemGrantBonus(ItemInfo *item);
    static bool isCtcProperty(int propId) { return propId != Enums::ItemProperties::CooldownReduction && Properties()->value(propId)->descPositive.startsWith('%'); }

    static bool canDisenchantIntoArcaneShards(ItemInfo *item);
    static bool canDisenchantIntoSignetOfLearning(ItemInfo *item);

private:
    static QHash<QByteArray, FullSetInfo> _sets;

    static QList<QByteArray> stringArrayOfCurrentLineInFile(QIODevice &d);
    static void expandMultilineString(QString *stringToExpand);

    static bool canDisenchant(ItemInfo *item);
};
#endif // ITEMDATABASE_H
