#include "itemparser.h"
#include "itemdatabase.h"
#include "reversebitreader.h"
#include "helpers.h"
#include "resourcepathmanager.hpp"
#include "itemsviewerdialog.h"

#include <QFile>
#include <QDataStream>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


const QByteArray ItemParser::kItemHeader("JM");
const QByteArray ItemParser::kPlugyPageHeader("ST");

const int ItemParser::kBeltMaxRows = 4;
const int ItemParser::kBeltMaxColumns = 4;
const int ItemParser::kInscribedNameCharacterLength = 7;

const QString &ItemParser::kEnhancedDamageFormat()
{
    static const QString s = tr("+%1% Enhanced Damage");
    return s;
}


QString ItemParser::parseItemsToBuffer(quint16 itemsTotal, QDataStream &inputDataStream, const QByteArray &bytes, const QString &corruptedItemFormat, ItemsList *itemsBuffer, bool isPlugyStash /*= false*/)
{
    QString corruptedItemsString;
    for (quint16 i = 0; i < itemsTotal; ++i)
    {
        if (ItemInfo *item = ItemParser::parseItem(inputDataStream, bytes, isPlugyStash && i == itemsTotal - 1))
        {
            itemsBuffer->append(item);

            if (item->status != ItemInfo::Ok)
                corruptedItemsString += itemStorageAndCoordinatesString(corruptedItemFormat, item) + "\n";
        }
    }
    if (itemsBuffer->size() != itemsTotal)
        qDebug("should be %u items, got %d", itemsTotal, itemsBuffer->size());
    return corruptedItemsString;
}

