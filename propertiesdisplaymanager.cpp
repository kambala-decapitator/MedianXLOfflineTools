#include "propertiesdisplaymanager.h"
#include "itemdatabase.h"
#include "itemparser.h"
#include "characterinfo.hpp"

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif

//#define SHOW_PROPERTY_ID


const QList<QByteArray> PropertiesDisplayManager::kDamageToUndeadTypes = QList<QByteArray>() << "mace" << "hamm" << "staf" << "scep" << "club" << "wand";

QString PropertiesDisplayManager::completeItemDescription(ItemInfo *item, bool useColor /*= false*/)
{
    QString ilvlText = tr("Item Level: %1").arg(item->ilvl) + "\n";
    if (item->isEar)
    {
        QString itemDescription = tr("%1's Ear", "param is character name").arg(item->earInfo.name.constData()) + "\n";
        itemDescription += Enums::ClassName::classes().at(item->earInfo.classCode) + "\n";
        itemDescription += tr("Level %1").arg(item->earInfo.level);
        return itemDescription + "\n" + ilvlText;
    }

    bool isClassCharm = ItemDataBase::isClassCharm(item);

    PropertiesMultiMap allProps;
    PropertiesMultiMap::const_iterator constIter = item->props.constBegin();
    while (constIter != item->props.constEnd())
    {
        allProps.insertMulti(constIter.key(), new ItemProperty(*constIter.value())); // original values mustn't be modified
        ++constIter;
    }

    if (item->isRW)
        addProperties(&allProps, item->rwProps);

    PropertiesMultiMap::iterator iter = allProps.begin();
    while (iter != allProps.end())
    {
        if (ItemDataBase::MysticOrbs()->contains(iter.key()) && isClassCharm)
            addChallengeNamesToClassCharm(iter);
        ++iter;
    }

    ItemBase *itemBase = ItemDataBase::Items()->value(item->itemType);
    foreach (ItemInfo *socketableItem, item->socketablesInfo)
    {
        PropertiesMultiMap socketableProps = socketableProperties(socketableItem, itemBase->socketableType);
        addProperties(&allProps, socketableProps);
        if (socketableProps != socketableItem->props)
            qDeleteAll(socketableProps);
    }

    // create full item description
    QString itemDescription = ItemDataBase::completeItemName(item, useColor) + "\n";
    if (useColor)
        itemDescription += ColorsManager::colorStrings().at(ColorsManager::White);
    else
        itemDescription.replace(kHtmlLineBreak, "\n");
    itemDescription += ilvlText;
    if (!itemBase->spelldesc.isEmpty())
        itemDescription += itemBase->spelldesc + "\n";

    QString runes;
    foreach (ItemInfo *socketable, item->socketablesInfo)
        if (ItemParser::itemTypesInheritFromType(ItemDataBase::Items()->value(socketable->itemType)->types, "rune"))
            runes += ItemDataBase::Socketables()->value(socketable->itemType)->letter;
    if (!runes.isEmpty()) // gem-/jewelwords don't have any letters
        itemDescription += QString("%1'%2'\n").arg(useColor ? ColorsManager::colorStrings().at(ColorsManager::Gold) : QString(), runes);

    quint8 clvl = CharacterInfo::instance().basicInfo.level;
    ItemProperty *foo = new ItemProperty;
    if (itemBase->genericType == Enums::ItemTypeGeneric::Armor)
    {
        int baseDef = item->defense, totalDef = baseDef;
        if (int ed = allProps.value(Enums::ItemProperties::EnhancedDefence, foo)->value + (allProps.value(Enums::ItemProperties::EnhancedDefenceBasedOnClvl, foo)->value * clvl) / 32)
            totalDef = (totalDef * (100 + ed)) / 100;
        totalDef += allProps.value(Enums::ItemProperties::Defence, foo)->value + (allProps.value(Enums::ItemProperties::DefenceBasedOnClvl, foo)->value * clvl) / 32;
        if (totalDef < 0)
            totalDef = 0;

        QString defString = "\n" + tr("Defense: %1");
        if (baseDef != totalDef)
            itemDescription += QString("%1 (%2)").arg(defString.arg(QString::number(totalDef))).arg(baseDef);
        else
            itemDescription += defString.arg(baseDef);
    }
    else if (itemBase->genericType == Enums::ItemTypeGeneric::Weapon)
    {
        // TODO: [later] use lambda to calculate damage
        int ed = allProps.value(Enums::ItemProperties::EnhancedDamage, foo)->value;
        int minDmgTotal = allProps.value(Enums::ItemProperties::MinimumDamage, foo)->value + allProps.value(Enums::ItemProperties::MinimumDamageSecondary, foo)->value;
        int maxDmgTotal = allProps.value(Enums::ItemProperties::MaximumDamage, foo)->value + allProps.value(Enums::ItemProperties::MaximumDamageSecondary, foo)->value +
                         (allProps.value(Enums::ItemProperties::MaximumDamageBasedOnClvl, foo)->value * clvl) / 32;
        QString damageFormat = tr("%1 to %2", "min-max damage");

        if (itemBase->minThrowDmg && itemBase->maxThrowDmg)
        {
            int minDmg = itemBase->minThrowDmg, maxDmg = itemBase->maxThrowDmg;
            if (item->isEthereal)
            {
                minDmg *= ItemDataBase::EtherealMultiplier;
                maxDmg *= ItemDataBase::EtherealMultiplier;
            }
            if (ed)
            {
                minDmg = (minDmg * (100 + ed)) / 100;
                maxDmg = (maxDmg * (100 + ed)) / 100;
            }
            minDmg += minDmgTotal;
            maxDmg += maxDmgTotal;

            itemDescription += QString("%1: %2\n").arg(tr("Throw Damage"), damageFormat.arg(minDmg).arg(maxDmg));
        }

        if (itemBase->min1hDmg && itemBase->max1hDmg)
        {
            int minDmg = itemBase->min1hDmg, maxDmg = itemBase->max1hDmg;
            if (item->isEthereal)
            {
                minDmg *= ItemDataBase::EtherealMultiplier;
                maxDmg *= ItemDataBase::EtherealMultiplier;
            }
            if (ed)
            {
                minDmg = (minDmg * (100 + ed)) / 100;
                maxDmg = (maxDmg * (100 + ed)) / 100;
            }
            minDmg += minDmgTotal;
            maxDmg += maxDmgTotal;

            itemDescription += QString("%1: %2\n").arg(tr("One-Hand Damage"), damageFormat.arg(minDmg).arg(maxDmg));
        }

        if (itemBase->min2hDmg && itemBase->max2hDmg)
        {
            int minDmg = itemBase->min2hDmg, maxDmg = itemBase->max2hDmg;
            if (item->isEthereal)
            {
                minDmg *= ItemDataBase::EtherealMultiplier;
                maxDmg *= ItemDataBase::EtherealMultiplier;
            }
            if (ed)
            {
                minDmg = (minDmg * (100 + ed)) / 100;
                maxDmg = (maxDmg * (100 + ed)) / 100;
            }
            minDmg += minDmgTotal;
            maxDmg += maxDmgTotal;

            itemDescription += QString("%1: %2\n").arg(tr("Two-Hand Damage"), damageFormat.arg(minDmg).arg(maxDmg));
        }
    }
    if (itemBase->hasDurability && item->maxDurability)
    {
        itemDescription += QString("\n%1: ").arg(tr("Durability"));
        bool isIndestructible = allProps.value(Enums::ItemProperties::Indestructible, foo)->value == 1;
        if (isIndestructible)
            itemDescription += QString("%1 [").arg(QChar(0x221e)); // infinity
        itemDescription += tr("%1 of %2", "durability").arg(item->currentDurability).arg(item->maxDurability);
        if (isIndestructible)
            itemDescription += "]";
    }
    if (itemBase->isStackable)
        itemDescription += "\n" + tr("Quantity: %1").arg(item->quantity);

    ItemProperty *classRestrictionProp = allProps.value(Enums::ItemProperties::ClassRestriction);
    qint8 classCode = classRestrictionProp ? (classRestrictionProp->value - 1) : itemBase->classCode;
    if (classCode > -1)
        itemDescription += "\n" + tr("(%1 Only)", "class-specific item").arg(Enums::ClassName::classes().at(classCode));

    // TODO: [later] use lambda to calculate requirements
    if (itemBase->rdex)
        if (int rdex = itemBase->rdex + (itemBase->rdex * allProps.value(Enums::ItemProperties::Requirements, foo)->value) / 100)
            itemDescription += "\n" + tr("Required Dexterity: %1").arg(rdex);

    if (itemBase->rstr)
        if (int rstr = itemBase->rstr + (itemBase->rstr * allProps.value(Enums::ItemProperties::Requirements, foo)->value) / 100)
            itemDescription += "\n" + tr("Required Strength: %1").arg(rstr);

    int rlvl;
    switch (item->quality)
    {
    case Enums::ItemQuality::Set: case Enums::ItemQuality::Unique:
    {
        SetOrUniqueItemInfo *info;
        if (item->quality == Enums::ItemQuality::Set)
            info = ItemDataBase::Sets()->value(item->setOrUniqueId);
        else
            info = ItemDataBase::Uniques()->value(item->setOrUniqueId);
        rlvl = info ? info->rlvl : itemBase->rlvl;
    }
        break;
    default:
        rlvl = itemBase->rlvl;
        break;
    }
    int maxSocketableRlvl = 0;
    foreach (ItemInfo *socketableItem, item->socketablesInfo)
    {
        int socketableRlvl = socketableItem->quality == Enums::ItemQuality::Unique ? ItemDataBase::Uniques()->value(socketableItem->setOrUniqueId)->rlvl : ItemDataBase::Items()->value(socketableItem->itemType)->rlvl;
        if (maxSocketableRlvl < socketableRlvl)
            maxSocketableRlvl = socketableRlvl;
    }
    if (int actualRlvl = qMax(rlvl, maxSocketableRlvl) + allProps.value(Enums::ItemProperties::RequiredLevel, foo)->value)
        itemDescription += "\n" + tr("Required Level: %1").arg(actualRlvl);
    delete foo;

    foreach (const QString &bonus, PropertiesDisplayManager::weaponDamageBonuses(itemBase))
        itemDescription += "\n" + bonus;

    // add '+50% damage to undead' if item type matches
    bool shouldAddDamageToUndeadInTheBottom = false;
    if (ItemParser::itemTypesInheritFromTypes(itemBase->types, kDamageToUndeadTypes))
    {
        if (allProps.contains(Enums::ItemProperties::DamageToUndead))
            allProps.value(Enums::ItemProperties::DamageToUndead)->value += 50;
        else
            shouldAddDamageToUndeadInTheBottom = true;
    }

    if (useColor)
        itemDescription += QString("\n-PROPS-") + ColorsManager::colorStrings().at(ColorsManager::Blue).constData();

    if (!allProps.isEmpty())
    {
        if (!item->isIdentified)
            itemDescription += "\n" + tr("[Unidentified]");
        itemDescription += propsToString(allProps);
    }
    else if (ItemDataBase::isGenericSocketable(item))
    {
        static const QStringList gearNames = QStringList() << tr("Armor") << tr("Shield") << tr("Weapon");
        QStringList propStrings;
        for (qint8 socketableType = SocketableItemInfo::Armor; socketableType <= SocketableItemInfo::Weapon; ++socketableType)
        {
            PropertiesMultiMap props = PropertiesDisplayManager::genericSocketableProperties(item, socketableType - 1);
            QMap<quint8, ItemPropertyDisplay> propsDisplayMap;
            constructPropertyStrings(props, &propsDisplayMap);

            QString propText;
            QMap<quint8, ItemPropertyDisplay>::const_iterator iter = propsDisplayMap.constEnd();
            while (iter != propsDisplayMap.constBegin())
            {
                --iter;
                propText += iter.value().displayString + ", ";
            }
            propText.remove(propText.length() - 2, 2); // remove last ", "

            propStrings += QString("\n%1: %2").arg(gearNames.at(socketableType), propText);
        }
        // weapon properties should be first
        propStrings.move(propStrings.size() - 1, 0);
        itemDescription += "\n" + propStrings.join(QString());
    }

    if (shouldAddDamageToUndeadInTheBottom)
        itemDescription += "\n" + tr("+50% Damage to Undead");
    if (item->isSocketed)
        itemDescription += "\n" + tr("Socketed: (%1), Inserted: (%2)").arg(item->socketsNumber).arg(item->socketablesNumber);
    if (item->isEthereal)
        itemDescription += "\n" + tr("Ethereal (Cannot be Repaired)");

    if (item->quality == Enums::ItemQuality::Set)
    {
        SetItemInfo *setItem = ItemDataBase::Sets()->value(item->setOrUniqueId);
        const FullSetInfo fullSetInfo = ItemDataBase::fullSetInfoForKey(setItem->key);

        if (item->location == Enums::ItemLocation::Equipped || item->location == Enums::ItemLocation::Merc)
        {
            // set item properties stored in item (seems that they're not needed)
            //if (!item->setProps.isEmpty())
            //    itemDescription += propertiesToHtml(item->setProps, ColorsManager::Green);

            // count equipped set items
            quint8 setItemsOnCharacter = 1;
            foreach (ItemInfo *anItem, ItemDataBase::itemsStoredIn(item->storage, item->location))
                if (anItem != item && anItem->quality == Enums::ItemQuality::Set && fullSetInfo.itemNames.contains(ItemDataBase::Sets()->value(anItem->setOrUniqueId)->itemName))
                    ++setItemsOnCharacter;

            if (quint8 partialPropsNumber = (setItemsOnCharacter - 1) * 2)
            {
                // set item properties from txt
                PropertiesMultiMap setItemFixedProps = PropertiesDisplayManager::collectSetFixedProps(setItem->fixedProperties, partialPropsNumber);
                if (!setItemFixedProps.isEmpty())
                {
                    QString s = propsToString(setItemFixedProps);
                    if (!s.isEmpty())
                        itemDescription += "\n-SET_ITEM-" + s;
                }
                qDeleteAll(setItemFixedProps);

                // set properties from txt
                PropertiesMultiMap setFixedProps = PropertiesDisplayManager::collectSetFixedProps(fullSetInfo.partialSetProperties, partialPropsNumber);
                if (setItemsOnCharacter == fullSetInfo.itemNames.size())
                    PropertiesDisplayManager::addTemporaryPropertiesAndDelete(&setFixedProps, PropertiesDisplayManager::collectSetFixedProps(fullSetInfo.fullSetProperties));

                QString s = propsToString(setFixedProps);
                if (!s.isEmpty())
                    itemDescription += "\n-SET-" + s;

                qDeleteAll(setFixedProps);
            }
        }
    }

    qDeleteAll(allProps);
    return itemDescription;
}

