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
    if (!allItems.size())
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
            item.width = data.at(2).toUShort();
            item.height = data.at(3).toUShort();
            item.genericType = static_cast<Enums::ItemTypeGeneric::ItemTypeGenericEnum>(data.at(4).toUShort());
            item.isStackable = data.at(5).toUShort();
            item.rlvl = data.at(6).toUShort();
            item.imageName = data.at(7);
            item.typeString = data.at(8);
            item.socketableType = data.at(9).isEmpty() ? -1 : data.at(9).toShort();
            item.classCode = data.at(10).toShort();
            allItems[data.at(0)] = item;
        }
    }
    return &allItems;
}

QHash<QByteArray, QList<QByteArray> > *ItemDataBase::ItemTypes()
{
    static QHash<QByteArray, QList<QByteArray> > types;
    if (!types.size())
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
    if (!allProperties.size())
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
    if (!allSets.size())
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
    if (!allSkills.size())
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
    if (!allUniques.size())
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
    if (!allMysticOrbs.size())
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
    if (!allMonsters.size())
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
    if (!allRunewords.size())
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
    if (!allSocketables.size())
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
    if (!allQualities.size())
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
    if (itemName.startsWith('['))
        itemName.remove(0, 4);
    if (!nonMagicalQuality.isEmpty())
        itemName.prepend(nonMagicalQuality + " ");
    if (!item->inscribedName.isEmpty())
        itemName.prepend(tr("%1's ", "personalized name").arg(item->inscribedName));

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
        specialName = shouldUseColor ? htmlStringFromDiabloColorString(item->rwName, Gold) : item->rwName;

    if (!specialName.isEmpty() && specialName != itemName)
    {
        if (shouldUseColor && item->isRW)
            itemName = htmlStringFromDiabloColorString(itemName, DarkGrey);
        // dirty hack for 'Eternal' RW
        itemName.prepend(QString("%1<br>").arg(specialName.contains("2005-") ? specialName.replace("\\n", htmlLineBreak) : specialName));
    }
    if (itemName.contains("\\n"))
    {
        QStringList list = itemName.split("\\n");
        if (list.at(0).contains(list.at(1)))
            itemName = list.at(0);
        else
        {
            QRegExp rx("(.+)<br>\\1");
            rx.setMinimal(true);
            if (rx.indexIn(list.at(1)) != -1)
            {
                list[1] = rx.cap(1);
                itemName = QString("%1<br>%2").arg(list.at(1), list.at(0));
            }
            else // multiline names
            {
                std::reverse(list.begin(), list.end());
                itemName = list.join(htmlLineBreak);
            }
        }
    }
    if (shouldUseColor)
    {
        if (!item->isRW)
            itemName = htmlStringFromDiabloColorString(itemName, colorOfItem(item));
    }
    else
    {
        foreach (const QByteArray &colorString, colorStrings)
            itemName.remove(colorString);

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
                itemName.prepend(QString("[%1]<br>").arg(quality));
            else if (item->isRW)
                itemName.prepend(QString("[%1]<br>").arg(tr("runeword")));

            if (item->isEthereal)
                itemName += QString("<br>[%1]").arg(tr("ethereal"));
        }
    }
    return itemName;
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
