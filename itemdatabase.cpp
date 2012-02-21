#include "itemdatabase.h"
#include "helpers.h"
#include "colors.hpp"
#include "resourcepathmanager.hpp"
#include "itemparser.h"

#include <QFile>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


QMultiHash<QString, QString> ItemDataBase::_sets;

ItemsList *ItemDataBase::currentCharacterItems = 0;
quint8 *ItemDataBase::clvl = 0;
Enums::ClassName::ClassNameEnum *ItemDataBase::charClass = 0;
QList<quint8> *ItemDataBase::charSkills = 0;

QHash<QByteArray, ItemBase> *ItemDataBase::Items()
{
    static QHash<QByteArray, ItemBase> allItems;
    if (allItems.isEmpty())
    {
        QFile f(ResourcePathManager::localizedPathForFileName("items"));
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            ERROR_BOX_NO_PARENT(tr("Items data not loaded.\nReason: %1").arg(f.errorString()));
            return 0;
        }

        while (!f.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(f);
            if (data.isEmpty())
                continue;

            ItemBase item;
            item.name = QString::fromUtf8(data.at(1));
            item.spelldesc = QString::fromUtf8(data.at(2)); // currently only misc items have it
            item.width = data.at(3).toUShort();
            item.height = data.at(4).toUShort();
            item.genericType = static_cast<Enums::ItemTypeGeneric::ItemTypeGenericEnum>(data.at(5).toUShort());
            item.isStackable = data.at(6).toUShort();
            item.rlvl = data.at(7).toUShort();
            item.imageName = data.at(8);
            item.typeString = data.at(9);
            item.socketableType = data.at(10).isEmpty() ? -1 : data.at(10).toShort();
            item.classCode = data.at(11).toShort();
            allItems[data.at(0)] = item;
        }
    }
    return &allItems;
}

QHash<QByteArray, QList<QByteArray> > *ItemDataBase::ItemTypes()
{
    static QHash<QByteArray, QList<QByteArray> > types;
    if (types.isEmpty())
    {
        QFile f(ResourcePathManager::dataPathForFileName("itemtypes.txt"));
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            ERROR_BOX_NO_PARENT(tr("Item types data not loaded.\nReason: %1").arg(f.errorString()));
            return 0;
        }

        while (!f.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(f);
            if (data.isEmpty())
                continue;

            types[data.at(0)] = data.at(1).split(',');
        }
    }
    return &types;
}

QHash<uint, ItemPropertyTxt> *ItemDataBase::Properties()
{
    static QHash<uint, ItemPropertyTxt> allProperties;
    if (allProperties.isEmpty())
    {
        QFile f(ResourcePathManager::localizedPathForFileName("props"));
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            ERROR_BOX_NO_PARENT(tr("Properties data not loaded.\nReason: %1").arg(f.errorString()));
            return 0;
        }

        while (!f.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(f);
            if (data.isEmpty())
                continue;

            ItemPropertyTxt item;
            item.add = data.at(1).toUShort();
            item.bits = data.at(2).toUShort();
            QList<QByteArray> groupIDs = data.at(3).split(',');
            if (!groupIDs.at(0).isEmpty())
                foreach (const QByteArray &id, groupIDs)
                    item.groupIDs += id.toUShort();
            item.descGroupNegative = QString::fromUtf8(data.at(4));
            item.descGroupPositive = QString::fromUtf8(data.at(5));
            item.descGroupStringAdd = QString::fromUtf8(data.at(6));
            item.descNegative = QString::fromUtf8(data.at(7));
            item.descPositive = QString::fromUtf8(data.at(8));
            item.descStringAdd = QString::fromUtf8(data.at(9));
            item.descFunc = data.at(10).toUShort();
            item.descPriority = data.at(11).toUShort();
            item.descVal = data.at(12).toUShort();
            item.descGroupFunc = data.at(13).toUShort();
            item.descGroupPriority = data.at(14).toUShort();
            item.descGroupVal = data.at(15).toUShort();
            item.saveParamBits = data.at(16).toUShort();
            allProperties[data.at(0).toUInt()] = item;
        }
    }
    return &allProperties;
}

