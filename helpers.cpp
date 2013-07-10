#include "helpers.h"
#include "structs.h"
#include "itemdatabase.h"
#include "itemparser.h"

#include <QTreeWidgetItem>
#include <QTreeView>

#include <QString>
#include <QStack>

#include <algorithm>


// private

bool isRWInGear(ItemInfo *item, const QByteArray &rune, const QByteArray &allowedItemType)
{
    if (item->isRW && item->location == Enums::ItemLocation::Equipped && ItemParser::itemTypesInheritFromType(ItemDataBase::Items()->value(item->itemType)->types, allowedItemType))
    {
        const RunewordHash *const rwHash = ItemDataBase::RW();
        RunewordKeyPair rwKey = qMakePair(rune, QByteArray());
        for (RunewordHash::const_iterator iter = rwHash->find(rwKey); iter != rwHash->end() && iter.key() == rwKey; ++iter)
            if (ItemParser::itemTypeInheritsFromTypes(allowedItemType, iter.value()->allowedItemTypes))
                return true;
    }
    return false;
}


// public

QString binaryStringFromNumber(quint64 number, bool needsInversion /*= false*/, int fieldWidth /*= 8*/)
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
        QPair<int, int> colorStringIndex = colorStringsIndeces.at(i);
        int index = colorStringIndex.first;
        int pos = colorStringIndex.second + ColorsManager::colorStrings().at(index).length(); // skip colorString
        QString coloredText_ = text.mid(pos, i != colorsNumberInString - 1 ? colorStringsIndeces.at(i + 1).second - pos : -1);

        QStringList lines = coloredText_.split(kHtmlLineBreak);
        QString reversedLines;
        for (QStringList::const_iterator iter = lines.constEnd() - 1; iter != lines.constBegin() - 1; --iter)
        {
            reversedLines.append(*iter);
            if (iter != lines.constBegin())
                reversedLines.append(kHtmlLineBreak);
        }
        if (!reversedLines.isEmpty())
        {
            QString newText = coloredText(reversedLines, index);
            if (!i && colorStringIndex.second > 0) // quick fix for '+1 to \red;Ultimative\blue;'
                newText.prepend(text.left(colorStringIndex.second));
            colorStringsStack.push(newText);
        }
    }

    // empty stack
    QString result;
    while (!colorStringsStack.isEmpty())
        result += colorStringsStack.pop();
    return result;
}

QList<QTreeWidgetItem *> treeItemsForItems(const ItemsList &items)
{
    QList<QTreeWidgetItem *> treeItems;
    foreach (ItemInfo *item, items)
    {
        qApp->processEvents();
        QString htmlName = ItemDataBase::completeItemName(item, true);
        QStringList list;
        QList<QColor> colors;

        QRegExp rx("<font color = \"(.+)\">(.+)</font>");
        rx.setMinimal(true);
        int matchIndex = 0;
        while ((matchIndex = rx.indexIn(htmlName, matchIndex)) != -1)
        {
            matchIndex += rx.cap().length();
            colors.prepend(rx.cap(1));
            list.prepend(rx.cap(2).replace(kHtmlLineBreak, " ", Qt::CaseInsensitive).trimmed());
        }

        QTreeWidgetItem *childItem = new QTreeWidgetItem(list);
        if (item->plugyPage)
        {
            static const struct { const char *source; const char *comment; } kPageFormat = QT_TRANSLATE_NOOP3("ItemsTree", "p. %1", "page abbreviation");
            childItem->setText(0, QString("[%1] %2").arg(qApp->translate("ItemsTree", kPageFormat.source, kPageFormat.comment).arg(item->plugyPage), childItem->text(0)));
        }
        for (int j = 0; j < list.size(); ++j)
            childItem->setForeground(j, colors.at(j));
        treeItems += childItem;
    }
    return treeItems;
}

