#include "itemdatabase.h"
#include "helpers.h"
#include "colorsmanager.hpp"
#include "resourcepathmanager.hpp"
#include "itemparser.h"
#include "characterinfo.hpp"
#include "reversebitwriter.h"

#include <QFile>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif

#define TOUINT16(byteArray) (static_cast<quint8>(byteArray.at(0)) + (static_cast<quint8>(byteArray.at(1)) << 8))
#define CRC_OF_BYTEARRAY(byteArray) qChecksum(byteArray.constData(), byteArray.length())


QMultiHash<QString, QString> ItemDataBase::_sets;

bool ItemDataBase::createUncompressedTempFile(const QString &compressedFilePath, const QString &errorMessage, QFile *uncompressedFile)
{
    QFile f(compressedFilePath);
    if (!f.open(QIODevice::ReadOnly))
    {
        ERROR_BOX_NO_PARENT(errorMessage + "\n" + tr("Reason: %1").arg(f.errorString()));
        return false;
    }

    QByteArray compressedCrcData = f.read(2), originalCrcData = f.read(2);
    quint16 compressedCrc = TOUINT16(compressedCrcData), originalCrc = TOUINT16(originalCrcData);
    static const QString decompressError(tr("Error decrypting file '%1'"));
    QByteArray compressedDataFile = f.readAll();
    if (CRC_OF_BYTEARRAY(compressedDataFile) != compressedCrc)
    {
        ERROR_BOX_NO_PARENT(decompressError.arg(compressedFilePath));
        return false;
    }

    QByteArray originalFileData = qUncompress(compressedDataFile);
    if (CRC_OF_BYTEARRAY(originalFileData) != originalCrc)
    {
        ERROR_BOX_NO_PARENT(decompressError.arg(compressedFilePath));
        return false;
    }

    uncompressedFile->setFileName(QString("%1/%2_temp.txt").arg(QDir::tempPath(), qApp->applicationName()));
    uncompressedFile->remove();
    if (!uncompressedFile->open(QIODevice::ReadWrite))
    {
        ERROR_BOX_NO_PARENT(decompressError.arg(compressedFilePath));
        return false;
    }

    uncompressedFile->write(originalFileData);
    uncompressedFile->flush();
    uncompressedFile->reset();

    return true;
}

QHash<QByteArray, ItemBase *> *ItemDataBase::Items()
{
    static QHash<QByteArray, ItemBase *> allItems;
    if (allItems.isEmpty())
    {
        QFile f;
        if (!createUncompressedTempFile(ResourcePathManager::localizedPathForFileName("items"), tr("Items data not loaded."), &f))
            return 0;

        while (!f.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(f);
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
            item->imageName = data.at(8);
            item->types = data.at(9).split(',');
            item->socketableType = data.at(10).isEmpty() ? -1 : data.at(10).toShort();
            item->classCode = data.at(11).toShort();
            allItems[data.at(0)] = item;
        }
        f.remove();
    }
    return &allItems;
}

QHash<QByteArray, QList<QByteArray> > *ItemDataBase::ItemTypes()
{
    static QHash<QByteArray, QList<QByteArray> > types;
    if (types.isEmpty())
    {
        QFile f;
        if (!createUncompressedTempFile(ResourcePathManager::dataPathForFileName("itemtypes.dat"), tr("Item types data not loaded."), &f))
            return 0;

        while (!f.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(f);
            if (data.isEmpty())
                continue;

            types[data.at(0)] = data.at(1).split(',');
        }
        f.remove();
    }
    return &types;
}

QHash<uint, ItemPropertyTxt *> *ItemDataBase::Properties()
{
    static QHash<uint, ItemPropertyTxt *> allProperties;
    if (allProperties.isEmpty())
    {
        QFile f;
        if (!createUncompressedTempFile(ResourcePathManager::localizedPathForFileName("props"), tr("Properties data not loaded."), &f))
            return 0;

        while (!f.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(f);
            if (data.isEmpty())
                continue;

            ItemPropertyTxt *item = new ItemPropertyTxt;
            item->add = data.at(1).toUShort();
            item->bits = data.at(2).toUShort();
            item->saveBits = data.at(3).toUShort();
            QList<QByteArray> groupIDs = data.at(4).split(',');
            if (!groupIDs.at(0).isEmpty())
                foreach (const QByteArray &id, groupIDs)
                    item->groupIDs += id.toUShort();
            item->descGroupNegative = QString::fromUtf8(data.at(5));
            item->descGroupPositive = QString::fromUtf8(data.at(6));
            item->descGroupStringAdd = QString::fromUtf8(data.at(7));
            item->descNegative = QString::fromUtf8(data.at(8));
            item->descPositive = QString::fromUtf8(data.at(9));
            item->descStringAdd = QString::fromUtf8(data.at(10));
            item->descFunc = data.at(11).toUShort();
            item->descPriority = data.at(12).toUShort();
            item->descVal = data.at(13).toUShort();
            item->descGroupFunc = data.at(14).toUShort();
            item->descGroupPriority = data.at(15).toUShort();
            item->descGroupVal = data.at(16).toUShort();
            item->saveParamBits = data.at(17).toUShort();
            allProperties[data.at(0).toUInt()] = item;
        }
        f.remove();
    }
    return &allProperties;
}

