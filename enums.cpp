#include "enums.h"
#include "itemdatabase.h"
#include "characterinfo.hpp"


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


int ItemOffsets::offsetLength(int offset)
{
    switch (offset)
    {
    case Ethereal: case IsPersonalized:
        return 1;
    case Location: case Storage:
        return 3;
    case EquipIndex: case Column: case Row:
        return 4;
    case Type: case (Type + 8): case (Type + 16): // type consists of 3 chars (excluding space)
        return 8;
    default:
        qWarning("offset length not set for offset %d!", offset);
        return 0;
    }
}


bool compareSkillIndexes(int i, int j)
{
    SkillInfo *iSkill = ItemDataBase::Skills()->value(i), *jSkill = ItemDataBase::Skills()->value(j);
    if (iSkill->tab == jSkill->tab)
    {
        if (iSkill->col == jSkill->col)
            return iSkill->row < jSkill->row;
        else
            return iSkill->col < jSkill->col;
    }
    else
        return iSkill->tab < jSkill->tab;
}

const QHash<ClassName::ClassNameEnum, Skills::SkillsOrderPair> &Skills::characterSkillsIndexes()
{
    static QHash<ClassName::ClassNameEnum, Skills::SkillsOrderPair> hash;
    if (hash.isEmpty())
    {
        QList<SkillInfo *> *skills = ItemDataBase::Skills();
        int n = skills->size();
        for (int classCode = ClassName::Amazon; classCode <= ClassName::Assassin; ++classCode)
        {
            QList<int> skillsIndexes;
            for (int i = 0; i < n; ++i)
                if (skills->at(i)->classCode == classCode)
                    skillsIndexes += i;

            SkillsOrderPair pair;
            pair.first = skillsIndexes;
            qSort(skillsIndexes.begin(), skillsIndexes.end(), compareSkillIndexes);
            pair.second = skillsIndexes;
            hash[static_cast<ClassName::ClassNameEnum>(classCode)] = pair;
        }
    }
    return hash;
}

Skills::SkillsOrderPair Skills::currentCharacterSkillsIndexes() { return characterSkillsIndexes().value(CharacterInfo::instance().basicInfo.classCode); }

}