ItemInfo *ItemParser::parseItem(QDataStream &inputDataStream, const QByteArray &bytes, bool isLastItemOnPlugyPage /*= false*/)
{
    ItemInfo *item = 0;
    ItemInfo::ParsingStatus status;
    int attempt = 0, searchEndOffset = 0;
    // loop tries maximum 2 times to read past JM in case it's a part of item
    do
    {
        inputDataStream.skipRawData(2); // JM
        int itemStartOffset = inputDataStream.device()->pos();
        if (!searchEndOffset)
            searchEndOffset = itemStartOffset;
        int nextItemOffset = bytes.indexOf(kItemHeader, searchEndOffset);
        if (nextItemOffset == -1)
            nextItemOffset = bytes.size();
        else if (isLastItemOnPlugyPage)
            nextItemOffset = bytes.lastIndexOf(kPlugyPageHeader, nextItemOffset);
        int itemSize = nextItemOffset - itemStartOffset;
        if (itemSize <= 0)
            break;

        try
        {
            QString itemBitData;
            itemBitData.reserve(itemSize * 8);
            for (int i = 0; i < itemSize; ++i)
            {
                quint8 aByte;
                inputDataStream >> aByte;
                itemBitData.prepend(binaryStringFromNumber(aByte));
            }
            ReverseBitReader bitReader(itemBitData);

            delete item;
            item = new ItemInfo(bitReader.notReadBits());
            item->isQuest = bitReader.readBool();
            bitReader.skip(3);
            item->isIdentified = bitReader.readBool();
            bitReader.skip(5);
            bitReader.skip(); // is duped
            item->isSocketed = bitReader.readBool();
            bitReader.skip(2);
            bitReader.skip(2); // is illegal equip + unk
            item->isEar = bitReader.readBool();
            item->isStarter = bitReader.readBool();
            bitReader.skip(2);
            bitReader.skip();
            item->isExtended = !bitReader.readBool();
            item->isEthereal = bitReader.readBool();
            bitReader.skip();
            item->isPersonalized = bitReader.readBool();
            bitReader.skip();
            item->isRW = bitReader.readBool();
            bitReader.skip(5);
            bitReader.skip(8); // version - should be 101
            bitReader.skip(2);
            item->location = bitReader.readNumber(3);
            item->whereEquipped = bitReader.readNumber(4);
            item->column = bitReader.readNumber(4);
            item->row = bitReader.readNumber(4);
            if (item->location == Enums::ItemLocation::Belt)
            {
                item->row = item->column / kBeltMaxRows;
                item->column %= kBeltMaxColumns;
            }
            item->storage = bitReader.readNumber(3);

            if (item->isEar)
            {
                item->earInfo.classCode = bitReader.readNumber(3);
                item->earInfo.level = bitReader.readNumber(7);
                for (int i = 0; i < 18; ++i)
                {
                    if (quint8 c = static_cast<quint8>(bitReader.readNumber(7)))
                        item->earInfo.name += c;
                    else
                        break;
                }
                item->earInfo.name = item->earInfo.name.trimmed();

                item->itemType = "ear";
                status = ItemInfo::Ok;
                break;
            }

            for (int i = 0; i < 4; ++i)
                item->itemType += static_cast<quint8>(bitReader.readNumber(8));
            item->itemType = item->itemType.trimmed();

            if (item->isExtended)
            {
                item->socketablesNumber = bitReader.readNumber(3);
                item->guid = bitReader.readNumber(32);
                item->ilvl = bitReader.readNumber(7);
                item->quality = bitReader.readNumber(4);
                if (bitReader.readBool())
                    item->variableGraphicIndex = bitReader.readNumber(3) + 1;
                // entry in the appropriate affix table (where a 0 value indicates none)
                // is used for autoprefix and magic/rare/crafted/probably honorific items
                if (bitReader.readBool()) // autoprefix
                    bitReader.skip(11);

                ItemBase *itemBase = ItemDataBase::Items()->value(item->itemType);
                switch (item->quality)
                {
                case Enums::ItemQuality::Normal:
                    break;
                case Enums::ItemQuality::LowQuality: case Enums::ItemQuality::HighQuality:
                    item->nonMagicType = bitReader.readNumber(3);
                    break;
                case Enums::ItemQuality::Magic:
                    bitReader.skip(22); // prefix & suffix
                    break;
                case Enums::ItemQuality::Set: case Enums::ItemQuality::Unique:
                    item->setOrUniqueId = bitReader.readNumber(12);
                    break;
                case Enums::ItemQuality::Rare: case Enums::ItemQuality::Crafted:
                    bitReader.skip(16); // first & second names
                    for (int i = 0; i < 6; ++i)
                        if (bitReader.readBool())
                            bitReader.skip(11); // prefix or suffix (1-3)
                    break;
                case Enums::ItemQuality::Honorific:
                    bitReader.skip(16); // no idea what these bits mean
                    break;
                default:
                    qDebug("Item '%s' of unknown quality %d found!", itemBase->name.toUtf8().constData(), item->quality);
                    break;
                }

                if (item->isRW)
                    bitReader.skip(16); // RW code - don't know how to use it

                item->inscribedNameOffset = bitReader.pos();
                if (item->isPersonalized)
                {
                    for (int i = 0; i < 16; ++i)
                    {
                        quint8 c = static_cast<quint8>(bitReader.readNumber(kInscribedNameCharacterLength));
                        if (!c)
                            break;
                        item->inscribedName += c;
                    }
                }

                bitReader.skip(); // tome of ID bit
                if (ItemDataBase::isTomeWithScrolls(item))
                    bitReader.skip(5); // some tome bits

                if (itemBase->genericType == Enums::ItemTypeGeneric::Armor)
                {
                    ItemPropertyTxt *defenceProp = ItemDataBase::Properties()->value(Enums::ItemProperties::Defence);
                    item->defense = bitReader.readNumber(defenceProp->bits) - defenceProp->add;
                }
                if (itemBase->genericType != Enums::ItemTypeGeneric::Misc)
                {
                    ItemPropertyTxt *maxDurabilityProp = ItemDataBase::Properties()->value(Enums::ItemProperties::DurabilityMax);
                    item->maxDurability = bitReader.readNumber(maxDurabilityProp->bits) - maxDurabilityProp->add;
                    if (item->maxDurability)
                    {
                        ItemPropertyTxt *durabilityProp = ItemDataBase::Properties()->value(Enums::ItemProperties::Durability);
                        item->currentDurability = bitReader.readNumber(durabilityProp->bits) - durabilityProp->add;
                        if (item->maxDurability < item->currentDurability)
                            item->maxDurability = item->currentDurability;
                    }
                }

                item->quantity = itemBase->isStackable ? bitReader.readNumber(9) : -1;
                if (item->isSocketed)
                    item->socketsNumber = bitReader.readNumber(4);

                const int kSetListsNumber = 5;
                bool hasSetLists[kSetListsNumber] = {false};
                if (item->quality == Enums::ItemQuality::Set)
                    for (int i = 0; i < kSetListsNumber; ++i)
                        hasSetLists[i] = bitReader.readBool();

                item->props = parseItemProperties(bitReader, &status);
                if (status != ItemInfo::Ok)
                {
                    inputDataStream.device()->seek(itemStartOffset - 2); // set to JM - beginning of the item
                    item->props.insert(0, new ItemProperty(tr("Error parsing item properties (status == failed), please report!")));
                    searchEndOffset = nextItemOffset + 1;
                    continue;
                }

                PropertiesMultiMap::iterator blessPropIter = item->props.find(Enums::ItemProperties::Trophy_ShrineBless_ClassCharmVeterans); // impossible to put inside the condition
                if (blessPropIter != item->props.end())
                {
                    QString newDesc = ItemDataBase::isUberCharm(item) ? (ItemDataBase::isClassCharm(item) ? tr("Veterans") : tr("Trophy'd"))
                                                                      : tr("Blessed");
                    blessPropIter.value()->displayString = QString("[%1]").arg(newDesc);
                }

                if (item->quality == Enums::ItemQuality::Set)
                    for (int i = 0; i < kSetListsNumber; ++i)
                        if (hasSetLists[i])
                            /*item->setProps = */parseItemProperties(bitReader, &status);

                if (item->isRW)
                {
                    item->rwProps = parseItemProperties(bitReader, &status);
                    if (status != ItemInfo::Ok)
                    {
                        inputDataStream.device()->seek(itemStartOffset - 2); // set to JM - beginning of the item
                        item->rwProps.insert(1, new ItemProperty(tr("Error parsing RW properties (status == failed), please report!")));
                        searchEndOffset = nextItemOffset + 1;
                        continue;
                    }
                }

                // parse all socketables
                QList<QByteArray> rwSocketableTypes;
                for (int i = 0; i < item->socketablesNumber; ++i)
                {
                    ItemInfo *socketableInfo = parseItem(inputDataStream, bytes);
                    item->socketablesInfo += socketableInfo;
                    if (item->isRW && i >= item->socketablesNumber - 2) // get the last socket filler to obtain RW name (and previous one to prevent disambiguation)
                        rwSocketableTypes.prepend(socketableInfo->itemType == ItemDataBase::kJewelType && i != item->socketablesNumber - 1 ? QByteArray() : socketableInfo->itemType);
                }

                if (!rwSocketableTypes.isEmpty())
                {
                    RunewordKeyPair rwKey = qMakePair(rwSocketableTypes.at(0), rwSocketableTypes.size() > 1 ? rwSocketableTypes.at(1) : QByteArray());
                    if (rwKey == qMakePair(QByteArray("r56"), QByteArray("r55"))) // maybe it's 'Eternal'?
                        item->rwName = ItemDataBase::RW()->value(qMakePair(QByteArray("r51"), QByteArray("r52")))->name;
                    else
                    {
                        const RunewordHash *const rwHash = ItemDataBase::RW();
                        RunewordHash::const_iterator iter = rwHash->find(rwKey);
                        for (; iter != rwHash->end() && iter.key() == rwKey; ++iter)
                        {
                            RunewordInfo *rwInfo = iter.value();
                            if (itemTypesInheritFromTypes(itemBase->types, rwInfo->allowedItemTypes))
                            {
                                item->rwName = rwInfo->name;
                                break;
                            }
                        }
                        if (iter == rwHash->end())
                            item->rwName = tr("Unknown RW, please report!");
                    }
                }
                else if (item->isRW)
                    item->rwName = tr("Unknown RW (no socketables detected), please report!");
            }
            else
                item->quality = Enums::ItemQuality::Normal;

            if (item->whereEquipped > 12)
                throw 4;
            else if (item->storage > Enums::ItemStorage::Stash)
                throw 5;

            status = ItemInfo::Ok;
        }
        catch (int exceptionCode)
        {
            qDebug("caught exception %d while parsing item (%d - %d)", exceptionCode, itemStartOffset, nextItemOffset);
            status = ItemInfo::Corrupted;
            inputDataStream.device()->seek(itemStartOffset - 2); // set to JM - beginning of the item //-V807
            searchEndOffset = nextItemOffset + 1;
            continue;
        }
    } while (status != ItemInfo::Ok && ++attempt < 2);

    if (!item || (item->status = status) != ItemInfo::Ok)
    {
        qDebug("current offset %lld", inputDataStream.device()->pos());
        inputDataStream.device()->seek(searchEndOffset - 1);
        qDebug("new offset %lld", inputDataStream.device()->pos());
    }
    return item;
}