void PropertiesDisplayManager::addProperties(PropertiesMap *mutableProps, const PropertiesMap &propsToAdd, const QSet<int> *pIgnorePropIds /*= 0*/)
{
    for (PropertiesMap::const_iterator iter = propsToAdd.constBegin(); iter != propsToAdd.constEnd(); ++iter)
    {
        int propId = iter.key();
        if (pIgnorePropIds && pIgnorePropIds->contains(propId))
            continue;

        ItemProperty *prop = iter.value();
        bool shouldAddNewProp = true;
        foreach (ItemProperty *existingProp, mutableProps->values(iter.key()))
        {
            if (existingProp->param == prop->param)
            {
                existingProp->value += prop->value;
                if (propId == Enums::ItemProperties::EnhancedDamage)
                    existingProp->displayString = ItemParser::kEnhancedDamageFormat().arg(existingProp->value);
                else
                    ItemParser::createDisplayStringForPropertyWithId(propId, existingProp);

                shouldAddNewProp = false;
                break;
            }
        }
        if (shouldAddNewProp)
            mutableProps->insertMulti(propId, new ItemProperty(*prop)); // we need a copy
    }
}

void PropertiesDisplayManager::addTemporaryPropertiesAndDelete(PropertiesMap *mutableProps, const PropertiesMap &tempPropsToAdd, const QSet<int> *pIgnorePropIds /*= 0*/)
{
    addProperties(mutableProps, tempPropsToAdd, pIgnorePropIds);
    qDeleteAll(tempPropsToAdd);
}