QHash<uint, SetItemInfo *> *ItemDataBase::Sets()
{
    static QHash<uint, SetItemInfo *> allSets;
    if (allSets.isEmpty())
    {
        QFile f;
        if (!createUncompressedTempFile(ResourcePathManager::localizedPathForFileName("sets"), tr("Sets data not loaded."), &f))
            return 0;

        while (!f.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(f);
            if (data.isEmpty())
                continue;

            SetItemInfo *item = new SetItemInfo;
            item->itemName = QString::fromUtf8(data.at(1));
            item->setName = QString::fromUtf8(data.at(2));
            allSets[data.at(0).toUInt()] = item;

            // do not add duplicate names
            if (_sets.count(item->setName) < 5)
                _sets.insert(item->setName, item->itemName);
        }
        f.remove();
    }
    return &allSets;
}

QList<SkillInfo *> *ItemDataBase::Skills()
{
    static QList<SkillInfo *> allSkills;
    if (allSkills.isEmpty())
    {
        QFile f;
        if (!createUncompressedTempFile(ResourcePathManager::localizedPathForFileName("skills"), tr("Skills data not loaded."), &f))
            return 0;

        while (!f.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(f);
            if (data.isEmpty())
                continue;

            SkillInfo *item = new SkillInfo;
            item->name = QString::fromUtf8(data.at(1));
            item->classCode = data.at(2).toShort();
            item->tab = data.at(3).toShort();
            item->row = data.at(4).toShort();
            item->col = data.at(5).toShort();
            allSkills.push_back(item);
        }
        f.remove();
    }
    return &allSkills;
}

QHash<uint, UniqueItemInfo *> *ItemDataBase::Uniques()
{
    static QHash<uint, UniqueItemInfo *> allUniques;
    if (allUniques.isEmpty())
    {
        QFile f;
        if (!createUncompressedTempFile(ResourcePathManager::localizedPathForFileName("uniques"), tr("Uniques data not loaded."), &f))
            return 0;

        while (!f.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(f);
            if (data.isEmpty())
                continue;

            UniqueItemInfo *item = new UniqueItemInfo;
            item->name = QString::fromUtf8(data.at(1));
            item->rlvl = data.at(2).toUShort();
            if (data.size() > 3)
                item->imageName = data.at(3);
            allUniques[data.at(0).toUInt()] = item;
        }
        f.remove();
    }
    return &allUniques;
}

QHash<uint, MysticOrb *> *ItemDataBase::MysticOrbs()
{
    static QHash<uint, MysticOrb *> allMysticOrbs;
    if (allMysticOrbs.isEmpty())
    {
        QFile f;
        if (!createUncompressedTempFile(ResourcePathManager::dataPathForFileName("mo.dat"), tr("Mystic Orbs data not loaded."), &f))
            return 0;
        
        while (!f.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(f);
            if (data.isEmpty())
                continue;

            MysticOrb *item = new MysticOrb;
            item->itemCode = data.at(1);
            QList<QByteArray> statIds = data.at(2).split(',');
            foreach (const QByteArray &statId, statIds)
                item->statIds << statId.toUShort();
            item->value = data.at(3).toUShort();
            allMysticOrbs[data.at(0).toUInt()] = item;
        }
        f.remove();
    }
    return &allMysticOrbs;
}

QHash<uint, QString> *ItemDataBase::Monsters()
{
    static QHash<uint, QString> allMonsters;
    if (allMonsters.isEmpty())
    {
        QFile f;
        if (!createUncompressedTempFile(ResourcePathManager::localizedPathForFileName("monsters"), tr("Monster names not loaded."), &f))
            return 0;

        while (!f.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(f);
            if (data.isEmpty())
                continue;

            allMonsters[data.at(0).toUInt()] = QString::fromUtf8(data.at(1));
        }
        f.remove();
    }
    return &allMonsters;
}

RunewordHash *ItemDataBase::RW()
{
    static RunewordHash allRunewords;
    if (allRunewords.isEmpty())
    {
        QFile f;
        if (!createUncompressedTempFile(ResourcePathManager::localizedPathForFileName("rw"), tr("Runewords data not loaded."), &f))
            return 0;

        while (!f.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(f);
            if (data.isEmpty())
                continue;

            RunewordInfo *item = new RunewordInfo;
            item->allowedItemTypes.reserve(6);
            for (int i = 0; i < 6; ++i)
                item->allowedItemTypes << data.at(i);
            item->name = QString::fromUtf8(data.at(6));
            allRunewords.insert(qMakePair(data.at(7), data.size() == 9 ? data.at(8) : QByteArray()), item);
        }
        f.remove();
    }
    return &allRunewords;
}

