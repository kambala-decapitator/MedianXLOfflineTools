#include "itemdatabase.h"
#include "helpers.h"
#include "colorsmanager.hpp"
#include "resourcepathmanager.hpp"
#include "itemparser.h"
#include "characterinfo.hpp"
#include "reversebitwriter.h"

#include <QBuffer>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif

#define TO_UINT16(byteArray) (static_cast<quint8>(byteArray.at(0)) + (static_cast<quint8>(byteArray.at(1)) << 8))
#define CRC_OF_BYTEARRAY(byteArray) qChecksum(byteArray.constData(), byteArray.length())


const char *const ItemDataBase::kJewelType = "jew";
QHash<QByteArray, FullSetInfo> ItemDataBase::_sets;

QByteArray ItemDataBase::decompressedFileData(const QString &compressedFilePath, const QString &errorMessage)
{
    QFile f(compressedFilePath);
    if (!f.open(QIODevice::ReadOnly))
    {
        ERROR_BOX_NO_PARENT(errorMessage + "\n" + tr("Reason: %1").arg(f.errorString()));
        return QByteArray();
    }

    QByteArray compressedCrcData = f.read(2), originalCrcData = f.read(2);
    quint16 compressedCrc = TO_UINT16(compressedCrcData), originalCrc = TO_UINT16(originalCrcData);
    static const QString decompressError(tr("Error decrypting file '%1'"));
    QByteArray compressedDataFile = f.readAll();
    if (CRC_OF_BYTEARRAY(compressedDataFile) != compressedCrc)
    {
        ERROR_BOX_NO_PARENT(decompressError.arg(compressedFilePath));
        return QByteArray();
    }

    QByteArray originalFileData = qUncompress(compressedDataFile);
    if (CRC_OF_BYTEARRAY(originalFileData) != originalCrc)
    {
        ERROR_BOX_NO_PARENT(decompressError.arg(compressedFilePath));
        return QByteArray();
    }

    return originalFileData;
}

QHash<QByteArray, ItemBase *> *ItemDataBase::Items()
{
    static QHash<QByteArray, ItemBase *> allItems;
    if (allItems.isEmpty())
    {
        QByteArray fileData = decompressedFileData(ResourcePathManager::localizedPathForFileName("items"), tr("Items data not loaded."));
        if (fileData.isEmpty())
            return 0;

        QBuffer buf(&fileData);
        if (!buf.open(QIODevice::ReadOnly))
            return 0;
        while (!buf.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(buf);
            if (data.isEmpty())
                continue;

            ItemBase *item = new ItemBase;
            item->name = QString::fromUtf8(data.at(1));
            item->spelldesc = QString::fromUtf8(data.at(2)); // currently only misc items have it
            item->width = data.at(3).toUShort();
            item->height = data.at(4).toUShort();
            item->genericType = static_cast<Enums::ItemTypeGeneric::ItemTypeGenericEnum>(data.at(5).toUShort());
            item->isStackable = data.at(6).toUShort();
            item->rlvl = data.at(7).toUShort();
            item->rstr = data.at(8).toUShort();
            item->rdex = data.at(9).toUShort();
            item->is1h2h = data.at(10).toUShort();
            item->is2h = data.at(11).toUShort();
            item->min1hDmg = data.at(12).toUShort();
            item->max1hDmg = data.at(13).toUShort();
            item->min2hDmg = data.at(14).toUShort();
            item->max2hDmg = data.at(15).toUShort();
            item->minThrowDmg = data.at(16).toUShort();
            item->maxThrowDmg = data.at(17).toUShort();
            item->imageName = data.at(18);
            item->questId = data.at(19).toUShort();
            item->types = data.at(20).split(',');
            item->socketableType = data.at(21).isEmpty() ? -1 : data.at(21).toShort();
            item->classCode = data.at(22).toShort();
            allItems[data.at(0)] = item;
        }
    }
    return &allItems;
}

QHash<QByteArray, ItemType> *ItemDataBase::ItemTypes()
{
    static QHash<QByteArray, ItemType> types;
    if (types.isEmpty())
    {
        QByteArray fileData = decompressedFileData(ResourcePathManager::dataPathForFileName("itemtypes.dat"), tr("Item types data not loaded."));
        if (fileData.isEmpty())
            return 0;

        QBuffer buf(&fileData);
        if (!buf.open(QIODevice::ReadOnly))
            return 0;
        while (!buf.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(buf);
            if (data.isEmpty())
                continue;

            ItemType type;
            type.baseItemTypes = data.at(1).split(',');
            if (data.size() > 2)
                type.variableImageNames = data.at(2).split(',');
            types[data.at(0)] = type;
        }
    }
    return &types;
}

