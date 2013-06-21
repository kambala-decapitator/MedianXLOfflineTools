#ifndef STRUCTS_H
#define STRUCTS_H

#include "enums.h"
#include "reversebitwriter.h"


// internal

struct ItemProperty
{
    int value, param; // param == 0 if it doesn't exist
    QString displayString; // not empty only for 'custom' descriptions

    ItemProperty() : value(0), param(0) {}
    ItemProperty(int v, int p = 0) : value(v), param(p) {}
    ItemProperty(const QString &text) : value(0), param(0), displayString(text) {}
};

typedef QMultiMap<int, ItemProperty *> PropertiesMultiMap;
typedef QMap<int, ItemProperty *> PropertiesMap;


class ItemInfo;
typedef QList<ItemInfo *> ItemsList;

class ItemInfo
{
public:
    bool isQuest, isIdentified, isSocketed, isEar, isStarter, isExtended, isEthereal, isPersonalized, isRW;
    int location, whereEquipped, row, column, storage;
    QByteArray itemType; // key to get ItemBase
    // fields below exist if isExtended == true
    quint32 guid;
    quint8 socketablesNumber, ilvl, quality, variableGraphicIndex;
    int nonMagicType;                     // quality == 1 || quality == 3 (low quality or superior)
    int setOrUniqueId;                    // key to get SetItemInfo or UniqueItemInfo
    QByteArray inscribedName;             // isPersonalized == true
    int defense;                          // itemBase.genericType == Enums::ItemType::Armor
    int currentDurability, maxDurability; // itemBase.genericType != Enums::ItemTypeGeneric::Misc
    int quantity;                         // itemBase.isStackable == true
    qint8 socketsNumber;                  // isSocketed == true
    PropertiesMultiMap props, rwProps;
    ItemsList socketablesInfo;            // 0 <= size <= 6
    QString rwName;                       // isRW == true

    quint32 plugyPage;
    bool hasChanged;
    QString bitString;

    enum ParsingStatus
    {
        Failed,
        Corrupted,
        Ok
    } status;

    bool shouldDeleteEverything; // is set to false in DupeCheckDialog to prevent crash

    ItemInfo() { init(); }
    ItemInfo(const QString &bits) : bitString(bits) { init(); }
    ~ItemInfo() { if (shouldDeleteEverything) { qDeleteAll(props); qDeleteAll(rwProps); qDeleteAll(socketablesInfo); } }

    void move(int newRow, int newCol)
    {
        row = newRow;
        column = newCol;

        ReverseBitWriter::updateItemRow(this);
        ReverseBitWriter::updateItemColumn(this);
    }

private:
    void init() { plugyPage = 0; hasChanged = false; ilvl = 1; variableGraphicIndex = 0; location = row = column = storage = -1; whereEquipped = 0; shouldDeleteEverything = true; }
};


struct PlugyStashInfo
{
    QString path;
    bool exists;
    QByteArray header;
    quint8 version;
    bool hasGold;
    quint32 gold, lastPage;
};


struct ItemPropertyDisplay
{
    QString displayString;
    int priority, propertyId;

    ItemPropertyDisplay() : priority(0), propertyId(0) {}
    ItemPropertyDisplay(const QString &displayString_, int priority_, int propertyId_) : displayString(displayString_), priority(priority_), propertyId(propertyId_) {}
};


// character

struct BaseStats
{
    struct StatsAtStart
    {
        qint32 strength, dexterity, vitality, energy;
        qint32 stamina;
        // life == vitality + 50
        // mana == energy

        StatsAtStart() {}
        StatsAtStart(qint32 str, qint32 d, qint32 v, qint32 e, qint32 sta) : strength(str), dexterity(d), vitality(v), energy(e), stamina(sta) {}

        qint32 statFromCode(Enums::CharacterStats::StatisticEnum statCode)
        {
            switch (statCode)
            {
            case Enums::CharacterStats::Strength:
                return strength;
            case Enums::CharacterStats::Dexterity:
                return dexterity;
            case Enums::CharacterStats::Vitality:
                return vitality;
            case Enums::CharacterStats::Energy:
                return energy;
            default:
                return 0;
            }
        }
    } statsAtStart;

    typedef struct StatsStep
    {
        qint32 life, stamina, mana; // divide by 4 and floor

        StatsStep() {}
        StatsStep(qint32 l, qint32 s, qint32 m) : life(l), stamina(s), mana(m) {}
    } StatsPerLevel, StatsPerPoint;

    StatsPerLevel statsPerLevel;
    StatsPerPoint statsPerPoint;

    BaseStats() {}
    BaseStats(StatsAtStart s, StatsStep l, StatsStep p) : statsAtStart(s), statsPerLevel(l), statsPerPoint(p) {}
};


// txt

struct ItemBase
{
    QString name, spelldesc;
    quint8 width, height;
    Enums::ItemTypeGeneric::ItemTypeGenericEnum genericType;
    bool isStackable;
    quint16 rlvl;
    QByteArray imageName;
    QList<QByteArray> types;
    qint8 socketableType, classCode;
};

struct ItemPropertyTxt
{
    quint16 add;
    quint8 bits, saveBits; // saveBits != 0 only for properties from Enums::CharacterStats::StatisticEnum
    QList<quint16> groupIDs;
    QString descGroupNegative, descGroupPositive, descGroupStringAdd;
    QString descNegative, descPositive, descStringAdd;
    quint8 descFunc, descPriority, descVal;
    quint8 descGroupFunc, descGroupPriority, descGroupVal;
    quint8 saveParamBits;
    // 'stat' column isn't needed
};

struct SetItemInfo
{
    QString itemName, setName;
};

struct SkillInfo
{
    QString name;
    qint8 classCode, tab, row, col;
};

typedef QList<quint8> SkillList;

struct UniqueItemInfo
{
    QString name;
    quint16 rlvl;
    QByteArray imageName;
};

struct MysticOrb
{
    QByteArray itemCode;
    QList<quint16> statIds;
    quint16 value;
};

struct RunewordInfo
{
    QList<QByteArray> allowedItemTypes;
    QString name;
};
typedef QPair<QByteArray, QByteArray> RunewordKeyPair;
typedef QMultiHash<RunewordKeyPair, RunewordInfo *> RunewordHash;

struct SocketableItemInfo
{
    struct Properties
    {
        int code, param, value;

        Properties() : code(-1), param(0), value(0) {}
        Properties(int c, int p, int v) : code(c), param(p), value(v) {}
    };

    enum PropertyType
    {
        Armor = 0,
        Shield,
        Weapon
    };

    QString name, letter;
    QHash<PropertyType, QList<Properties> > properties;
};


// helper

struct StashSortOptions
{
    bool isQualityOrderAscending;
    bool separateSacredItems, separateEtherealItems, separateCotw, placeSimilarItemsOnOnePage;
    bool newRowTier, newRowCotw, newRowGemQuality, newRowVisuallyDifferent;

    int firstPage, lastPage;
    int diffQualitiesBlankPages, diffTypesBlankPages;
};

#endif // STRUCTS_H
