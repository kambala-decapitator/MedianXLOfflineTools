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

        QObject statsDynamicData;
        quint16 totalStatPoints;
        
        SkillList skills;
        SkillList skillsReadable; // used in planner and skill tree
        quint16 totalSkillPoints;

        quint16 corpses;
    } basicInfo;

    struct
    {
        QList<bool> denOfEvil, radament, lamEsensTome, izual; // size == 3

        quint8 denOfEvilQuestsCompleted()    const { return sumOfList(denOfEvil);    }
        quint8 radamentQuestsCompleted()     const { return sumOfList(radament);     }
        quint8 lamEsensTomeQuestsCompleted() const { return sumOfList(lamEsensTome); }
        quint8 izualQuestsCompleted()        const { return sumOfList(izual);        }

        void clear() { denOfEvil.clear(); radament.clear(); lamEsensTome.clear(); izual.clear(); }

    private:
        quint8 sumOfList(const QList<bool> &list) const { return std::accumulate(list.constBegin(), list.constEnd(), quint8(0)); }
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