QHash<uint, ItemPropertyTxt *> *ItemDataBase::Properties()
{
    static QHash<uint, ItemPropertyTxt *> allProperties;
    if (allProperties.isEmpty())
    {
        QByteArray fileData = decompressedFileData(ResourcePathManager::localizedPathForFileName("props"), tr("Properties data not loaded."));
        if (fileData.isEmpty())
            return 0;

        QBuffer buf(&fileData);
        if (!buf.open(QIODevice::ReadOnly))
            return 0;
        while (!buf.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(buf);
            if (data.isEmpty())
                continue;

            ItemPropertyTxt *prop = new ItemPropertyTxt;
            prop->add = data.at(1).toUShort();
            prop->bits = data.at(2).toUShort();
            prop->paramBitsSave = data.at(3).toUShort();
            prop->bitsSave = data.at(4).toUShort();
            QList<QByteArray> groupIDs = data.at(5).split(',');
            if (!groupIDs.at(0).isEmpty())
                foreach (const QByteArray &id, groupIDs)
                    prop->groupIDs += id.toUShort();
            prop->descGroupNegative = QString::fromUtf8(data.at(6));
            prop->descGroupPositive = QString::fromUtf8(data.at(7));
            prop->descGroupStringAdd = QString::fromUtf8(data.at(8));
            prop->descNegative = QString::fromUtf8(data.at(9));
            prop->descPositive = QString::fromUtf8(data.at(10));
            prop->descStringAdd = QString::fromUtf8(data.at(11));
            prop->descFunc = data.at(12).toUShort();
            prop->descPriority = data.at(13).toUShort();
            prop->descVal = data.at(14).toUShort();
            prop->descGroupFunc = data.at(15).toUShort();
            prop->descGroupPriority = data.at(16).toUShort();
            prop->descGroupVal = data.at(17).toUShort();
            prop->paramBits = data.at(18).toUShort();
            prop->stat = data.at(19);
            allProperties[data.at(0).toUInt()] = prop;
        }
    }
    return &allProperties;
}

QList<SetFixedProperty> collectSetProperties(const QList<QByteArray> &data, quint16 firstColumn, quint16 lastColumn = 0)
{
    QList<SetFixedProperty> result;
    for (quint16 i = firstColumn, n = lastColumn ? lastColumn : data.size(); i < n; i += 4)
    {
        SetFixedProperty prop;
        QByteArray propIds = data.at(i);
        if (!propIds.isEmpty())
        {
            foreach (const QByteArray &propId, propIds.split(',')) //-V807
                prop.ids << propId.toUShort();
            prop.param = data.at(i + 1).toInt();
            prop.minValue = data.at(i + 2).toInt();
            prop.maxValue = data.at(i + 3).toInt();
        }
        result << prop;
    }
    return result;
}

QHash<uint, SetItemInfo *> *ItemDataBase::Sets()
{
    static QHash<uint, SetItemInfo *> allSets;
    if (allSets.isEmpty())
    {
        // set items
        QByteArray fileData = decompressedFileData(ResourcePathManager::localizedPathForFileName("setitems"), tr("Set items data not loaded."));
        if (fileData.isEmpty())
            return 0;

        QBuffer buf(&fileData);
        if (!buf.open(QIODevice::ReadOnly))
            return 0;
        while (!buf.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(buf);
            if (data.isEmpty())
                continue;

            SetItemInfo *setItem = new SetItemInfo;
            setItem->itemName = QString::fromUtf8(data.at(1));
            setItem->setName  = QString::fromUtf8(data.at(2));
            setItem->key  = data.at(3);
            setItem->rlvl = data.at(4).toUShort();
            if (data.size() > 5)
            {
                setItem->imageName = data.at(5);
                setItem->fixedProperties = collectSetProperties(data, 6);
            }
            allSets[data.at(0).toUInt()] = setItem;

            _sets[setItem->key].itemNames << setItem->itemName;
        }
        buf.close();

        // full set bonuses
        fileData = decompressedFileData(ResourcePathManager::dataPathForFileName("sets.dat"), tr("Sets data not loaded."));
        if (fileData.isEmpty())
            return 0;

        buf.setBuffer(&fileData);
        if (!buf.open(QIODevice::ReadOnly))
            return 0;
        while (!buf.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(buf);
            if (data.size() < 2)
                continue;

            FullSetInfo &info = _sets[data.at(0)];
            info.partialSetProperties = collectSetProperties(data, 1, 33);
            info.fullSetProperties = collectSetProperties(data, 33);
        }
    }
    return &allSets;
}

