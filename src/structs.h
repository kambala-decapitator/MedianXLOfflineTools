#ifndef STRUCTS_H
#define STRUCTS_H

#include "enums.h"
#include "reversebitwriter.h"


// internal

struct ItemProperty
{
    int bitStringOffset, value;
    quint32 param; // param == 0 if it doesn't exist
    QString displayString; // not empty only for 'custom' descriptions

    ItemProperty() : bitStringOffset(0), value(0), param(0) {}
    ItemProperty(int v, quint32 p = 0) : bitStringOffset(0), value(v), param(p) {}
    ItemProperty(const QString &text) : bitStringOffset(0), value(0), param(0), displayString(text) {}
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
    struct
    {
        quint8 classCode, level;
        QByteArray name;
    } earInfo; // isEar == true
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
    PropertiesMultiMap props, rwProps;//, setProps;
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
    quint16 inscribedNameOffset;

    ItemInfo() { init(); }
    ItemInfo(const QString &bits) : bitString(bits) { init(); }
    ~ItemInfo() { if (shouldDeleteEverything) { qDeleteAll(props); qDeleteAll(rwProps); qDeleteAll(socketablesInfo); } }

    void move(int newRow, int newCol, quint32 newPage, bool shouldChangeBits = true)
    {
        row = newRow;
        column = newCol;
        plugyPage = newPage;

        if (shouldChangeBits)
        {
            hasChanged = true;
            ReverseBitWriter::updateItemRow(this);
            ReverseBitWriter::updateItemColumn(this);
        }
    }

private:
    void init() { plugyPage = 0; hasChanged = false; ilvl = 1; variableGraphicIndex = 0; location = row = column = storage = -1; whereEquipped = 0; shouldDeleteEverything = true; }
};


// no longer PlugY format, Sigma v2 instead
struct PlugyStashInfo
{ //-V802
    QString path;
    bool exists;
    quint32 version;
    quint32 activePage;
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
{ //-V802
    QString name, spelldesc;
    quint8 width, height;
    Enums::ItemTypeGeneric::ItemTypeGenericEnum genericType;
    bool isStackable;
    quint16 rlvl, rstr, rdex;
    bool is1h2h, is2h;
    quint16 min1hDmg, max1hDmg;
    quint16 min2hDmg, max2hDmg;
    quint16 minThrowDmg, maxThrowDmg;
    QByteArray imageName;
    quint8 questId; // non-zero value means that it's a quest item
    quint8 strBonus, dexBonus;
    QList<QByteArray> types;
    qint8 socketableType, classCode;
};

struct ItemType
{
    QList<QByteArray> baseItemTypes, variableImageNames;
};

struct ItemPropertyTxt
{
    quint16 add;
    quint8 bits, paramBits;
    quint8 bitsSave, paramBitsSave; // saveBits != 0 only for properties from Enums::CharacterStats::StatisticEnum
    QList<quint16> groupIDs;
    QString descGroupNegative, descGroupPositive, descGroupStringAdd;
    QString descNegative, descPositive, descStringAdd;
    quint8 descFunc, descPriority, descVal;
    quint8 descGroupFunc, descGroupPriority, descGroupVal;
    QByteArray stat;
};

struct SetOrUniqueItemInfo
{
    quint16 rlvl, ilvl;
    QByteArray imageName;
};

struct SetFixedProperty
{
    QList<quint16> ids;
    int param, minValue, maxValue;

    SetFixedProperty() : param(0), minValue(0), maxValue(0) {}
};

struct SetItemInfo : public SetOrUniqueItemInfo
{
    QString itemName, setName;
    QByteArray key;
    QList<SetFixedProperty> fixedProperties;
};

struct UniqueItemInfo : public SetOrUniqueItemInfo
{
    QString name;
};

struct SkillInfo
{
    QString name;
    qint8 classCode, tab, row, col;
    quint16 imageId;
};

typedef QList<quint8> SkillList;

struct MysticOrb
{
    QByteArray itemCode;
    QList<quint16> statIds;
    qint16 value;
    quint32 param;
    quint8 rlvl;

    MysticOrb() : param(0) {}
};

struct RunewordInfo
{
    QList<QByteArray> allowedItemTypes;
    QString name;
};
typedef QMultiHash<QByteArray, RunewordInfo *> RunewordHash;

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
    bool isNewRowTier, isNewRowCotw, isNewRowVisuallyDifferentMisc;
    bool isEachTypeFromNewPage, shouldPlaceSimilarMiscItemsOnOnePage, shouldSeparateSacred, shouldSeparateEth;

    quint32 firstPage, lastPage;
    int diffQualitiesBlankPages, diffTypesBlankPages;
};

#endif // STRUCTS_H
