#ifndef CHARACTERINFO_H
#define CHARACTERINFO_H

#include "structs.h"

#include <QString>

#include <numeric>


class CharacterInfo
{
public:
    static CharacterInfo &instance()
    {
        static CharacterInfo obj;
        return obj;
    }

    quint32 valueOfStatistic(Enums::CharacterStats::StatisticEnum stat) const
    {
        QList<QVariant> list = basicInfo.statsDynamicData.value(stat).toList();
        return list.empty() ? 0 : list.first().toULongLong();
    }
    void setValueForStatisitc(quint32 value, Enums::CharacterStats::StatisticEnum stat) { basicInfo.statsDynamicData.replace(stat, QList<QVariant>() << value); }

    struct CharacterInfoBasic
    {
        QString originalName, newName;
        Enums::ClassName::ClassNameEnum classCode; // immutable
        quint8 titleCode; // immutable
        quint8 level;
        //quint8 currentDifficulty, currentAct; // not used atm
        bool isHardcore;
        bool hadDied;
        bool isLadder;

        QMultiMap<Enums::CharacterStats::StatisticEnum, QVariant> statsDynamicData; // there're multiple Achievements, each contains 2 numbers
        quint16 totalStatPoints;
        
        SkillList skills;
        SkillList skillsReadable; // used in planner and skill tree
        quint16 totalSkillPoints;

        QList<quint32> hotkeyedSkills;
        struct { quint32 lmb, rmb; } mainHandSkills, altHandSkills;
    } basicInfo;

    struct
    {
        QList<bool> denOfEvil, radament, goldenBird, lamEsensTome, izual, rescueAnya; // size == 3

        quint8 denOfEvilQuestsCompleted()    const { return sumOfList(denOfEvil);    }
        quint8 radamentQuestsCompleted()     const { return sumOfList(radament);     }
        quint8 goldenBirdQuestsCompleted()   const { return sumOfList(goldenBird);   }
        quint8 lamEsensTomeQuestsCompleted() const { return sumOfList(lamEsensTome); }
        quint8 izualQuestsCompleted()        const { return sumOfList(izual);        }
        quint8 rescueAnyaQuestsCompleted()   const { return sumOfList(rescueAnya);   }

        void clear() { denOfEvil.clear(); radament.clear(); goldenBird.clear(); lamEsensTome.clear(); izual.clear(); rescueAnya.clear(); }

    private:
        quint8 sumOfList(const QList<bool> &list) const { return std::accumulate(list.constBegin(), list.constEnd(), quint8(0)); }
    } questsInfo;

    struct
    { //-V802
        bool exists; // immutable
        quint16 nameIndex;
        Enums::Mercenary::MercenaryEnum code;
        quint32 experience; // immutable
        quint8 level; // immutable
    } mercenary;

    struct
    {
        ItemsList character;

        bool hasCube() { return std::find_if(character.constBegin(), character.constEnd(), isCubeInCharacterItems) != character.constEnd(); }
    } items;

    // itemsOffset points to the number of character items - just after the very first JM
    quint32 skillsOffset, itemsOffset, itemsEndOffset;

private:
    CharacterInfo() {}
    Q_DISABLE_COPY(CharacterInfo)
};

#endif // CHARACTERINFO_H
