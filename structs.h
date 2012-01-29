#ifndef STRUCTS_H
#define STRUCTS_H

#include "enums.h"


// internal

struct ItemProperty
{
	int value, param; // param == 0 if it doesn't exist
    QString displayString; // not empty only for 'custom' descriptions

    ItemProperty() : value(0) {}
	ItemProperty(int v, int p = 0) : value(v), param(p) {}
	ItemProperty(const QString &text) : displayString(text) {}
};
Q_DECLARE_TYPEINFO(ItemProperty, Q_MOVABLE_TYPE);


struct ItemInfo;
typedef QList<ItemInfo *> ItemsList;

struct ItemInfo
{
	bool isQuest, isIdentified, isSocketed, isEar, isStarter, isExtended, isEthereal, isPersonalized, isRW;
    int location, whereEquipped, row, column, storage;
	QByteArray itemType; // key to get ItemBase
	// fields below exist if isExtended == true
	quint32 guid;
    quint8 socketablesNumber, ilvl, quality, variableGraphicIndex;
	int nonMagicType; // quality == 1 || quality == 3 (low quality or superior)
	int setOrUniqueId; // key to get SetItemInfo or UniqueItemInfo
	QString inscribedName; // isPersonalized == true
	int defense; // itemBase.type == Enums::ItemType::Armor
	int currentDurability, maxDurability; // itemBase.type != Enums::ItemType::Misc
	int quantity; // itemBase.isStackable == true
	qint8 socketsNumber; // isSocketed == true
    QMultiMap<int, ItemProperty> props, rwProps;
    ItemsList socketablesInfo; // 0 ≤ size ≤ 6
	QString rwName; // isRW == true

    quint32 plugyPage;
    bool hasChanged;
    QString bitString;

    ItemInfo() { init(); }
    ItemInfo(const QString &bits) : bitString(bits) { init(); }

private:
	void init() { plugyPage = 0; hasChanged = false; ilvl = 1; variableGraphicIndex = 0; }
};
Q_DECLARE_TYPEINFO(ItemInfo, Q_MOVABLE_TYPE);


struct PlugyStashInfo
{
    QString path;
	bool exists;
    QByteArray header;
    quint8 version;
    bool hasGold;
    quint32 gold, lastPage;
};


// character

struct CharacterInfo
{
	struct
	{
		QString originalName, newName;
		Enums::ClassName::ClassNameEnum classCode; // immutable
		quint8 titleCode; // immutable
		quint8 level;
		//quint8 currentDifficulty, currentAct; // not used atm
		bool isHardcore;
		bool hadDied;
		quint16 totalSkillPoints, totalStatPoints;
	} basicInfo;

	struct
	{
		quint8 denOfEvilQuestsCompleted, radamentQuestsCompleted, lamEsensTomeQuestsCompleted, izualQuestsCompleted; // 0-3
	} questsInfo;

	struct
	{
		bool exists; // immutable
		quint16 nameIndex;
		Enums::Mercenary::MercenaryEnum code;
		quint32 experience; // immutable
		quint8 level; // immutable
	} mercenary;

	struct
	{
		quint8 corpses;
        ItemsList character;
	} items;

    // itemsOffset points to the number of character items - just after the very first JM
    quint32 skillsOffset, itemsOffset, itemsEndOffset;
};

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

	struct StatsPerLevel
	{
		qint32 life, stamina, mana; // divide by 4 and floor

		StatsPerLevel() {}
		StatsPerLevel(qint32 l, qint32 s, qint32 m) : life(l), stamina(s), mana(m) {}
	} statsPerLevel;

	struct StatsPerPoint
	{
		qint32 life, stamina, mana; // divide by 4 and floor

		StatsPerPoint() {}
		StatsPerPoint(qint32 l, qint32 s, qint32 m) : life(l), stamina(s), mana(m) {}
	} statsPerPoint;

	BaseStats() {}
	BaseStats(StatsAtStart s, StatsPerLevel l, StatsPerPoint p) : statsAtStart(s), statsPerLevel(l), statsPerPoint(p) {}
};


// txt

struct ItemBase
{
	QString name;
	quint8 width, height;
	Enums::ItemType::ItemTypeEnum type;
	bool isStackable;
	quint16 rlvl;
    QByteArray imageName, typeString;
    qint8 socketableType, classCode;

	ItemBase() : imageName(0), typeString(0) {}
};

struct ItemPropertyTxt
{
	quint16 add;
	quint8 bits;
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
	qint8 classCode;
};
Q_DECLARE_TYPEINFO(SkillInfo, Q_MOVABLE_TYPE);

struct UniqueItemInfo
{
	QString name;
	quint16 rlvl;
};

struct MysticOrb
{
	QByteArray itemCode;
	quint16 statId, value;

	MysticOrb() : itemCode(0) {}
};

struct RunewordInfo
{
	QList<QByteArray> allowedItemTypes;
	QString name;
};
typedef QPair<QByteArray, QByteArray> RunewordKeyPair;

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

#endif // STRUCTS_H