void PropertiesDisplayManager::constructPropertyStrings(const PropertiesMultiMap &properties, QMap<quint8, ItemPropertyDisplay> *outDisplayPropertiesMultiMap, bool shouldColor /*= false*/, ItemInfo *item /*= 0*/)
{
    using namespace Enums;

    QMap<quint8, ItemPropertyDisplay> propsDisplayMap;
    for (PropertiesMultiMap::const_iterator iter = properties.constBegin(); iter != properties.constEnd(); ++iter)
    {
        ItemProperty *prop = iter.value();
        int propId = iter.key();

        // don't include secondary_(min/max)damage
        QString hiddenPropertyText;
        if (!item && ((propId == ItemProperties::MinimumDamageSecondary && properties.contains(ItemProperties::MinimumDamage)) ||
                      (propId == ItemProperties::MaximumDamageSecondary && properties.contains(ItemProperties::MaximumDamage))))
        {
            continue;
        }
        else if (item)
        {
            // if it equals simple (min/max)damage
            if (propId == ItemProperties::MinimumDamageSecondary || propId == ItemProperties::MaximumDamageSecondary)
            {
                SecondaryDamageUsage u = secondaryDamageUsage(propId, prop->value, properties, item);
                if (u == UsedWithPrimary)
                    hiddenPropertyText = QString(" [%1]").arg(tr("hidden", "secondary_(min/max)damage property"));
                else if (u == Unused)
                    continue;
            }
        }

        QString displayString = prop->displayString.isEmpty() ? propertyDisplay(prop, propId, shouldColor) : prop->displayString;
        if (!displayString.isEmpty())
        {
            displayString += hiddenPropertyText;

            quint8 descPriority = ItemDataBase::Properties()->value(propId)->descPriority;
            propsDisplayMap.insertMulti(descPriority, ItemPropertyDisplay(displayString, descPriority, propId));
        }
    }

    // merge damage properties to 'adds x-y damage'
    static QHash<QPair<quint8, quint8>, QString> mergeDamagePropertiesHash;
    if (mergeDamagePropertiesHash.isEmpty())
    {
        QList<quint8> minPropIds = QList<quint8>() << ItemProperties::MinimumDamage << ItemProperties::MinimumDamageSecondary << ItemProperties::MinimumDamageFire << ItemProperties::MinimumDamageLightning
                                                   << ItemProperties::MinimumDamageMagic << ItemProperties::MinimumDamageCold << ItemProperties::MinimumDamagePoison;
        QStringList damageStrings = QStringList() << tr("Damage") << tr("Damage") << tr("fire damage") << tr("lightning damage") << tr("magic damage") << tr("cold damage") << tr("poison damage");
        for (int i = 0; i < minPropIds.size(); ++i)
            mergeDamagePropertiesHash[qMakePair<quint8, quint8>(minPropIds.at(i), minPropIds.at(i) + 1)] = damageStrings.at(i);
    }
    for (QHash<QPair<quint8, quint8>, QString>::const_iterator iter = mergeDamagePropertiesHash.constBegin(); iter != mergeDamagePropertiesHash.constEnd(); ++iter)
    {
        const QPair<quint8, quint8> &elementalPropIdPair = iter.key();
        ItemPropertyDisplay propMin = propsDisplayMap.value(ItemDataBase::Properties()->value(elementalPropIdPair.first)->descPriority), propMax = propsDisplayMap.value(ItemDataBase::Properties()->value(elementalPropIdPair.second)->descPriority);
        if (propMin.propertyId && propMax.propertyId)
        {
            propsDisplayMap.remove(propMin.priority);
            propsDisplayMap.remove(propMax.priority);

            QRegExp rx("\\d+");
            rx.indexIn(propMin.displayString);
            QString minDamage = rx.cap();
            rx.indexIn(propMax.displayString);
            QString maxDamage = rx.cap();

            QString displayString;
            if (minDamage == maxDamage)
                displayString = QString("+%1 %2").arg(minDamage).arg(iter.value());
            else
                displayString = tr("Adds %1-%2 %3").arg(minDamage, maxDamage).arg(iter.value());
            if (propMin.propertyId == ItemProperties::MinimumDamagePoison)
                displayString += tr(" over %n second(s)", 0, properties.value(propMin.propertyId)->param / 25);

            propsDisplayMap.insertMulti(propMin.priority, ItemPropertyDisplay(displayString, propMin.priority, propMin.propertyId));
        }
    }

    // group properties
    for (QMap<quint8, ItemPropertyDisplay>::iterator iter = propsDisplayMap.begin(); iter != propsDisplayMap.end(); ++iter)
    {
        ItemPropertyDisplay &itemPropDisplay = iter.value();
        ItemPropertyTxt *itemPropertyTxt = ItemDataBase::Properties()->value(itemPropDisplay.propertyId);
        if (!itemPropertyTxt->groupIDs.isEmpty())
        {
            QList<quint16> availableGroupIDs;
            int propValue = properties.value(itemPropDisplay.propertyId)->value;
            for (QMap<quint8, ItemPropertyDisplay>::const_iterator jter = propsDisplayMap.constBegin(); jter != propsDisplayMap.constEnd(); ++jter)
            {
                int propId = jter.value().propertyId;
                if (itemPropertyTxt->groupIDs.contains(propId) && properties.value(propId)->value == propValue)
                    availableGroupIDs += propId;
            }
            qSort(availableGroupIDs);
            if (itemPropertyTxt->groupIDs == availableGroupIDs) // all props from the group are present
            {
                if (properties.value(itemPropDisplay.propertyId)->value > 0)
                    itemPropDisplay.displayString.replace(itemPropertyTxt->descPositive, itemPropertyTxt->descGroupPositive);
                else
                    itemPropDisplay.displayString.replace(itemPropertyTxt->descNegative, itemPropertyTxt->descGroupNegative);

                QMap<quint8, ItemPropertyDisplay>::iterator jter = propsDisplayMap.begin();
                while (jter != propsDisplayMap.end())
                {
                    if (availableGroupIDs.contains(jter.value().propertyId))
                        jter = propsDisplayMap.erase(jter);
                    else
                        ++jter;
                }
            }
        }
    }
    *outDisplayPropertiesMultiMap = propsDisplayMap;
}

