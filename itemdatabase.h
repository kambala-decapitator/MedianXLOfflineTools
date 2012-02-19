#ifndef ITEMDATABASE_H
#define ITEMDATABASE_H

#include "structs.h"
#include "languagemanager.hpp"


class QFile;

class ItemDataBase
{
    Q_DECLARE_TR_FUNCTIONS(ItemDataBase)

public:
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

    static bool isClassCharm(ItemInfo *item);
    static bool isUberCharm(ItemInfo *item);
    static bool isGenericSocketable(ItemInfo *item); // checks if an item is a gem or a rune

    static ItemsList *currentCharacterItems;

    // bad design, but I'm lazy to create a new class
    static quint8 *clvl;
    static Enums::ClassName::ClassNameEnum *charClass;
    static QList<quint8> *charSkills;

private:
    static QList<QByteArray> stringArrayOfCurrentLineInFile(QFile &f);
    static void expandMultilineString(QString *stringToExpand);

    static QMultiHash<QString, QString> _sets;
};

#endif // ITEMDATABASE_H
