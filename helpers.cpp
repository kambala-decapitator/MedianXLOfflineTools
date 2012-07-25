#include "helpers.h"
#include "structs.h"
#include "itemdatabase.h"
#include "itemparser.h"

#include <QString>
#include <QStack>

#include <algorithm>


// private

bool isRWInGear(ItemInfo *item, const QByteArray &rune, const QByteArray &allowedItemType)
{
    if (item->isRW && item->location == Enums::ItemLocation::Equipped && ItemParser::itemTypesInheritFromType(ItemDataBase::Items()->operator[](item->itemType).types, allowedItemType))
    {
        const RunewordHash *const rwHash = ItemDataBase::RW();
        RunewordKeyPair rwKey = qMakePair(rune, QByteArray());
        for (RunewordHash::const_iterator iter = rwHash->find(rwKey); iter != rwHash->end() && iter.key() == rwKey; ++iter)
            if (ItemParser::itemTypeInheritsFromTypes(allowedItemType, iter.value().allowedItemTypes))
                return true;
    }
    return false;
}


// public

QString binaryStringFromNumber(quint64 number, bool needsInversion, int fieldWidth)
{
    QString binaryString = QString("%1").arg(number, fieldWidth, 2, kZeroChar);
    if (needsInversion)
        std::reverse(binaryString.begin(), binaryString.end());
    return binaryString;
}

QString coloredText(const QString &text, int colorIndex)
{
    return QString("<font color = \"%1\">%2</font>").arg(ColorsManager::colors().at(colorIndex).name(), text);
}

bool colorStringsIndecesLessThan(const QPair<int, int> &a, const QPair<int, int> &b)
{
    return a.second < b.second;
}

QString htmlStringFromDiabloColorString(const QString &name, ColorsManager::ColorIndex defaultColor /*= White*/)
{
    QString text = name;
    if (defaultColor != ColorsManager::NoColor)
        text.prepend(ColorsManager::colorStrings().at(defaultColor));
    text.replace("\\n", kHtmlLineBreak).replace("\\grey;", ColorsManager::colorStrings().at(ColorsManager::White));

    QList<QPair<int, int> > colorStringsIndeces; // <index_of_color_string_in_array, position_in_string>
    for (int i = 0; i < ColorsManager::colors().size(); ++i)
    {
        QString colorString = ColorsManager::colorStrings().at(i);
        for (int j = 0, n = text.count(colorString), pos = 0; j < n; ++j, pos += colorString.length())
        {
            pos = text.indexOf(colorString, pos);
            colorStringsIndeces += qMakePair(i, pos);
        }
    }

    // sort colorStringsIndeces by position in ascending order
    qSort(colorStringsIndeces.begin(), colorStringsIndeces.end(), colorStringsIndecesLessThan);

    int colorsNumberInString = colorStringsIndeces.size();
    if (!colorsNumberInString)
        return text;

    QStack<QString> colorStringsStack;
    for (int i = 0; i < colorsNumberInString; ++i)
    {
        int index = colorStringsIndeces.at(i).first;
        int pos = colorStringsIndeces.at(i).second + ColorsManager::colorStrings().at(index).length(); // skip colorString
        QString coloredText_ = text.mid(pos, i != colorsNumberInString - 1 ? colorStringsIndeces.at(i + 1).second - pos : -1);

        QStringList lines = coloredText_.split(kHtmlLineBreak);
        QString reversedLines;
        for (QStringList::const_iterator iter = lines.end() - 1; iter != lines.begin() - 1; --iter)
        {
            reversedLines.append(*iter);
            if (iter != lines.begin())
                reversedLines.append(kHtmlLineBreak);
        }
        if (!reversedLines.isEmpty())
            colorStringsStack.push(coloredText(reversedLines, index));
    }

    // empty stack
    QString result;
    while (!colorStringsStack.isEmpty())
        result += colorStringsStack.pop();
    return result;
}

bool isUltimative()
{
    return isUltimative4() || isUltimative5OrLater();
}

bool isUltimative4()
{
    return ItemDataBase::Properties()->operator[](Enums::CharacterStats::Strength).saveBits != 12;
}

bool isUltimative5OrLater()
{
    return ItemDataBase::Properties()->operator[](Enums::CharacterStats::FreeSkillPoints).saveBits != 8;
}

bool isCubeInCharacterItems(ItemInfo *item)
{
    return (item->storage == Enums::ItemStorage::Inventory || item->storage == Enums::ItemStorage::Stash) && ItemDataBase::isCube(item);
}

bool hasChanged(ItemInfo *item)
{
    return item->hasChanged;
}

bool isTiered(ItemInfo *item)
{
    return ItemDataBase::Items()->operator[](item->itemType).types.contains("tier");
}

bool isSacred(ItemInfo *item)
{
    return !isTiered(item);
}

bool isClassCharm(ItemInfo *item)
{
    return ItemDataBase::isClassCharm(item);
}

bool isCrystallineFlameMedallion(ItemInfo *item)
{
    return item->itemType == "lok";
}

bool isMoonOfSpider(ItemInfo *item)
{
    return item->itemType == "yqe";
}

bool isLegacyOfBlood(ItemInfo *item)
{
    return item->itemType == "adi";
}

bool isDrekavacInGear(ItemInfo *item)
{
    return isRWInGear(item, "r51", "glov"); // Taha in gloves
}

bool isVeneficaInGear(ItemInfo *item)
{
    return isRWInGear(item, "r53", "stor"); // Qor in sorc armor
}

bool isArcaneShard(ItemInfo *item)
{
    return item->itemType == "qul";
}

bool isSignetOfLearning(ItemInfo *item)
{
    return item->itemType == "zkq" || item->itemType == "zkc";
}

bool isCharacterOrb(const QByteArray &itemType)
{
    return itemType == ">.<";
}

bool isSunstoneOfElements(const QByteArray &itemType)
{
    return itemType == "x#x";
}

bool isCharacterOrbOrSunstoneOfElements(const QByteArray &itemType)
{
    return isSunstoneOfElements(itemType) || isCharacterOrb(itemType);
}

bool isCharacterOrb(ItemInfo *item)
{
    return isCharacterOrb(item->itemType);
}

bool isSunstoneOfElements(ItemInfo *item)
{
    return isSunstoneOfElements(item->itemType);
}

bool isCharacterOrbOrSunstoneOfElements(ItemInfo *item)
{
    return isCharacterOrbOrSunstoneOfElements(item->itemType);
}

bool isArcaneShard2(ItemInfo *item)
{
    return item->itemType == "#3^";
}

bool isArcaneShard3(ItemInfo *item)
{
    return item->itemType == "#4^";
}

bool isArcaneShard4(ItemInfo *item)
{
    return item->itemType == "#5^";
}


bool compareItemsByPlugyPage(ItemInfo *a, ItemInfo *b)
{
    return a->plugyPage < b->plugyPage;
}