QHash<QByteArray, SocketableItemInfo *> *ItemDataBase::Socketables()
{
    static QHash<QByteArray, SocketableItemInfo *> allSocketables;
    if (allSocketables.isEmpty())
    {
        QFile f;
        if (!createUncompressedTempFile(ResourcePathManager::localizedPathForFileName("socketables"), tr("Socketables data not loaded."), &f))
            return 0;

        while (!f.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(f);
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
        f.remove();
    }
    return &allSocketables;
}

QStringList *ItemDataBase::NonMagicItemQualities()
{
    static QStringList allQualities;
    if (allQualities.isEmpty())
    {
        QFile f;
        if (!createUncompressedTempFile(ResourcePathManager::localizedPathForFileName("LowQualityItems"), tr("Non-magic qualities data not loaded."), &f))
            return 0;

        while (!f.atEnd())
        {
            QList<QByteArray> data = stringArrayOfCurrentLineInFile(f);
            if (!data.isEmpty())
                allQualities << QString::fromUtf8(data.at(0));
        }
        f.remove();
    }
    return &allQualities;
}

QList<QByteArray> ItemDataBase::stringArrayOfCurrentLineInFile(QFile &f)
{
    bool isFirstPos = f.pos() == 0;
    QByteArray itemString = f.readLine().trimmed();
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
            specialName += QString(" [%1]").arg(setItem->setName);
    }
    else if (item->quality == Enums::ItemQuality::Unique)
        specialName = Uniques()->contains(item->setOrUniqueId) ? Uniques()->value(item->setOrUniqueId)->name : QString();
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
                item->plugyPage = plugyPage;

                ReverseBitWriter::replaceValueInBitString(item->bitString, Enums::ItemOffsets::Storage, storage > Enums::ItemStorage::Stash ? Enums::ItemStorage::Stash : storage);
                ReverseBitWriter::updateItemRow(item);
                ReverseBitWriter::updateItemColumn(item);

                return true;
            }

    return false;
}

bool ItemDataBase::canStoreItemAt(quint8 row, quint8 col, const QByteArray &storeItemType, const ItemsList &items, int rowsTotal, int colsTotal /*= 10*/)
{
    // col is horizontal (x), row is vertical (y)
    ItemBase *storeItemBase = ItemDataBase::Items()->value(storeItemType);
    QRect storeItemRect(col, row, storeItemBase->width, storeItemBase->height);
    if (storeItemRect.right() >= colsTotal || storeItemRect.bottom() >= rowsTotal) // beyond grid
        return false;

    bool ok = true;
    foreach (ItemInfo *item, items)
    {
        ItemBase *itemBase = ItemDataBase::Items()->value(item->itemType);
        if (storeItemRect.intersects(QRect(item->column, item->row, itemBase->width, itemBase->height)))
        {
            ok = false;
            break;
        }
    }
    return ok;
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

bool ItemDataBase::isUberCharm(ItemInfo *item)
{
    return item->isEar ? false : ItemParser::itemTypesInheritFromType(Items()->value(item->itemType)->types, "char");
}

bool ItemDataBase::isGenericSocketable(ItemInfo *item)
{
    return Socketables()->contains(item->itemType);
}

bool ItemDataBase::isCube(ItemInfo *item)
{
    return item->itemType == "box";
}

bool ItemDataBase::canDisenchantIntoArcaneShards(ItemInfo *item)
{
    // Ultimative prohibits disenchanting TUs from the Gift Box into shards
    return canDisenchant(item) && !(isUltimative() && item->props.contains(Enums::ItemProperties::ItemDuped));
}

bool ItemDataBase::canDisenchantIntoSignetOfLearning(ItemInfo *item)
{
    // Ultimative prohibits disenchanting TUs into signets
    return canDisenchant(item) && !(isUltimative() && item->quality == Enums::ItemQuality::Unique && Items()->value(item->itemType)->genericType != Enums::ItemTypeGeneric::Misc && isTiered(item));
}

bool ItemDataBase::canDisenchant(ItemInfo *item)
{
    if (item && item->location == Enums::ItemLocation::Stored && (item->quality == Enums::ItemQuality::Set || item->quality == Enums::ItemQuality::Unique))
    {
        static const QList<QByteArray> kDisenchantableItemTypes = QList<QByteArray>() << "weap" << "armo" << "amul" << "ring" << "misl" << "jewl";
        QList<QByteArray> itemTypes = Items()->value(item->itemType)->types;
        // actually, it's possible to use the same condition for Ultimative, but using just one item type is quicker
        return isUltimative() ? ItemParser::itemTypesInheritFromType(itemTypes, "dcht") : ItemParser::itemTypesInheritFromTypes(itemTypes, kDisenchantableItemTypes);
    }
    return false;
}