QList<SkillInfo *> *ItemDataBase::Skills()
{
    static QList<SkillInfo *> allSkills;
    if (allSkills.isEmpty())
    {
        QByteArray fileData = decompressedFileData(ResourcePathManager::localizedPathForFileName("skills"), tr("Skills data not loaded."));
        if (fileData.isEmpty())
            return 0;

        QBuffer buf(&fileData);
        if (!buf.open(QIODevice::ReadOnly))
            return 0;
        while (!buf.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(buf);
            if (data.isEmpty())
                continue;

            SkillInfo *skill = new SkillInfo;
            skill->name = QString::fromUtf8(data.at(1));
            skill->classCode = data.at(2).toShort();
            if (data.size() > 3)
            {
                skill->tab = data.at(3).toShort();
                skill->row = data.at(4).toShort();
                skill->col = data.at(5).toShort();
                skill->imageId = data.at(6).toShort();
            }
            allSkills.push_back(skill);
        }
    }
    return &allSkills;
}

QHash<uint, UniqueItemInfo *> *ItemDataBase::Uniques()
{
    static QHash<uint, UniqueItemInfo *> allUniques;
    if (allUniques.isEmpty())
    {
        QByteArray fileData = decompressedFileData(ResourcePathManager::localizedPathForFileName("uniques"), tr("Uniques data not loaded."));
        if (fileData.isEmpty())
            return 0;

        QBuffer buf(&fileData);
        if (!buf.open(QIODevice::ReadOnly))
            return 0;
        while (!buf.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(buf);
            if (data.isEmpty())
                continue;

            UniqueItemInfo *uniqueItem = new UniqueItemInfo;
            uniqueItem->name = QString::fromUtf8(data.at(1));
            uniqueItem->rlvl = data.at(2).toUShort();
            if (data.size() > 3)
                uniqueItem->imageName = data.at(3);
            allUniques[data.at(0).toUInt()] = uniqueItem;
        }
    }
    return &allUniques;
}

QHash<uint, MysticOrb *> *ItemDataBase::MysticOrbs()
{
    static QHash<uint, MysticOrb *> allMysticOrbs;
    if (allMysticOrbs.isEmpty())
    {
        QByteArray fileData = decompressedFileData(ResourcePathManager::dataPathForFileName("mo.dat"), tr("Mystic Orbs data not loaded."));
        if (fileData.isEmpty())
            return 0;

        QBuffer buf(&fileData);
        if (!buf.open(QIODevice::ReadOnly))
            return 0;
        while (!buf.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(buf);
            if (data.isEmpty())
                continue;

            MysticOrb *mo = new MysticOrb;
            mo->itemCode = data.at(1);
            foreach (const QByteArray &statId, data.at(2).split(',')) //-V807
                mo->statIds << statId.toUShort();
            mo->value = data.at(3).toShort();
            mo->rlvl = data.size() > 7 ? data.at(6).toUShort() : 2; // before MXL 2017 it has always been 2

            if (data.size() > 4)
            {
                quint16 param = data.at(5).toUShort();
                mo->param = isCtcProperty(mo->statIds.at(0)) ? (data.at(4).toUShort() + (param << 6)) : param;
            }

            allMysticOrbs[data.at(0).toUInt()] = mo;
        }
    }
    return &allMysticOrbs;
}

QHash<uint, QString> *ItemDataBase::Monsters()
{
    static QHash<uint, QString> allMonsters;
    if (allMonsters.isEmpty())
    {
        QByteArray fileData = decompressedFileData(ResourcePathManager::localizedPathForFileName("monsters"), tr("Monster names not loaded."));
        if (fileData.isEmpty())
            return 0;

        QBuffer buf(&fileData);
        if (!buf.open(QIODevice::ReadOnly))
            return 0;
        while (!buf.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(buf);
            if (data.isEmpty())
                continue;

            allMonsters[data.at(0).toUInt()] = data.size() > 1 ? QString::fromUtf8(data.at(1)) : QString();
        }
    }
    return &allMonsters;
}

