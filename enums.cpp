#include "enums.h"

#include <QHash>


namespace Enums
{
    const QStringList &ClassName::classes()
    {
        static const QStringList classes = QStringList() << tr("Amazon") << tr("Sorceress") << tr("Necromancer") << tr("Paladin") << tr("Barbarian") << tr("Druid") << tr("Assassin");
        return classes;
    }


    QPair<QString, quint8> Progression::titleNameAndMaxDifficultyFromValue(quint8 value, bool isMale, bool isHC)
    {
        if (value < Hatred)
            return QPair<QString, quint8>(tr("Nobody"), 0);
        else if (value < Nightmare)
            return QPair<QString, quint8>(isHC ? tr("Destroyer") : tr("Slayer"), 1);
        else if (value < Destruction)
            return QPair<QString, quint8>(isHC ? tr("Conqueror") : tr("Champion"), 2);
        else if (value < Completed)
            return QPair<QString, quint8>(isHC ? tr("Guardian") : (isMale ? tr("Patriarch") : tr("Matriarch")), 2);
        else
            return QPair<QString, quint8>();
    }


    const QStringList &Mercenary::types()
    {
        static const QStringList types = QStringList() << tr("Ranger") << tr("Priestess") << tr("Exemplar") << tr("Shapeshifter") << tr("Fighter Mage")
                                                       << tr("Necrolyte") << tr("Bloodmage") << tr("Abjurer") << tr("Barbarian");
        return types;
    }

    Mercenary::MercenaryEnum Mercenary::mercCodeFromValue(quint16 value)
    {
        switch(value)
        {
        case 0: case 2: case 4:
            return Ranger;
        case 1: case 3: case 5:
            return Priestess;
        case 6: case 9: case 12:
            return Exemplar;
        case 7: case 10: case 13:
            return Shapeshifter;
        case 8: case 11: case 14:
            return FighterMage;
        case 15: case 18: case 21:
            return Necrolyte;
        case 16: case 19: case 22:
            return Bloodmage;
        case 17: case 20: case 23:
            return Abjurer;
        default:
            return MercBarbarian;
        }
    }

    int Mercenary::mercBaseValueFromCode(MercenaryEnum code)
    {
        switch (code)
        {
        case Ranger: case Priestess:
            return 0;
        case Exemplar: case Shapeshifter: case FighterMage:
            return 6;
        case Necrolyte: case Bloodmage: case Abjurer:
            return 15;
        default: // Barbarian
            return 24;
        }
    }

    QPair<int, int> Mercenary::allowedTypesForMercCode(MercenaryEnum code, int *curIndex)
    {
        switch(code)
        {
        case Ranger: case Priestess:
            *curIndex = code;
            return QPair<int, int>(0, 2);
        case Exemplar: case Shapeshifter: case FighterMage:
            *curIndex = code - Exemplar;
            return QPair<int, int>(2, 3);
        case Necrolyte: case Bloodmage: case Abjurer:
            *curIndex = code - Necrolyte;
            return QPair<int, int>(5, 3);
        default: // Barbarian
            *curIndex = 0;
            return QPair<int, int>(8, 1);
        }
    }

    int Mercenary::mercNamesIndexFromCode(MercenaryEnum code)
    {
        switch(code)
        {
        case Ranger: case Priestess:
            return 0;
        case Exemplar: case Shapeshifter: case FighterMage:
            return 1;
        case Necrolyte: case Bloodmage: case Abjurer:
            return 2;
        default: // Barbarian
            return 3;
        }
    }


    int CharacterStats::statLengthFromValue(quint8 value)
    {
        switch(value)
        {
        case Strength: case Energy: case Dexterity: case Vitality: case SignetsOfLearningEaten:
            return 12;
        case FreeStatPoints:
            return 11;
        case FreeSkillPoints: case SignetsOfSkillEaten:
            return 8;
        case Life: case BaseLife: case Mana: case BaseMana: case Stamina: case BaseStamina:
            return 21;
        case Level:
            return 7;
        case Experience:
            return 32;
        case StashGold: case InventoryGold:
            return 25;
        default:
            return 0;
        }
    }


    int ItemOffsets::offsetLength(ItemOffsetsEnum offset)
    {
        static QHash<ItemOffsetsEnum, int> hash;
        if (hash.isEmpty())
        {
            hash[Ethereal] = 1;
            hash[Location] = 3;
            hash[EquipIndex] = 4;
            hash[Column] = 4;
            hash[Row] = 4;
            hash[Storage] = 3;
        }
        return hash.value(offset);
    }
}
