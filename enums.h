#ifndef ENUMS_H
#define ENUMS_H

#include "helpers.h"

//#include <QMetaEnum>
#include <QStringList>
#include <QCoreApplication>


namespace Enums
{
class Offsets
{
public:
    enum
    {
        FileSize = 0x8,
        Checksum = 0xC,
        Name = 0x14,
        Status = 0x24,
        Progression = 0x25,
        Class = 0x28,
        Level = 0x2B,
        CurrentLocation = 0xA8,
        Mercenary = 0xB3,
        QuestsHeader = 0x14F,
        QuestsData = 0x159,
        WaypointsHeader = 0x279,
        WaypointsData = 0x281,
        NPCHeader = 0x2CA,
        StatsHeader = 0x2FD,
        StatsData = 0x2FF
    };
};


class StatusBits
{
public:
    enum
    {
        IsHardcore = 0x4,
        HadDied = 0x8,
        IsExpansion = 0x20
    };
};


class ClassName
{
    Q_DECLARE_TR_FUNCTIONS(ClassName)

public:
    enum ClassNameEnum
    {
        Amazon = 0,
        Sorceress,
        Necromancer,
        Paladin,
        Barbarian,
        Druid,
        Assassin
    };

    static const QStringList &classes();
};


class Progression
{
    Q_DECLARE_TR_FUNCTIONS(Progression)

public:
    enum
    {
        Hatred = 0x4,
        Nightmare = 0x9,
        Destruction = 0xE,
        Completed = 0x10
    };

    static QPair<QString, quint8> titleNameAndMaxDifficultyFromValue(quint8 value, bool isMale, bool isHC);
};

class DifficultyBits
{
public:
    enum
    {
        IsActive = 0x80,
        CurrentAct = 0x7
    };
};


class Mercenary
{
    Q_DECLARE_TR_FUNCTIONS(Mercenary)

public:
    enum MercenaryEnum
    {
        // A1
        Ranger = 0,
        Priestess,
        // A2
        Exemplar,
        Shapeshifter,
        FighterMage,
        // A3
        Necrolyte,
        Bloodmage,
        Abjurer,
        // A5
        MercBarbarian
    };

    enum
    {
        MaxCode = 29
    };

    static const QStringList &types();

    static MercenaryEnum mercCodeFromValue(quint16 value);
    static int mercBaseValueFromCode(MercenaryEnum code);
    static QPair<int, int> allowedTypesForMercCode(MercenaryEnum code, int *curIndex);
    static int mercNamesIndexFromCode(MercenaryEnum code);
};


class Quests
{
public:
    enum
    {
        DenOfEvil = 0x2,
        Radament = 0x12,
        LamEsensTome = 0x22,
        Izual = 0x32,
        Act5Enabled = 0x38
    };

    enum
    {
        IsCompleted = 0x1,
        IsTaskDone = 0x2,
        Size = 0x60
    };
};


class CharacterStats : public QObject
{
    Q_OBJECT
    Q_ENUMS(StatisticEnum)

public:
    enum StatisticEnum
    {
        Strength = 0,
        Energy,
        Dexterity,
        Vitality,
        FreeStatPoints,
        FreeSkillPoints,
        Life,
        BaseLife,
        Mana,
        BaseMana,
        Stamina,
        BaseStamina,
        Level,
        Experience,
        InventoryGold,
        StashGold,
        SignetsOfLearningEaten = 185,
        SignetsOfSkillEaten,
        End = 511
    };

    enum
    {
        StatCodeLength = 9,
        MaxLevel = 126,
        InventoryGoldFactor = 10000,
        StashGoldMax = 2500000,
        SignetsOfLearningMax = 500,
        SignetsOfSkillMax = 3
    };

    static int statLengthFromValue(quint8 value);
    static QMetaEnum statisticMetaEnum() { return metaEnumFromName<CharacterStats>("StatisticEnum"); }
    static const char *statisticNameFromValue(quint8 value) { return statisticMetaEnum().valueToKey(value); }
};


// item related

class ItemType
{
public:
    enum ItemTypeEnum
    {
        Armor = 0,
        Weapon,
        Misc
    };
};


class ItemProperties
{
public:
    enum
    {
        EnhancedDefence = 16,
        Defence = 31,
        Durability = 72,
        DurabilityMax = 73,
        RequiredLevel = 92,
        DefenceBoCL = 214,
        End = 511
    };
};

class ItemQuality
{
public:
    enum
    {
        LowQuality = 1,
        Normal,
        HighQuality,
        Magic,
        Set,
        Rare,
        Unique,
        Crafted,
        Honorific
    };
};

class ItemStorage : public QObject
{
    Q_OBJECT
    Q_ENUMS(ItemStorageEnum)

public:
    enum ItemStorageEnum
    {
        NotInStorage = 0,
        Inventory,
        Cube = 4,
        Stash,
        PersonalStash,
        SharedStash,
        HCStash
    };

    static QMetaEnum metaEnum() { return metaEnumFromName<Enums::ItemStorage>("ItemStorageEnum"); }
};

class ItemLocation
{
public:
    enum ItemLocationEnum
    {
        Stored = 0,
        Equipped
    };
};

class ItemOffsets
{
public:
	enum
	{
		Location = 0x3A,
		EquipIndex = 0x3D,
		Columns = 0x41,
		Rows = 0x45,
		Storage = 0x49
	};
};
}

#endif // ENUMS_H
