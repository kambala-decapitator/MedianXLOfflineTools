#ifndef ENUMS_H
#define ENUMS_H

#include "helpers.h"

#include <QCoreApplication>
#include <QHash>

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
        SkillKeys = 0x38,
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
        IsExpansion = 0x20,
        IsLadder = 0x40
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
        Normal = 0x4,
        Nightmare = 0x9,
        Hell = 0xE,
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
        GoldenBird = 0x28,
        LamEsensTome = 0x22,
        Izual = 0x32,
        Act5Enabled = 0x38,
        Anya = 0x4A,
        Nihlathak = 0x4C
    };

    enum
    {
        IsCompleted = 1 << 0,
        IsTaskDone = 1 << 1,
        Size = 0x60
    };
};


class CharacterStats
{
    Q_GADGET
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
        Achievements = 88,
        SignetsOfLearningEaten = 185,
        SignetsOfSkillEaten,
        End = 511
    };

    enum
    {
        StatCodeLength = 9,
        MaxLevel = 150,
        InventoryGoldFactor = 10000,
        StashGoldMax = 2500000,
        SignetsOfLearningMax = 400,
        SignetsOfSkillMax = 2,
        AchievementsCount = 10
    };

    static QMetaEnum statisticMetaEnum() { return metaEnumFromName<CharacterStats>("StatisticEnum"); }
    static const char *statisticNameFromValue(quint8 value) { return statisticMetaEnum().valueToKey(value); }
};


// item related

class ItemTypeGeneric
{
public:
    enum ItemTypeGenericEnum
    {
        Unknown = -1,
        Armor,
        Weapon,
        Misc
    };
};


class ItemProperties
{
public:
    enum
    {
        Strength  = CharacterStats::Strength,
        Energy    = CharacterStats::Energy,
        Dexterity = CharacterStats::Dexterity,
        Vitality  = CharacterStats::Vitality,
        Life      = CharacterStats::BaseLife,
        Mana      = CharacterStats::BaseMana,
        Stamina   = CharacterStats::BaseStamina,

        EnhancedDefence = 16,
        EnhancedDamage,
        MinimumDamage = 21,
        MaximumDamage,
        MinimumDamageSecondary, // these 2 properties are the
        MaximumDamageSecondary, // combined 'adds x-y damage'

        Defence = 31,

        MinimumDamageFire = 48,
        MaximumDamageFire,
        MinimumDamageLightning,
        MaximumDamageLightning,
        MinimumDamageMagic,
        MaximumDamageMagic,
        MinimumDamageCold,
        MaximumDamageCold,
        DurationCold,
        MinimumDamagePoison,
        MaximumDamagePoison,
        DurationPoison,

        Durability = 72,
        DurabilityMax,
        ReplenishLife = 74,
        LifeBonus = 76,
        ManaBonus,
        ClassSkills = 83,
        Requirements = 91,
        RequiredLevel = 92,
        Oskill = 97,
        ClassOnlySkill = 107,
        DamageToUndead = 122,
        AllSkills = 127,
        Indestructible = 152,
        ChargedSkill = 204,
        DefenceBasedOnClvl = 214,
        EnhancedDefenceBasedOnClvl,
        MaximumDamageBasedOnClvl = 218,
        ShrineBless = 219,
        EdyremUpgrade = 270,
        CantDisenchant = 276,
        MysticOrbsEffectQuadrupled = 288,
        MysticOrbsEffectDoubled = 296,
        CooldownReduction = 309,
        Avoid1 = 338,
        Avoid2,
        Avoid3,
        StrengthBonus = 359,
        DexterityBonus,
        EnergyBonus,
        VitalityBonus,
        Trophy = 450,
        CubeUpgrade1 = 452,
        CubeUpgrade2,
        CubeUpgrade3,
        CubeUpgrade4,
        ClassRestriction = 463,
        MaxSlvlIncrease = 479,
        ShrineVesselCounter = 501,
        StrengthBasedOnBlessedLifeSlvl = 503,
        DexterityBasedOnBlessedLifeSlvl,
        ReplenishLifeBasedOnClvl,
        End = 511
    };
};

class ClassCharmChallenges
{
public:
    enum
    {
        WindowsInHell = 313,
        MirrorMirror = 314,
        Countess = 400,
        LC2 = 401,
        Crowned = 403,
        Bremmtown = 419,
        Ennead = 473,
        BlackRoad = 474
    };
};

class ItemQuality
{
    Q_GADGET
    Q_ENUMS(ItemQualityEnum)

public:
    enum ItemQualityEnum
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

class ItemStorage
{
    Q_GADGET
    Q_ENUMS(ItemStorageEnum)

public:
    enum ItemStorageEnum
    {
        NotInStorage = 0,
        Inventory,
        Cube = 4,
        Stash,
        PersonalStash,
        SigmaSharedStash,
        SigmaHCStash,
        SharedStash,
        HCStash
    };

    static QMetaEnum metaEnum() { return metaEnumFromName<ItemStorage>("ItemStorageEnum"); }
};

class ItemLocation
{
    Q_GADGET
    Q_ENUMS(ItemLocationEnum)

public:
    enum ItemLocationEnum
    {
        Stored = 0,
        Equipped,
        Belt,
        Merc,
        Corpse,
        IronGolem
    };
};

class ItemOffsets
{
public:
    enum ItemOffsetsEnum
    {
        Ethereal = 0x26,
        IsPersonalized = 0x28,
        Location = 0x3A,
        EquipIndex = 0x3D,
        Column = 0x41,
        Row = 0x45,
        Storage = 0x49,
        Type = 0x4C
    };

    static int offsetLength(int offset); // modify whenever enum is modified
};

class Skills
{
public:
    enum
    {
        BlessedLife = 649
    };

    typedef QPair<QList<int>, QList<int> > SkillsOrderPair; // first is save file order, second - visual and planner order
    static const QHash<ClassName::ClassNameEnum, SkillsOrderPair> &characterSkillsIndexes();
    static SkillsOrderPair currentCharacterSkillsIndexes();
};

} // namespace Enums

#endif // ENUMS_H