PropertiesDisplayManager::SecondaryDamageUsage PropertiesDisplayManager::secondaryDamageUsage(int secondaryDamageId, int secondaryDamageValue, const PropertiesMultiMap &allProperties, ItemInfo *item)
{
    ItemProperty *foo = new ItemProperty;
    int damageProperty = secondaryDamageId == Enums::ItemProperties::MaximumDamageSecondary ? Enums::ItemProperties::MaximumDamage : Enums::ItemProperties::MinimumDamage;
    int damageValue = allProperties.value(damageProperty, foo)->value;
    if (allProperties != item->rwProps)
        foreach (ItemInfo *socketableItem, item->socketablesInfo)
            damageValue -= socketableProperties(socketableItem, ItemDataBase::Items()->value(item->itemType)->socketableType).value(damageProperty, foo)->value;
    delete foo;

    if (damageValue)
        return damageValue != secondaryDamageValue ? UsedWithPrimary : Unused;
    else
        return UsedWithoutPrimary;
}

QString PropertiesDisplayManager::propertyDisplay(ItemProperty *propDisplay, int propId, bool shouldColor /*= false*/)
{
    int value = propDisplay->value;
    if (!value)
        return QString();

    ItemPropertyTxt *prop = ItemDataBase::Properties()->value(propId);

    const CharacterInfo::CharacterInfoBasic &basicInfo = CharacterInfo::instance().basicInfo;
    if (prop->stat.endsWith("perlevel")) // based on clvl
        value = (value * basicInfo.level) / 32;
    else if (prop->stat.endsWith("perblessedlife"))
        value = basicInfo.classCode == Enums::ClassName::Paladin ? (value * basicInfo.skills.at(Enums::Skills::currentCharacterSkillsIndexes().first.indexOf(Enums::Skills::BlessedLife))) / 32 : 0;

    char valueStringSigned[10];
#ifdef Q_CC_MSVC
    sprintf_s
#else
    sprintf
#endif
    (valueStringSigned, "%+d", prop->descFunc == 32 ? value / 10 : value);

    QString description = value < 0 ? prop->descNegative : prop->descPositive, result;
    switch (prop->descFunc) // it's described in https://d2mods.info/index.php?ind=reviews&op=entry_view&iden=448 - ItemStatCost.txt tutorial
    {
    case 0:
        break;
    case 1:
        result = QString(prop->descVal == 1 ? "%1 %2" : "%2 %1").arg(valueStringSigned).arg(description); // 1 or 2
        break;
    case 2:
        result = QString(prop->descVal == 1 ? "%1% %2" : "%2 %1%").arg(value).arg(description); // 1 or 2
        break;
    case 3:
        result = prop->descVal ? QString("%1 %2").arg(description).arg(value) : description; // 0 or 2
        break;
    case 4:
        result = QString(prop->descVal == 1 ? "%1% %2" : "%2 %1%").arg(valueStringSigned).arg(description); // 1 or 2
        break;
    case 5: // HCMTF
        result = QString("%1 %2%").arg(description).arg(value * 100 / 128);
        break;
    case 6:
        result = QString(QString(prop->descVal == 1 ? "%1 %2" : "%2 %1") + " %3").arg(valueStringSigned).arg(description).arg(prop->descStringAdd); // 1 or 2
        break;
    case 7: case 8:
        result = QString(QString(prop->descVal == 1 ? "%1% %2" : "%2 %1%") + " %3").arg(prop->descFunc == 7 ? QString::number(value) : valueStringSigned, description, prop->descStringAdd);
        break;
    case 11:
        result = tr("Repairs 1 Durability in %1 Seconds").arg(100 / value);
        break;
    case 12:
        result = QString("%1 %2").arg(description).arg(valueStringSigned);
        break;
    case 20:
        result = QString("%1% %2").arg(-value).arg(description); // 1 or 2
        break;
    case 23: // reanimate
    {
        const QString &monsterName = ItemDataBase::Monsters()->value(propDisplay->param);
        result = QString("%1% %2 %3").arg(value).arg(description).arg(shouldColor ? htmlStringFromDiabloColorString(monsterName) : monsterName);
        break;
    }
    // new in Sigma:
    case 31: case 34:
    {
        QString s = ItemDataBase::StringTable()->value(propDisplay->param, tr("tbl key %1 missing").arg(propDisplay->param));
        if (shouldColor)
        {
            enum eStringColors {
                D2C_NONE,
                D2C_RED,
                D2C_LGREEN,
                D2C_BLUE,
                D2C_GOLD,
                D2C_GREY,
                D2C_BLACK,
                D2C_TAN,
                D2C_ORANGE,
                D2C_YELLOW,
                D2C_DGREEN,
                D2C_PURPLE,
                D2C_DGREEN2
            };

            --value; // property can't store a value of 0
            ColorsManager::ColorIndex textColor;
            if (prop->descFunc == 34)
                textColor = ColorsManager::Orange;
            else if (value > D2C_DGREEN2)
                textColor = ColorsManager::Blue;
            else if (value == D2C_DGREEN2)
                textColor = ColorsManager::DarkGreen;
            else
            {
                if (value >= D2C_BLACK)
                    --value;
                textColor = static_cast<ColorsManager::ColorIndex>(value);
            }
            result = htmlStringFromDiabloColorString(s, textColor);
        }
        else
            result = s;
        break;
    }
    case 32: // HP regen
        result = QString("%1 %2").arg(valueStringSigned).arg(description);
        break;
    case 33: // cooldown
        result = description.replace(QLatin1String("%s"), ItemDataBase::Skills()->at(propDisplay->param)->name).replace(QLatin1String("%.1g"), QString::number(value / 25.0, 'g', 1) + ItemDataBase::stringFromTblKey(prop->descStringAdd));
        break;
    case 35: // innate elemental damage
    {
        if (int formatStringOffset = propDisplay->param & 0xFF)
        {
            quint32 stringIndex = ItemDataBase::StringTable()->key(description) + formatStringOffset;
            description = ItemDataBase::StringTable()->value(stringIndex);
        }
        const QByteArray descUtf8 = description.toUtf8();

        int statStringOffset = propDisplay->param >> 8;
        Enums::CharacterStats::StatisticEnum stat;
        switch (statStringOffset)
        {
        case 0:
            stat = Enums::CharacterStats::Strength;
            break;
        case 2:
            stat = Enums::CharacterStats::Dexterity;
            break;
        case 6:
            stat = Enums::CharacterStats::Vitality;
            break;
        case 9:
            stat = Enums::CharacterStats::Energy;
            break;
        default:
            return description;
        }
        quint32 statStrIndex = ItemDataBase::tblIndexLookup.value(QLatin1String("strchrstr")) + statStringOffset;
        const QByteArray statStrUtf8 = ItemDataBase::StringTable()->value(statStrIndex).toUtf8();

        int innateElementalDamage = 0; // TODO: computation ignores innate elemental damage stat from all items

        const char *formatStr = descUtf8.constData();
        float param2 = propDisplay->value * (1 + innateElementalDamage / 100.0f);
        int param1 = param2 * CharacterInfo::instance().valueOfStatistic(stat) / 100;
        const char *param3 = statStrUtf8.constData();

        int size = ::snprintf(0, 0, formatStr, param1, param2, param3) + 1;
        char *buf = new char[size];
        ::snprintf(buf, size, formatStr, param1, param2, param3);
        result = QString::fromUtf8(buf, size);
        delete [] buf;
        break;
    }
    // 9, 10, 14, 16-19 - absent
    // everything else is constructed in ItemParser (mostly in parseItemProperties()), i.e. has displayString
    default:
        result = tr("[special case %1, please report] %2 '%3' (id %4)").arg(prop->descFunc).arg(value).arg(description).arg(propId);
    }
    return result;
}