PropertiesMultiMap ItemParser::parseItemProperties(ReverseBitReader &bitReader, ItemInfo::ParsingStatus *status)
{
    using namespace Enums;

    PropertiesMultiMap props;
    while (bitReader.pos() != -1)
    {
        ItemProperty *propToAdd = 0;
        try
        {
            int id = bitReader.readNumber(CharacterStats::StatCodeLength);
            if (id == ItemProperties::End)
            {
                *status = ItemInfo::Ok;
                return props;
            }

            propToAdd = new ItemProperty;
            propToAdd->bitStringOffset = bitReader.pos() + 16; // include 'JM' bit length

            ItemPropertyTxt *txtProperty = ItemDataBase::Properties()->value(id);
            propToAdd->param = txtProperty->saveParamBits ? bitReader.readNumber(txtProperty->saveParamBits) : 0;
            propToAdd->value = bitReader.readNumber(txtProperty->bits) - txtProperty->add;
            if (id == ItemProperties::EnhancedDamage)
            {
                qint16 minEnhDamage = bitReader.readNumber(txtProperty->bits) - txtProperty->add;
                if (minEnhDamage < propToAdd->value) // it shouldn't possible (they must always be equal), but let's make sure
                    propToAdd->value = minEnhDamage;
                propToAdd->displayString = kEnhancedDamageFormat().arg(propToAdd->value);
            }

            // elemental damage
            if (   id == ItemProperties::MinimumDamageFire || id == ItemProperties::MinimumDamageLightning || id == ItemProperties::MinimumDamageMagic
                || id == ItemProperties::MinimumDamageCold || id == ItemProperties::MinimumDamagePoison)
            {
                bool hasLength = false;
                if (id == ItemProperties::MinimumDamageCold || id == ItemProperties::MinimumDamagePoison)
                    hasLength = true; // length is present only when min damage is specified
                else if (id == ItemProperties::MinimumDamageMagic)
                    convertParamsInMagicDamageString(propToAdd, txtProperty);
                props.insert(id++, propToAdd);

                // get max elemental damage
                ItemProperty *maxElementalDamageProp = new ItemProperty;
                maxElementalDamageProp->bitStringOffset = bitReader.pos() + 16; // include 'JM' bit length

                ItemPropertyTxt *txtMaxElementalDamageProp = ItemDataBase::Properties()->value(id);
                maxElementalDamageProp->value = bitReader.readNumber(txtMaxElementalDamageProp->bits) - txtMaxElementalDamageProp->add;

                if (id == ItemProperties::MaximumDamageMagic)
                    convertParamsInMagicDamageString(maxElementalDamageProp, txtMaxElementalDamageProp);
                props.insert(id, maxElementalDamageProp);

                if (hasLength) // cold or poison length
                {
                    ItemPropertyTxt *lengthProp = ItemDataBase::Properties()->value(++id);
                    qint16 length = bitReader.readNumber(lengthProp->bits) - lengthProp->add;
                    //propToAdd->displayString = QString(" with length of %1 frames (%2 second(s))").arg(length).arg(static_cast<double>(length) / 25.0, 1);
                    //propToAdd->value = length;
                    //props[id] = propToAdd;

                    if (id == ItemProperties::DurationPoison)
                    {
                        // set correct min/max poison damage
                        props.replace(id - 1, new ItemProperty(qRound(maxElementalDamageProp->value * length / 256.0), length));
                        props.replace(id - 2, new ItemProperty(qRound(             propToAdd->value * length / 256.0), length));

                        delete maxElementalDamageProp;
                        delete propToAdd;
                    }
                }

                continue;
            }

            createDisplayStringForPropertyWithId(id, propToAdd);
            props.insert(id, propToAdd);
        }
        catch (int exceptionCode)
        {
            delete propToAdd;
            qDebug("caught exception while parsing item properties: %d", exceptionCode);
            *status = ItemInfo::Corrupted;
            // Requirements txtProperty has the lowest descpriority, so it'll appear at the bottom
            props.insert(ItemProperties::Requirements, new ItemProperty(tr("Error parsing item properties (exception == %1), please report!").arg(exceptionCode)));
            return props;
        }
    }

    *status = ItemInfo::Failed;
    return PropertiesMultiMap();
}

