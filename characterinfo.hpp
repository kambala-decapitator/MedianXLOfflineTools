#ifndef CHARACTERINFO_H
#define CHARACTERINFO_H

#include "structs.h"

#include <QString>

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
        QList<quint8> skills;
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

private:
    CharacterInfo() {}
};

#endif // CHARACTERINFO_H