QHash<uint, SetItemInfo> *ItemDataBase::Sets()
{
    static QHash<uint, SetItemInfo> allSets;
    if (allSets.isEmpty())
    {
        QFile f(ResourcePathManager::localizedPathForFileName("sets"));
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            ERROR_BOX_NO_PARENT(tr("Sets data not loaded.\nReason: %1").arg(f.errorString()));
            return 0;
        }

        while (!f.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(f);
            if (data.isEmpty())
                continue;

            SetItemInfo item;
            item.itemName = QString::fromUtf8(data.at(1));
            item.setName = QString::fromUtf8(data.at(2));
            allSets[data.at(0).toUInt()] = item;

            // do not add duplicate names
            if (_sets.count(item.setName) < 5)
                _sets.insert(item.setName, item.itemName);
        }
    }
    return &allSets;
}

QList<SkillInfo> *ItemDataBase::Skills()
{
    static QList<SkillInfo> allSkills;
    if (allSkills.isEmpty())
    {
        QFile f(ResourcePathManager::localizedPathForFileName("skills"));
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            ERROR_BOX_NO_PARENT(tr("Skills data not loaded.\nReason: %1").arg(f.errorString()));
            return 0;
        }

        while (!f.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(f);
            if (data.isEmpty())
                continue;

            SkillInfo item;
            item.name = QString::fromUtf8(data.at(1));
            item.classCode = data.at(2).toShort();
            allSkills.push_back(item);
        }
    }
    return &allSkills;
}

QHash<uint, UniqueItemInfo> *ItemDataBase::Uniques()
{
    static QHash<uint, UniqueItemInfo> allUniques;
    if (allUniques.isEmpty())
    {
        QFile f(ResourcePathManager::localizedPathForFileName("uniques"));
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            ERROR_BOX_NO_PARENT(tr("Skills data not loaded.\nReason: %1").arg(f.errorString()));
            return 0;
        }

        while (!f.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(f);
            if (data.isEmpty())
                continue;

            UniqueItemInfo item;
            item.name = QString::fromUtf8(data.at(1));
            item.rlvl = data.at(2).toUShort();
            allUniques[data.at(0).toUInt()] = item;
        }
    }
    return &allUniques;
}

QHash<uint, MysticOrb> *ItemDataBase::MysticOrbs()
{
    static QHash<uint, MysticOrb> allMysticOrbs;
    if (allMysticOrbs.isEmpty())
    {
        QFile f(ResourcePathManager::dataPathForFileName("mo.txt"));
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            ERROR_BOX_NO_PARENT(tr("Mystic Orbs data not loaded.\nReason: %1").arg(f.errorString()));
            return 0;
        }
        
        while (!f.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(f);
            if (data.isEmpty())
                continue;

            MysticOrb item;
            item.itemCode = data.at(1);
            item.statId = data.at(2).toUShort();
            item.value = data.at(3).toUShort();
            allMysticOrbs[data.at(0).toUInt()] = item;
        }
    }
    return &allMysticOrbs;
}

QHash<uint, QString> *ItemDataBase::Monsters()
{
    static QHash<uint, QString> allMonsters;
    if (allMonsters.isEmpty())
    {
        QFile f(ResourcePathManager::localizedPathForFileName("monsters"));
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            ERROR_BOX_NO_PARENT(tr("Monster names not loaded.\nReason: %1").arg(f.errorString()));
            return 0;
        }

        while (!f.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(f);
            if (data.isEmpty())
                continue;

            allMonsters[data.at(0).toUInt()] = QString::fromUtf8(data.at(1));
        }
    }
    return &allMonsters;
}