void customizeItemsTreeView(QTreeView *treeView)
{
    treeView->setStyleSheet(
        "QTreeView                { background-color: black; }"
        "QTreeView::item          { selection-color: red; }"
        "QTreeView::item:hover    { border: 1px solid #bfcde4; }"
        "QTreeView::item:selected { border: 1px solid #567dbc; }"
        //"QTreeView::item:hover    { background-color: blue; }"
        //"QTreeView::item:selected { background-color: rgba(255,0,0,50%); }"
        );
    treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    treeView->setDropIndicatorShown(false);
}


bool isTiered(ItemInfo *item)
{
    return isTiered(ItemDataBase::Items()->value(item->itemType)->types);
}

bool isTiered(const QList<QByteArray> &itemTypes)
{
    return itemTypes.contains("tier");
}

bool isSacred(ItemInfo *item)
{
    return !isTiered(item);
}

bool isSacred(const QList<QByteArray> &itemTypes)
{
    return !isTiered(itemTypes);
}

bool areBothItemsSetOrUnique(ItemInfo *a, ItemInfo *b)
{
    return (a->quality == Enums::ItemQuality::Unique && b->quality == Enums::ItemQuality::Unique) || (a->quality == Enums::ItemQuality::Set && b->quality == Enums::ItemQuality::Set);
}


bool isUltimative()
{
    return isUltimative4() || isUltimative5OrLater();
}

bool isUltimative4()
{
    return ItemDataBase::Properties()->value(Enums::CharacterStats::Strength)->saveBits != 12;
}

bool isUltimative5OrLater()
{
    return ItemDataBase::Properties()->value(Enums::CharacterStats::FreeSkillPoints)->saveBits != 8;
}

bool isSigma()
{
    return ItemDataBase::Properties()->value(Enums::CharacterStats::Level)->saveBits != 7;
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

bool isArcaneShard(ItemInfo *item)
{
    return item->itemType == "qul";
}

bool isSignetOfLearning(ItemInfo *item)
{
    return item->itemType == "zkq" || item->itemType == "zkc";
}

bool isCotw(ItemInfo *item)
{
    return item->quality == Enums::ItemQuality::Unique && item->itemType == "jew" && item->props.size() <= 2 && item->props.contains(Enums::ItemProperties::Oskill);
}


bool isCharacterOrb(const QByteArray &itemType)
{
    return itemType == ">.<";
}

bool isCharacterOrb(ItemInfo *item)
{
    return isCharacterOrb(item->itemType);
}

bool isSunstoneOfElements(const QByteArray &itemType)
{
    return itemType == "x#x";
}

bool isSunstoneOfElements(ItemInfo *item)
{
    return isSunstoneOfElements(item->itemType);
}

bool isTradersChest(const QByteArray &itemType)
{
    return itemType == "`^`";
}

bool isTradersChest(ItemInfo *item)
{
    return isTradersChest(item->itemType);
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

bool compareItemsByRlvl(ItemInfo *a, ItemInfo *b)
{
    if (areBothItemsSetOrUnique(a, b) && isSacred(a) && isSacred(b))
        return a->setOrUniqueId < b->setOrUniqueId;
    return ItemDataBase::Items()->value(a->itemType)->rlvl < ItemDataBase::Items()->value(b->itemType)->rlvl;
}

bool compareItemsByRlvlAndEthereality(ItemInfo *a, ItemInfo *b)
{
    bool ethCompareResult = a->isEthereal < b->isEthereal;
    if (areBothItemsSetOrUnique(a, b) && isSacred(a) && isSacred(b))
        return a->setOrUniqueId == b->setOrUniqueId ? ethCompareResult : a->setOrUniqueId < b->setOrUniqueId;

    quint16 aRlvl = ItemDataBase::Items()->value(a->itemType)->rlvl, bRlvl = ItemDataBase::Items()->value(b->itemType)->rlvl;
    return aRlvl == bRlvl ? ethCompareResult : aRlvl < bRlvl;
}

bool compareItemsByCode(ItemInfo *a, ItemInfo *b)
{
    return a->itemType < b->itemType;
}
