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
    if (item->isRW && item->location == Enums::ItemLocation::Equipped && ItemParser::itemTypeInheritsFromType(ItemDataBase::Items()->value(item->itemType).typeString, allowedItemType))
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

QString colorHexString(const QColor &c)
{
    return c.name();//QString("#%1%2%3").arg(c.red(), 2, 16, kZeroChar).arg(c.green(), 2, 16, kZeroChar).arg(c.blue(), 2, 16, kZeroChar);
}

QString coloredText(const QString &text, int colorIndex)
{
    return QString("<font color = \"%1\">%2</font>").arg(colorHexString(ColorsManager::colors().at(colorIndex)), text);
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
        int occurencesCount = text.count(colorString), position = 0, length = colorString.length();
        for (int j = 0; j < occurencesCount; ++j, position += length)
        {
            position = text.indexOf(colorString, position);
            colorStringsIndeces += qMakePair(i, position);
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
        int position = colorStringsIndeces.at(i).second + ColorsManager::colorStrings().at(index).length(); // skip colorString
        QString coloredText_ = text.mid(position, i != colorsNumberInString - 1 ? colorStringsIndeces.at(i + 1).second - position : -1);

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
    return ItemDataBase::Properties()->value(Enums::CharacterStats::Strength).saveBits != 12;
}

bool isCubeInCharacterItems(ItemInfo *item)
{
    return (item->storage == Enums::ItemStorage::Inventory || item->storage == Enums::ItemStorage::Stash) && ItemDataBase::isCube(item);
}

bool hasChanged(ItemInfo *item)
{
    return item->hasChanged;
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

bool isSacred(ItemInfo *item)
{
    return ItemParser::itemTypeInheritsFromTypes(ItemDataBase::Items()->value(item->itemType).typeString, QList<QByteArray>() << "ct1a" << "ct2a" << "ct1w" << "ct2w");
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