QMultiHash<RunewordKeyPair, RunewordInfo> *ItemDataBase::RW()
{
    static QMultiHash<RunewordKeyPair, RunewordInfo> allRunewords;
    if (allRunewords.isEmpty())
    {
        QFile f(ResourcePathManager::localizedPathForFileName("rw"));
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            ERROR_BOX_NO_PARENT(tr("Runewords data not loaded.\nReason: %1").arg(f.errorString()));
            return 0;
        }

        while (!f.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(f);
            if (data.isEmpty())
                continue;

            RunewordInfo item;
            item.allowedItemTypes.reserve(6);
            for (int i = 0; i < 6; ++i)
                item.allowedItemTypes << data.at(i);
            item.name = QString::fromUtf8(data.at(6));
            allRunewords.insert(qMakePair(data.at(7), data.size() == 9 ? data.at(8) : QByteArray()), item);
        }
    }
    return &allRunewords;
}

QHash<QByteArray, SocketableItemInfo> *ItemDataBase::Socketables()
{
    static QHash<QByteArray, SocketableItemInfo> allSocketables;
    if (allSocketables.isEmpty())
    {
        QFile f(ResourcePathManager::localizedPathForFileName("socketables"));
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            ERROR_BOX_NO_PARENT(tr("Socketables data not loaded.\nReason: %1").arg(f.errorString()));
            return 0;
        }

        while (!f.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(f);
            if (data.isEmpty())
                continue;

            SocketableItemInfo item;
            item.name = QString::fromUtf8(data.at(1));
            item.letter = QString::fromUtf8(data.at(2));
            for (int i = SocketableItemInfo::Armor, firstCol = 3; i <= SocketableItemInfo::Weapon; ++i)
            {
                QList<SocketableItemInfo::Properties> props;
                for (int j = 0; j < 3; ++j, firstCol += 3)
                {
                    if (firstCol >= data.size())
                        break;
                    QByteArray codeString = data.at(firstCol);
                    if (codeString.isEmpty())
                    {
                        firstCol += (3 - j) * 3;
                        break;
                    }
                    int param = data.at(firstCol + 1).toInt(), value = data.at(firstCol + 2).toInt();
                    foreach (const QByteArray &code, codeString.split(','))
                        props += SocketableItemInfo::Properties(code.toInt(), param, value);
                }
                item.properties[static_cast<SocketableItemInfo::PropertyType>(i)] = props;
            }
            allSocketables[data.at(0)] = item;
        }
    }
    return &allSocketables;
}

QStringList *ItemDataBase::NonMagicItemQualities()
{
    static QStringList allQualities;
    if (allQualities.isEmpty())
    {
        QFile f(ResourcePathManager::localizedPathForFileName("LowQualityItems"));
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            ERROR_BOX_NO_PARENT(tr("Non-magic qualities data not loaded.\nReason: %1").arg(f.errorString()));
            return 0;
        }

        while (!f.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(f);
            if (!data.isEmpty())
                allQualities << QString::fromUtf8(data.at(0));
        }
    }
    return &allQualities;
}

QList<QByteArray> ItemDataBase::stringArrayOfCurrentLineInFile(QFile &f)
{
    QByteArray itemString = f.readLine().trimmed();
    return itemString.isEmpty() || itemString.startsWith('#') ? QList<QByteArray>() : itemString.split('\t');
}