void ItemParser::convertParamsInMagicDamageString(ItemProperty *prop, ItemPropertyTxt *txtProp)
{
    QString desc = txtProp->descPositive;
    prop->displayString = desc.replace("%d", "%1").arg(prop->value);
}

void ItemParser::createDisplayStringForPropertyWithId(int id, ItemProperty *prop)
{
#ifdef Q_CC_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-compare"
#endif
    if (id == Enums::ItemProperties::ClassSkills)
        prop->displayString = tr("+%1 to %2 Skill Levels", "+x to class skills").arg(prop->value).arg(prop->param < Enums::ClassName::classes().size() ? Enums::ClassName::classes().at(prop->param) : "WTF");
    else if (id == Enums::ItemProperties::Oskill || id == Enums::ItemProperties::ClassOnlySkill)
    {
        SkillInfo *skill;
        bool deleteSkill = prop->param >= ItemDataBase::Skills()->size();
#ifdef Q_CC_CLANG
#pragma clang diagnostic pop
#endif
        if (!deleteSkill)
            skill = ItemDataBase::Skills()->at(prop->param);
        else
        {
            skill = new SkillInfo;
            skill->name = "REALM-ONLY SKILL";
            skill->classCode = -1;
        }
        prop->displayString = tr("+%1 to %2", "oskill").arg(prop->value).arg(skill->name);

        if (id == Enums::ItemProperties::ClassOnlySkill)
            prop->displayString += " " + tr("(%1 Only)", "class-specific skill").arg(skill->classCode > -1 ? Enums::ClassName::classes().at(skill->classCode) : "TROLOLOL");

        if (deleteSkill)
            delete skill;
    }
    else if (id == Enums::ItemProperties::ChargedSkill)
        prop->displayString = tr("Level %1 %2 (%3/%4 Charges)").arg(prop->param & 63).arg(ItemDataBase::Skills()->value(prop->param >> 6)->name).arg(prop->value & 255).arg(prop->value >> 8);
    else if (id == Enums::ItemProperties::ItemDuped)
        prop->displayString = QString("[%1]").arg(tr("duped flag"));
    else if (ItemDataBase::isCtcProperty(id))
    {
        QString desc = ItemDataBase::Properties()->value(id)->descPositive;
        for (int i = 0, k = 1; k <= 3 && i < desc.length(); ++i)
            if (desc.at(i) == '%' && desc.at(i + 1).isLetter())
                desc[++i] = QString::number(k++).at(0);
        prop->displayString = desc.replace("%%", "%").arg(prop->value).arg(prop->param & 63).arg(ItemDataBase::Skills()->value(prop->param >> 6)->name);
    }
    else if (ItemDataBase::MysticOrbs()->contains(id))
        prop->displayString = QString("%1 x '%2'").arg(prop->value).arg(mysticOrbReadableProperty(ItemDataBase::Items()->value(ItemDataBase::MysticOrbs()->value(id)->itemCode)->spelldesc));
}


