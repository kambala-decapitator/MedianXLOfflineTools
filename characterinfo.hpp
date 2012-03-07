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

    struct CharacterInfoBasic
    {
        QString originalName, newName;
        Enums::ClassName::ClassNameEnum classCode; // immutable
        quint8 titleCode; // immutable
        quint8 level;
        //quint8 currentDifficulty, currentAct; // not used atm
        bool isHardcore;
        bool hadDied;
        quint16 totalSkillPoints, totalStatPoints;
        SkillList skills;
        SkillList skillsReadable; // used in planner and skill tree
    } basicInfo;

    struct
    {
        QList<quint8> denOfEvil, radament, lamEsensTome, izual; // range of values is 0-3

        quint8 denOfEvilQuestsCompleted()    const { return sumOfList(denOfEvil);    }
        quint8 radamentQuestsCompleted()     const { return sumOfList(radament);     }
        quint8 lamEsensTomeQuestsCompleted() const { return sumOfList(lamEsensTome); }
        quint8 izualQuestsCompleted()        const { return sumOfList(izual);        }

        void clear() { denOfEvil.clear(); radament.clear(); lamEsensTome.clear(); izual.clear(); }

    private:
        quint8 sumOfList(const QList<quint8> &list) const { return std::accumulate(list.constBegin(), list.constEnd(), quint8(0)); }
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

private:
    CharacterInfo() {}
};

#endif // CHARACTERINFO_H
