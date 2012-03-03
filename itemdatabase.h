#ifndef ITEMDATABASE_H
#define ITEMDATABASE_H

#include "structs.h"
#include "languagemanager.hpp"


class QFile;

class ItemDataBase
{
    Q_DECLARE_TR_FUNCTIONS(ItemDataBase)

public:
    static bool createUncompressedTempFile(const QString &compressedFilePath, const QString &errorMessage, QFile *uncompressedFile);

    static QHash<QByteArray, ItemBase> *Items();
    static QHash<QByteArray, QList<QByteArray> > *ItemTypes();
    static QHash<uint, ItemPropertyTxt> *Properties();
    static QHash<uint, SetItemInfo> *Sets();
    static QList<SkillInfo> *Skills();
    static QHash<uint, UniqueItemInfo> *Uniques();
    static QHash<uint, MysticOrb> *MysticOrbs();
    static QHash<uint, QString> *Monsters();
    static QMultiHash<RunewordKeyPair, RunewordInfo> *RW();
    static QHash<QByteArray, SocketableItemInfo> *Socketables();
    static QStringList *NonMagicItemQualities();

    static QStringList completeSetForName(const QString &setName) { return _sets.values(setName); }
    static QString completeItemName(ItemInfo *item, bool shouldUseColor, bool showQualityText = true);
    static QHash<int, ColorIndex> *itemQualityColorsHash();
    static ColorIndex colorOfItem(ItemInfo *item);
    static QString &removeColorCodesFromString(QString &s);

    static ItemInfo *loadItemFromFile(const QString &fileName);
    static ItemsList itemsStoredIn(int storage, int location = Enums::ItemLocation::Stored, quint32 *pPlugyPage = 0, ItemsList *allItems = 0);
    static bool storeItemIn(ItemInfo *item, Enums::ItemStorage::ItemStorageEnum storage, quint8 rowsTotal, quint32 plugyPage = 0, quint8 colsTotal = 10);
    static bool canStoreItemAt(quint8 row, quint8 col, const QByteArray &storeItemType, const ItemsList &items, int rowsTotal, int colsTotal = 10);

    static bool isClassCharm(ItemInfo *item);
    static bool isUberCharm(ItemInfo *item);
    static bool isGenericSocketable(ItemInfo *item); // checks if an item is a gem or a rune
    static bool isCube(ItemInfo *item);
    static bool hasCube();

private:
    static QList<QByteArray> stringArrayOfCurrentLineInFile(QFile &f);
    static void expandMultilineString(QString *stringToExpand);

    static QMultiHash<QString, QString> _sets;
};
#endif // ITEMDATABASE_H