QString ItemDataBase::completeItemName(ItemInfo *item, bool shouldUseColor, bool showQualityText /*= true*/)
{
    QString itemName = Items()->value(item->itemType).name, nonMagicalQuality;
    if (item->quality == Enums::ItemQuality::LowQuality)
        nonMagicalQuality = NonMagicItemQualities()->at(item->nonMagicType);
    else if (item->quality == Enums::ItemQuality::HighQuality)
        nonMagicalQuality = NonMagicItemQualities()->last();
    if (nonMagicalQuality.startsWith('[')) // gender/number
    {
        int genderSize = 4, actualTypeStart = nonMagicalQuality.indexOf(itemName.left(genderSize)) + genderSize;
        nonMagicalQuality = nonMagicalQuality.mid(actualTypeStart, nonMagicalQuality.indexOf('[', actualTypeStart) - actualTypeStart);
    }
    if (itemName.startsWith('[')) // gender/number
        itemName.remove(0, 4);
    if (!nonMagicalQuality.isEmpty())
        itemName.prepend(nonMagicalQuality + " ");

    QString specialName;
    if (item->quality == Enums::ItemQuality::Set)
    {
        const SetItemInfo &setItem = Sets()->value(item->setOrUniqueId);
        specialName = setItem.itemName;
        if (!shouldUseColor)
            specialName += QString(" [%1]").arg(setItem.setName);
    }
    else if (item->quality == Enums::ItemQuality::Unique)
        specialName = Uniques()->value(item->setOrUniqueId).name;
    else if (item->isRW)
        specialName = item->rwName;

    if (isUberCharm(item))
    {
        // choose the longest name
        if (specialName.length() > itemName.length())
            itemName = specialName;
        specialName.clear();
    }

    if (!item->inscribedName.isEmpty())
        (specialName.isEmpty() || item->isRW ? itemName : specialName).prepend(tr("%1's ", "personalized name").arg(item->inscribedName));

    if (shouldUseColor)
    {
        if (item->isRW)
        {
            itemName = htmlStringFromDiabloColorString(itemName, DarkGrey);
            specialName = htmlStringFromDiabloColorString(specialName, Gold);
            itemName.prepend(specialName + htmlLineBreak);
        }
        else
        {
            bool hasSpecialName = !specialName.isEmpty() && specialName != itemName;
            ColorIndex colorIndex = colorOfItem(item);
            itemName = htmlStringFromDiabloColorString(itemName, colorIndex);
            if (hasSpecialName)
            {
                specialName = htmlStringFromDiabloColorString(specialName, colorIndex);
                itemName.prepend(specialName + htmlLineBreak);
            }
        }
    }
    else
    {
        if (!specialName.isEmpty() && specialName != itemName)
            itemName += "\\n" + specialName;
        expandMultilineString(&itemName);

        if (showQualityText)
        {
            QString quality;
            switch (item->quality)
            {
            case Enums::ItemQuality::Magic:
                quality = tr("magic");
                break;
            case Enums::ItemQuality::Set:
                quality = tr("set");
                break;
            case Enums::ItemQuality::Rare:
                quality = tr("rare");
                break;
            case Enums::ItemQuality::Unique:
                quality = isUberCharm(item) ? tr("charm") : tr("unique");
                break;
            case Enums::ItemQuality::Crafted:
                quality = tr("crafted");
                break;
            case Enums::ItemQuality::Honorific:
                quality = tr("honorific");
                break;
            default:
                break;
            }
            if (!quality.isEmpty()) // skip non-magic types
                itemName.prepend(QString("[%1]%2").arg(quality, htmlLineBreak));
            else if (item->isRW)
                itemName.prepend(QString("[%1]%2").arg(tr("runeword"), htmlLineBreak));

            QString spelldesc = Items()->value(item->itemType).spelldesc;
            if (!spelldesc.isEmpty())
            {
                expandMultilineString(&spelldesc);
                itemName += htmlLineBreak + spelldesc;
            }

            itemName.remove("\\grey;");
            foreach (const QByteArray &colorString, colorStrings)
                itemName.remove(colorString);

            if (item->isEthereal)
                itemName += QString("%1[%2]").arg(htmlLineBreak, tr("ethereal"));
        }
    }
    return itemName;
}

void ItemDataBase::expandMultilineString(QString *stringToExpand)
{
    QStringList lines = stringToExpand->split("\\n");
    std::reverse(lines.begin(), lines.end());
    *stringToExpand = lines.join(htmlLineBreak);
}

QHash<int, ColorIndex> *ItemDataBase::itemQualityColorsHash()
{
    static QHash<int, ColorIndex> colorsHash;
    if (colorsHash.isEmpty())
    {
        colorsHash[Enums::ItemQuality::Set] = Green;
        colorsHash[Enums::ItemQuality::Unique] = Gold;
        colorsHash[Enums::ItemQuality::Crafted] = Orange;
        colorsHash[Enums::ItemQuality::Rare] = Yellow;
        colorsHash[Enums::ItemQuality::Magic] = Blue;
        colorsHash[Enums::ItemQuality::Honorific] = DarkGreen;
    }
    return &colorsHash;
}