void ItemParser::writeItems(const ItemsList &items, QDataStream &ds)
{
    foreach (ItemInfo *item, items)
    {
        writeByteArrayDataWithoutNull(ds, kItemHeader);

        QByteArray itemBytes;
        for (int i = 0, n = item->bitString.length(); i < n; i += 8)
            itemBytes.prepend(item->bitString.mid(i, 8).toShort(0, 2));
        writeByteArrayDataWithoutNull(ds, itemBytes);

        writeItems(item->socketablesInfo, ds);

        item->hasChanged = false;
    }
}

QString ItemParser::itemStorageAndCoordinatesString(const QString &text, ItemInfo *item)
{
    return text.arg(ItemsViewerDialog::tabNameAtIndex(ItemsViewerDialog::tabIndexFromItemStorage(item->storage))).arg(item->row + 1).arg(item->column + 1).arg(item->plugyPage ? item->plugyPage : item->whereEquipped);
}

bool ItemParser::itemTypesInheritFromType(const QList<QByteArray> &itemTypes, const QByteArray &allowedItemType)
{
    return itemTypesInheritFromTypes(itemTypes, QList<QByteArray>() << allowedItemType);
}

bool ItemParser::itemTypeInheritsFromTypes(const QByteArray &itemType, const QList<QByteArray> &allowedItemTypes)
{
    return itemTypesInheritFromTypes(QList<QByteArray>() << itemType, allowedItemTypes);
}

bool ItemParser::itemTypesInheritFromTypes(const QList<QByteArray> &itemTypes, const QList<QByteArray> &allowedItemTypes)
{
    foreach (const QByteArray &itemType, itemTypes)
        if (allowedItemTypes.contains(itemType) || itemTypesInheritFromTypes(ItemDataBase::ItemTypes()->value(itemType).baseItemTypes, allowedItemTypes))
            return true;
    return false;
}

QString ItemParser::mysticOrbReadableProperty(const QString &fullDescription)
{
    int start = fullDescription.indexOf("\\n") + 2, end = fullDescription.indexOf("\\n", start);
    return fullDescription.mid(start, end - start);
}