RunewordHash *ItemDataBase::RW()
{
    static RunewordHash allRunewords;
    if (allRunewords.isEmpty())
    {
        QByteArray fileData = decompressedFileData(ResourcePathManager::localizedPathForFileName("rw"), tr("Runewords data not loaded."));
        if (fileData.isEmpty())
            return 0;

        QBuffer buf(&fileData);
        if (!buf.open(QIODevice::ReadOnly))
            return 0;
        while (!buf.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(buf);
            if (data.isEmpty())
                continue;

            RunewordInfo *rw = new RunewordInfo;
            int column = 0;

            rw->allowedItemTypes.reserve(6);
            for (; column < 6; ++column)
                rw->allowedItemTypes << data.at(column);
            rw->name = QString::fromUtf8(data.at(column++));

            QByteArray key;
            foreach (const QByteArray &rune, data.mid(column))
                key += rune;
            allRunewords.insert(key, rw);
        }
    }
    return &allRunewords;
}

QHash<QByteArray, SocketableItemInfo *> *ItemDataBase::Socketables()
{
    static QHash<QByteArray, SocketableItemInfo *> allSocketables;
    if (allSocketables.isEmpty())
    {
        QByteArray fileData = decompressedFileData(ResourcePathManager::localizedPathForFileName("socketables"), tr("Socketables data not loaded."));
        if (fileData.isEmpty())
            return 0;

        QBuffer buf(&fileData);
        if (!buf.open(QIODevice::ReadOnly))
            return 0;
        while (!buf.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(buf);
            if (data.isEmpty())
                continue;

            SocketableItemInfo *item = new SocketableItemInfo;
            item->name = QString::fromUtf8(data.at(1));
            item->letter = QString::fromUtf8(data.at(2));
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
                item->properties[static_cast<SocketableItemInfo::PropertyType>(i)] = props;
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
        QByteArray fileData = decompressedFileData(ResourcePathManager::localizedPathForFileName("LowQualityItems"), tr("Non-magic qualities data not loaded."));
        if (fileData.isEmpty())
            return 0;

        QBuffer buf(&fileData);
        if (!buf.open(QIODevice::ReadOnly))
            return 0;
        while (!buf.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(buf);
            if (!data.isEmpty())
                allQualities << QString::fromUtf8(data.at(0));
        }
    }
    return &allQualities;
}

QHash<QString, QString> *ItemDataBase::StringTable()
{
    static QHash<QString, QString> strings;
    if (strings.isEmpty())
    {
        QFile f(ResourcePathManager::localizedPathForFileName("tbl"));
        if (!f.open(QIODevice::ReadOnly))
        {
            ERROR_BOX_NO_PARENT(tr("String table not loaded.") + "\n" + tr("Reason: %1").arg(f.errorString()));
            return 0;
        }

        QByteArray fileData = f.readAll();
        QBuffer buf(&fileData);
        if (!buf.open(QIODevice::ReadOnly))
            return 0;
        while (!buf.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(buf);
            if (!data.isEmpty())
                strings[QString::fromUtf8(data.at(0))] = data.size() > 1 ? QString::fromUtf8(data.at(1)) : QString();
        }
    }
    return &strings;
}

QList<QByteArray> ItemDataBase::stringArrayOfCurrentLineInFile(QIODevice &d)
{
    bool isFirstPos = d.pos() == 0;
    QByteArray itemString = d.readLine().trimmed();
    return itemString.isEmpty() || (isFirstPos && itemString.startsWith('#')) ? QList<QByteArray>() : itemString.split('\t');
}

QString ItemDataBase::completeItemName(ItemInfo *item, bool shouldUseColor, bool showQualityText /*= true*/)
{
    QString itemName = Items()->value(item->itemType)->name, nonMagicalQuality;
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
        SetItemInfo *setItem = Sets()->value(item->setOrUniqueId);
        specialName = setItem->itemName;
        if (!shouldUseColor)
            specialName += QString(" [%1]").arg(QString(setItem->setName).replace("\\n", " "));
    }
    else if (item->quality == Enums::ItemQuality::Unique)
        specialName = Uniques()->contains(item->setOrUniqueId) ? Uniques()->value(item->setOrUniqueId)->name : QString();
    else if (item->isRW)
        specialName = item->rwName;

    if (isUberCharm(item))
    {
        // choose the longest name
        if (specialName.length() >= itemName.length())
            itemName = specialName;
        specialName.clear();
    }

    if (!item->inscribedName.isEmpty())
        (specialName.isEmpty() || item->isRW ? itemName : specialName).prepend(tr("%1's ", "personalized name").arg(item->inscribedName.constData()));

    if (shouldUseColor)
    {
        if (item->isRW)
        {
            itemName = htmlStringFromDiabloColorString(itemName, ColorsManager::DarkGrey);
            specialName = htmlStringFromDiabloColorString(specialName, ColorsManager::Gold);
            itemName.prepend(specialName + kHtmlLineBreak);
        }
        else
        {
            bool hasSpecialName = !specialName.isEmpty() && specialName != itemName;
            ColorsManager::ColorIndex colorIndex = colorOfItem(item);
            itemName = htmlStringFromDiabloColorString(itemName, colorIndex);
            if (hasSpecialName)
            {
                specialName = htmlStringFromDiabloColorString(specialName, colorIndex);
                itemName.prepend(specialName + kHtmlLineBreak);
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
                itemName.prepend(QString("[%1]%2").arg(quality, kHtmlLineBreak));
            else if (item->isRW)
                itemName.prepend(QString("[%1]%2").arg(tr("runeword"), kHtmlLineBreak));

            QString spelldesc = Items()->value(item->itemType)->spelldesc;
            if (!spelldesc.isEmpty())
            {
                expandMultilineString(&spelldesc);
                itemName += kHtmlLineBreak + spelldesc;
            }

            removeColorCodesFromString(itemName);

            if (item->isEthereal)
                itemName += QString("%1[%2]").arg(kHtmlLineBreak, tr("ethereal"));
        }
    }
    return itemName;
}

void ItemDataBase::expandMultilineString(QString *stringToExpand)
{
    QStringList lines = stringToExpand->split("\\n");
    std::reverse(lines.begin(), lines.end());
    *stringToExpand = lines.join(kHtmlLineBreak);
}

QHash<int, ColorsManager::ColorIndex> *ItemDataBase::itemQualityColorsHash()
{
    static QHash<int, ColorsManager::ColorIndex> colorsHash;
    if (colorsHash.isEmpty())
    {
        colorsHash[Enums::ItemQuality::Set] = ColorsManager::Green;
        colorsHash[Enums::ItemQuality::Unique] = ColorsManager::Gold;
        colorsHash[Enums::ItemQuality::Crafted] = ColorsManager::Orange;
        colorsHash[Enums::ItemQuality::Rare] = ColorsManager::Yellow;
        colorsHash[Enums::ItemQuality::Magic] = ColorsManager::Blue;
        colorsHash[Enums::ItemQuality::Honorific] = ColorsManager::DarkGreen;
    }
    return &colorsHash;
}

ColorsManager::ColorIndex ItemDataBase::colorOfItem(ItemInfo *item)
{
    return !itemQualityColorsHash()->contains(item->quality) && (item->isSocketed || item->isEthereal) ? ColorsManager::DarkGrey : itemQualityColorsHash()->value(item->quality);
}

QString &ItemDataBase::removeColorCodesFromString(QString &s)
{
    s.remove("\\grey;");
    foreach (const QByteArray &colorString, ColorsManager::colorStrings())
        s.remove(colorString);
    return s;
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

ItemsList ItemDataBase::extractItemsFromPageRange(const ItemsList &items, quint32 firstPage, quint32 lastPage)
{
    ItemsList extractedItems;
    foreach (ItemInfo *item, items)
        if (item->plugyPage >= firstPage && item->plugyPage <= lastPage)
            extractedItems += item;
    return extractedItems;
}

ItemsList ItemDataBase::itemsStoredIn(int storage, int location /*= Enums::ItemLocation::Stored*/, quint32 *pPlugyPage /*= 0*/, ItemsList *allItems /*= 0*/)
{
    ItemsList items, *characterItems = allItems ? allItems : &CharacterInfo::instance().items.character;
    for (int i = 0; i < characterItems->size(); ++i)
    {
        ItemInfo *item = characterItems->at(i);
        if (item->storage == storage && item->location == location && (!pPlugyPage || item->plugyPage == *pPlugyPage))
            items += item;
    }
    return items;
}

bool ItemDataBase::storeItemIn(ItemInfo *item, Enums::ItemStorage::ItemStorageEnum storage, quint8 rowsTotal, quint8 colsTotal, quint32 plugyPage /*= 0*/, bool shouldChangeCoordinatesBits /*= true*/)
{
    return storeItemIn(item, storage, rowsTotal, colsTotal, Enums::ItemLocation::Stored, 0, plugyPage, shouldChangeCoordinatesBits);
}

bool ItemDataBase::storeItemIn(ItemInfo *item, Enums::ItemStorage::ItemStorageEnum storage, quint8 rowsTotal, quint8 colsTotal, Enums::ItemLocation::ItemLocationEnum location, ItemsList *pItems /*= 0*/, quint32 plugyPage /*= 0*/, bool shouldChangeCoordinatesBits /*= true*/)
{
    ItemsList items = pItems ? *pItems : itemsStoredIn(storage, location, plugyPage ? &plugyPage : 0);
    for (quint8 i = 0; i < rowsTotal; ++i)
    {
        for (quint8 j = 0; j < colsTotal; ++j)
        {
            if (canStoreItemAt(i, j, item->itemType, items, rowsTotal, colsTotal))
            {
                item->move(i, j, plugyPage, shouldChangeCoordinatesBits);
                item->storage = storage;
                item->location = location;

                ReverseBitWriter::replaceValueInBitString(item->bitString, Enums::ItemOffsets::Storage, storage > Enums::ItemStorage::Stash ? Enums::ItemStorage::Stash : storage);
                ReverseBitWriter::replaceValueInBitString(item->bitString, Enums::ItemOffsets::Location, location);

                return true;
            }
        }
    }
    return false;
}

bool ItemDataBase::canStoreItemAt(quint8 row, quint8 col, const QByteArray &storeItemType, const ItemsList &items, int rowsTotal, int colsTotal)
{
    // col is horizontal (x), row is vertical (y)
    ItemBase *storeItemBase = ItemDataBase::Items()->value(storeItemType);
    QRect storeItemRect(col, row, storeItemBase->width, storeItemBase->height);
    if (storeItemRect.right() >= colsTotal || storeItemRect.bottom() >= rowsTotal) // beyond grid
        return false;

    foreach (ItemInfo *item, items)
    {
        ItemBase *itemBase = ItemDataBase::Items()->value(item->itemType);
        if (storeItemRect.intersects(QRect(item->column, item->row, itemBase->width, itemBase->height)))
            return false;
    }
    return true;
}

bool ItemDataBase::isClassCharm(const QByteArray &itemType)
{
    foreach (const QByteArray &type, Items()->value(itemType)->types) //-V807
        if (type.startsWith("ara"))
            return true;
    return false;
}

bool ItemDataBase::isClassCharm(ItemInfo *item)
{
    return isClassCharm(item->itemType);
}

bool ItemDataBase::isUberCharm(const QList<QByteArray> &itemBaseTypes)
{
    return ItemParser::itemTypesInheritFromType(itemBaseTypes, "char");
}

bool ItemDataBase::isUberCharm(const QByteArray &itemBaseType)
{
    return isUberCharm(QList<QByteArray>() << itemBaseType);
}

bool ItemDataBase::isUberCharm(ItemInfo *item)
{
    return isUberCharm(Items()->value(item->itemType)->types);
}

bool ItemDataBase::isGenericSocketable(ItemInfo *item)
{
    return Socketables()->contains(item->itemType);
}

bool ItemDataBase::isCube(ItemInfo *item)
{
    return item->itemType == "box";
}

bool ItemDataBase::isTomeWithScrolls(ItemInfo *item)
{
    return item->itemType == "ibk" || item->itemType == "tbk";
}

bool ItemDataBase::doesItemGrantBonus(ItemInfo *item)
{
    // ignore alt. hand
    return (item->location == Enums::ItemLocation::Equipped && item->whereEquipped < 11) || (item->storage == Enums::ItemStorage::Inventory && isUberCharm(item));
}

bool ItemDataBase::canDisenchantIntoArcaneShards(ItemInfo *item)
{
    // prohibit disenchanting items with respective property and quest items into shards
    return canDisenchant(item) && item->quality == Enums::ItemQuality::Unique && ItemParser::itemTypesInheritFromType(Items()->value(item->itemType)->types, "dcht")
            && !(item->props.contains(Enums::ItemProperties::CantDisenchant) || Items()->value(item->itemType)->questId > 0);
}

bool ItemDataBase::canDisenchantIntoSignetOfLearning(ItemInfo *item)
{
    // prohibit disenchanting TUs into signets
    return canDisenchant(item) && (item->quality == Enums::ItemQuality::Set || item->quality == Enums::ItemQuality::Unique) && ItemParser::itemTypesInheritFromType(Items()->value(item->itemType)->types, "ssgl");
}

bool ItemDataBase::canDisenchant(ItemInfo *item)
{
    return item && item->location == Enums::ItemLocation::Stored;
}