QString PropertiesDisplayManager::propertiesToHtml(const PropertiesMultiMap &properties, ItemInfo *item /*= 0*/, int textColor /*= ColorsManager::Blue*/)
{
    QMap<quint8, ItemPropertyDisplay> propsDisplayMap;
    constructPropertyStrings(properties, &propsDisplayMap, true, item);

    QString html;
    QMap<quint8, ItemPropertyDisplay>::const_iterator iter = propsDisplayMap.constEnd();
    while (iter != propsDisplayMap.constBegin())
    {
        --iter;
#ifdef SHOW_PROPERTY_ID
        html += QString("[%1] ").arg(iter.value().propertyId);
#endif
        html += htmlStringFromDiabloColorString(iter.value().displayString, ColorsManager::NoColor) + kHtmlLineBreak;
    }
    return coloredText(html, textColor);
}

PropertiesMultiMap PropertiesDisplayManager::socketableProperties(ItemInfo *socketableItem, qint8 socketableType)
{
    return ItemDataBase::isGenericSocketable(socketableItem) ? genericSocketableProperties(socketableItem, socketableType) : socketableItem->props;
}

PropertiesMultiMap PropertiesDisplayManager::genericSocketableProperties(ItemInfo *socketableItem, qint8 socketableType)
{
    PropertiesMultiMap props;
    SocketableItemInfo *socketableItemInfo = ItemDataBase::Socketables()->value(socketableItem->itemType);
    const QList<SocketableItemInfo::Properties> &socketableProps = socketableItemInfo->properties[static_cast<SocketableItemInfo::PropertyType>(socketableType + 1)];
    foreach (const SocketableItemInfo::Properties &prop, socketableProps)
    {
        if (prop.code != -1)
        {
            ItemProperty *itemProperty = new ItemProperty(prop.value, prop.param);
            switch (prop.code)
            {
            case Enums::ItemProperties::EnhancedDamage:
                itemProperty->displayString = ItemParser::kEnhancedDamageFormat().arg(prop.value);
                break;
            default:
                break;
            }
            props.insertMulti(prop.code, itemProperty);
        }
    }
    return props;
}