ColorIndex ItemDataBase::colorOfItem(ItemInfo *item)
{
    return !itemQualityColorsHash()->contains(item->quality) && (item->isSocketed || item->isEthereal) ? DarkGrey : itemQualityColorsHash()->value(item->quality);
}

ItemInfo *ItemDataBase::loadItemFromFile(const QString &fileName)
{
    ItemInfo *item = 0;
    QString filePath = ResourcePathManager::pathForResourceItem(fileName);
    QFile itemFile(filePath);
    if (itemFile.open(QIODevice::ReadOnly))
    {
        QByteArray itemBytes = itemFile.readAll();
        itemFile.close();

        QDataStream ds(itemBytes);
        ds.setByteOrder(QDataStream::LittleEndian);

        item = ItemParser::parseItem(ds, itemBytes);
        item->hasChanged = true;
        item->row = item->column = -1;
    }
    else
        ERROR_BOX_NO_PARENT(tr("Error loading '%1'").arg(filePath) + "\n" + tr("Reason: %1", "error with file").arg(itemFile.errorString()));
    return item;
}

ItemsList ItemDataBase::itemsStoredIn(int storage, int location /*= Enums::ItemLocation::Stored*/, quint32 *pPlugyPage /*= 0*/, ItemsList *allItems /*= 0*/)
{
    ItemsList items, *characterItems = allItems ? allItems : currentCharacterItems;
    for (int i = 0; i < characterItems->size(); ++i)
    {
        ItemInfo *item = characterItems->at(i);
        if (item->storage == storage && item->location == location && (!pPlugyPage || pPlugyPage && item->plugyPage == *pPlugyPage))
            items += item;
    }
    return items;
}

bool ItemDataBase::storeItemIn(ItemInfo *item, Enums::ItemStorage::ItemStorageEnum storage, quint8 rowsTotal, quint32 plugyPage /*= 0*/, quint8 colsTotal /*= 10*/)
{
    ItemsList items = itemsStoredIn(storage, Enums::ItemLocation::Stored, plugyPage ? &plugyPage : 0);
    for (quint8 i = 0; i < rowsTotal; ++i)
        for (quint8 j = 0; j < colsTotal; ++j)
            if (canStoreItemAt(i, j, item->itemType, items, rowsTotal, colsTotal))
            {
                item->storage = storage;
                item->row = i;
                item->column = j;
                return true;
            }

            return false;
}

bool ItemDataBase::canStoreItemAt(quint8 row, quint8 col, const QByteArray &storeItemType, const ItemsList &items, int rowsTotal, quint32 plugyPage /*= 0*/, int colsTotal /*= 10*/)
{
    // col is horizontal (x), row is vertical (y)
    const ItemBase &storeItemBase = ItemDataBase::Items()->value(storeItemType);
    QRect storeItemRect(col, row, storeItemBase.width, storeItemBase.height);
    if (storeItemRect.right() >= colsTotal || storeItemRect.bottom() >= rowsTotal) // beyond grid
        return false;

    bool ok = true;
    foreach (ItemInfo *item, items)
    {
        const ItemBase &itemBase = ItemDataBase::Items()->value(item->itemType);
        if (storeItemRect.intersects(QRect(item->column, item->row, itemBase.width, itemBase.height)))
        {
            ok = false;
            break;
        }
    }
    return ok;
}

bool ItemDataBase::isClassCharm(ItemInfo *item)
{
    return Items()->value(item->itemType).typeString.startsWith("ara");
}

bool ItemDataBase::isUberCharm(ItemInfo *item)
{
    return ItemParser::itemTypeInheritsFromTypes(Items()->value(item->itemType).typeString, QList<QByteArray>() << "char");
}

bool ItemDataBase::isGenericSocketable(ItemInfo *item)
{
    return Socketables()->contains(item->itemType);
}

bool ItemDataBase::isCube(ItemInfo *item)
{
    return item->itemType == "box";
}

bool ItemDataBase::hasCube()
{
    return std::find_if(currentCharacterItems->constBegin(), currentCharacterItems->constEnd(), isCubeItem) != currentCharacterItems->constEnd();
}