void PropertiesDisplayManager::addChallengeNamesToClassCharm(PropertiesMultiMap::iterator &iter)
{
    QString &desc = iter.value()->displayString;
    switch (iter.key())
    {
    case Enums::ClassCharmChallenges::WindowsInHell:
        desc = tr("They have Windows in Hell (legacy)");
        break;
    case Enums::ClassCharmChallenges::MirrorMirror:
        desc = tr("Mirror Mirror");
        break;
    case Enums::ClassCharmChallenges::Countess:
        desc = tr("Countess");
        break;
    case Enums::ClassCharmChallenges::LC2:
        desc = tr("Level Challenge 2");
        break;
    case Enums::ClassCharmChallenges::Crowned:
        desc = tr("Crowned");
        break;
    default:
        desc = tr("Challenge with id %1 found, please report!").arg(iter.key());
        break;
    }
    desc = QString("[%1]").arg(desc);
}

PropertiesMultiMap PropertiesDisplayManager::collectSetFixedProps(const QList<SetFixedProperty> &setProps, quint8 propsNumber /*= 0*/)
{
    PropertiesMultiMap setFixedProps;
    for (quint8 i = 0, n = propsNumber ? qMin(propsNumber, static_cast<quint8>(setProps.size())) : setProps.size(); i < n; ++i)
    {
        const SetFixedProperty &setProp = setProps.at(i);
        foreach (int propId, setProp.ids)
        {
            ItemProperty *prop = new ItemProperty(0, setProp.param);
            if (ItemDataBase::isCtcProperty(propId))
            {
                prop->value = setProp.minValue;
                prop->param = (setProp.param << 6) + setProp.maxValue;
            }
            else
            {
                switch (propId)
                {
                case Enums::ItemProperties::MinimumDamageFire: case Enums::ItemProperties::MinimumDamageLightning: case Enums::ItemProperties::MinimumDamageMagic: case Enums::ItemProperties::MinimumDamageCold:
                    prop->value = setProp.minValue;
                    break;
                default:
                    prop->value = setProp.maxValue;
                    break;
                }
            }
            ItemParser::createDisplayStringForPropertyWithId(propId, prop);
            setFixedProps.insert(propId, prop);
        }
    }
    return setFixedProps;
}

QString PropertiesDisplayManager::propsToString(const PropertiesMultiMap &setProps)
{
    QString s;
    QMap<quint8, ItemPropertyDisplay> propsDisplayMap;
    constructPropertyStrings(setProps, &propsDisplayMap);
    QMap<quint8, ItemPropertyDisplay>::const_iterator iter = propsDisplayMap.constEnd();
    while (iter != propsDisplayMap.constBegin())
    {
        --iter;
        s += "\n" + iter.value().displayString;
    }
    return s;
}

QStringList PropertiesDisplayManager::weaponDamageBonuses(ItemBase *itemBase)
{
    if (itemBase->genericType != Enums::ItemTypeGeneric::Weapon)
        return QStringList();

    const CharacterInfo &charInfo = CharacterInfo::instance();
    QStringList bonuses;
    bonuses.reserve(2);
    if (itemBase->strBonus > 0)
        bonuses += ItemDataBase::StringTable()->value(10104) + QString(" %1%").arg(charInfo.valueOfStatistic(Enums::CharacterStats::Strength) * itemBase->strBonus / 100);
    if (itemBase->dexBonus > 0)
        bonuses += ItemDataBase::StringTable()->value(10105) + QString(" %1%").arg(charInfo.valueOfStatistic(Enums::CharacterStats::Dexterity) * itemBase->dexBonus / 100);
    return bonuses;
}
