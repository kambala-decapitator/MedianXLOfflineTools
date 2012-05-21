#include "propertiesdisplaymanager.h"
#include "itemdatabase.h"
#include "itemparser.h"
#include "characterinfo.hpp"

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


const QList<QByteArray> PropertiesDisplayManager::damageToUndeadTypes = QList<QByteArray>() << "mace" << "hamm" << "staf" << "scep" << "club" << "wand";

QString PropertiesDisplayManager::completeItemDescription(ItemInfo *item)
{
    bool isClassCharm = ItemDataBase::isClassCharm(item);

    PropertiesMap allProps = item->props;
    if (item->isRW)
        addProperties(&allProps, item->rwProps);

    PropertiesMap::iterator iter = allProps.begin();
    while (iter != allProps.end())
    {
        if (ItemDataBase::MysticOrbs()->contains(iter.key()) && isClassCharm)
            addChallengeNamesToClassCharm(iter);
        ++iter;
    }

    const ItemBase &itemBase = ItemDataBase::Items()->value(item->itemType);
    foreach (ItemInfo *socketableItem, item->socketablesInfo)
        addProperties(&allProps, ItemDataBase::isGenericSocketable(socketableItem) ? genericSocketableProperties(socketableItem, itemBase.socketableType) : socketableItem->props);

    // create full item description
    QString itemDescription = ItemDataBase::completeItemName(item, false).replace(htmlLineBreak, "\n") + "\n" + tr("Item Level: %1").arg(item->ilvl);
    if (!itemBase.spelldesc.isEmpty())
        itemDescription += itemBase.spelldesc + "\n";

    QString runes;
    foreach (ItemInfo *socketable, item->socketablesInfo)
        if (ItemDataBase::Items()->value(socketable->itemType).typeString == "rune")
            runes += ItemDataBase::Socketables()->value(socketable->itemType).letter;
    if (!runes.isEmpty()) // gem-/jewelwords don't have any letters
        itemDescription += QString("\n'%1'").arg(runes);

    if (itemBase.genericType == Enums::ItemTypeGeneric::Armor)
    {
        int baseDef = item->defense, totalDef = baseDef;
        quint8 clvl = CharacterInfo::instance().basicInfo.level;
        ItemProperty foo;
        int ed = allProps.value(Enums::ItemProperties::EnhancedDefence, foo).value + (allProps.value(Enums::ItemProperties::EnhancedDefenceBasedOnClvl, foo).value * clvl) / 32;
        if (ed)
            totalDef = (totalDef * (100 + ed)) / 100;
        totalDef += allProps.value(Enums::ItemProperties::Defence, foo).value + (allProps.value(Enums::ItemProperties::DefenceBasedOnClvl, foo).value * clvl) / 32;
        if (totalDef < 0)
            totalDef = 0;

        QString defString = "\n" + tr("Defense: %1");
        if (baseDef != totalDef)
            itemDescription += QString("%1 (%2)").arg(defString.arg(QString::number(totalDef))).arg(baseDef);
        else
            itemDescription += defString.arg(baseDef);
    }
    if (itemBase.genericType != Enums::ItemTypeGeneric::Misc && item->maxDurability)
    {
        itemDescription += QString("\n%1: ").arg(tr("Durability"));
        bool isIndestructible = allProps.value(Enums::ItemProperties::Indestructible).value == 1;
        if (isIndestructible)
            itemDescription += QString("%1 [").arg(QChar(0x221e)); // infinity
        itemDescription += tr("%1 of %2", "durability").arg(item->currentDurability).arg(item->maxDurability);
        if (isIndestructible)
            itemDescription += "]";
    }
    if (itemBase.isStackable)
        itemDescription += "\n" + tr("Quantity: %1").arg(item->quantity);
    if (itemBase.classCode > -1)
        itemDescription += "\n" + tr("(%1 Only)", "class-specific item").arg(Enums::ClassName::classes().at(itemBase.classCode));

    int rlvl;
    switch (item->quality)
    {
    case Enums::ItemQuality::Set:
        rlvl = 100;
        break;
    case Enums::ItemQuality::Unique:
        rlvl = ItemDataBase::Uniques()->contains(item->setOrUniqueId) ? ItemDataBase::Uniques()->value(item->setOrUniqueId).rlvl : ItemDataBase::Items()->value(item->itemType).rlvl;
        break;
    default:
        rlvl = itemBase.rlvl;
        break;
    }
    int maxSocketableRlvl = 0;
    foreach (ItemInfo *socketableItem, item->socketablesInfo)
    {
        int socketableRlvl = ItemDataBase::Items()->value(socketableItem->itemType).rlvl;
        if (maxSocketableRlvl < socketableRlvl)
            maxSocketableRlvl = socketableRlvl;
    }
    int actualRlvl = qMax(rlvl, maxSocketableRlvl) + (allProps.contains(Enums::ItemProperties::RequiredLevel) ? allProps[Enums::ItemProperties::RequiredLevel].value : 0);
    if (actualRlvl)
        itemDescription += "\n" + tr("Required Level: %1").arg(/*actualRlvl > 555 ? 555 : */actualRlvl);

    // add '+50% damage to undead' if item type matches
    bool shouldAddDamageToUndeadInTheBottom = false;
    if (ItemParser::itemTypeInheritsFromTypes(itemBase.typeString, damageToUndeadTypes))
    {
        if (allProps.contains(Enums::ItemProperties::DamageToUndead))
            allProps[Enums::ItemProperties::DamageToUndead].value += 50;
        else
            shouldAddDamageToUndeadInTheBottom = true;
    }

    if (!allProps.isEmpty())
    {
        if (!item->isIdentified)
            itemDescription += "\n" + tr("[Unidentified]");

        QMap<quint8, ItemPropertyDisplay> propsDisplayMap;
        constructPropertyStrings(allProps, &propsDisplayMap);
        QMap<quint8, ItemPropertyDisplay>::const_iterator iter = propsDisplayMap.constEnd();
        while (iter != propsDisplayMap.constBegin())
        {
            --iter;
            itemDescription += "\n" + iter.value().displayString;
        }
    }
    else if (ItemDataBase::isGenericSocketable(item))
    {
        static const QStringList gearNames = QStringList() << tr("Armor") << tr("Shield") << tr("Weapon");
        QStringList propStrings;
        for (qint8 socketableType = SocketableItemInfo::Armor; socketableType <= SocketableItemInfo::Weapon; ++socketableType)
        {
            PropertiesMap props = PropertiesDisplayManager::genericSocketableProperties(item, socketableType - 1);
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
        itemDescription += "\n" + propStrings.join("");
    }

    if (shouldAddDamageToUndeadInTheBottom)
        itemDescription += "\n" + tr("+50% Damage to Undead");
    if (item->isSocketed)
        itemDescription += "\n" + tr("Socketed: (%1), Inserted: (%2)").arg(item->socketsNumber).arg(item->socketablesNumber);
    if (item->isEthereal)
        itemDescription += "\n" + tr("Ethereal (Cannot be Repaired)");
    return itemDescription;
}

void PropertiesDisplayManager::addProperties(PropertiesMap *mutableProps, const PropertiesMap &propsToAdd)
{
    for (PropertiesMultiMap::const_iterator iter = propsToAdd.constBegin(); iter != propsToAdd.constEnd(); ++iter)
    {
        int propId = iter.key();
        bool shouldNotAddNewProp;
        if ((shouldNotAddNewProp = mutableProps->contains(propId)))
        {
            ItemProperty &prop = (*mutableProps)[propId];
            if ((shouldNotAddNewProp = (prop.param == iter.value().param)))
            {
                prop.value += iter.value().value;
                if (propId == Enums::ItemProperties::EnhancedDamage)
                    prop.displayString = ItemParser::enhancedDamageFormat().arg(prop.value);
            }
        }
        if (!shouldNotAddNewProp)
            mutableProps->insertMulti(propId, iter.value());
    }
}

void PropertiesDisplayManager::constructPropertyStrings(const PropertiesMap &properties, QMap<quint8, ItemPropertyDisplay> *outDisplayPropertiesMap, bool shouldColor /*= false*/)
{
    QMap<quint8, ItemPropertyDisplay> propsDisplayMap;
    for (PropertiesMap::const_iterator iter = properties.constBegin(); iter != properties.constEnd(); ++iter)
    {
        const ItemProperty &prop = iter.value();
        int propId = iter.key();
        // don't include secondary_(min/max)damage
        if ((propId == Enums::ItemProperties::MinimumDamageSecondary && properties.contains(Enums::ItemProperties::MinimumDamage)) ||
            (propId == Enums::ItemProperties::MaximumDamageSecondary && properties.contains(Enums::ItemProperties::MaximumDamage)))
        {
            continue;
        }

        QString displayString = prop.displayString.isEmpty() ? propertyDisplay(prop, propId, shouldColor) : prop.displayString;
        if (!displayString.isEmpty())
            propsDisplayMap.insertMulti(ItemDataBase::Properties()->value(propId).descPriority,
            ItemPropertyDisplay(displayString, ItemDataBase::Properties()->value(propId).descPriority, propId));
    }

    // group properties
    for (QMap<quint8, ItemPropertyDisplay>::iterator iter = propsDisplayMap.begin(); iter != propsDisplayMap.end(); ++iter)
    {
        ItemPropertyDisplay &itemPropDisplay = iter.value();
        const ItemPropertyTxt &itemPropertyTxt = ItemDataBase::Properties()->value(itemPropDisplay.propertyId);
        if (!itemPropertyTxt.groupIDs.isEmpty())
        {
            QList<quint16> availableGroupIDs;
            int propValue = properties[itemPropDisplay.propertyId].value;
            for (QMap<quint8, ItemPropertyDisplay>::const_iterator jter = propsDisplayMap.constBegin(); jter != propsDisplayMap.constEnd(); ++jter)
            {
                int propId = jter.value().propertyId;
                if (itemPropertyTxt.groupIDs.contains(propId) && properties[propId].value == propValue)
                    availableGroupIDs += propId;
            }
            qSort(availableGroupIDs);
            if (itemPropertyTxt.groupIDs == availableGroupIDs) // all props from the group are present
            {
                if (properties[itemPropDisplay.propertyId].value > 0)
                    itemPropDisplay.displayString.replace(itemPropertyTxt.descPositive, itemPropertyTxt.descGroupPositive);
                else
                    itemPropDisplay.displayString.replace(itemPropertyTxt.descNegative, itemPropertyTxt.descGroupNegative);

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
    *outDisplayPropertiesMap = propsDisplayMap;
}

QString PropertiesDisplayManager::propertyDisplay(const ItemProperty &propDisplay, int propId, bool shouldColor /*= false*/)
{
    int value = propDisplay.value;
    if (!value)
        return QString();

    const ItemPropertyTxt &prop = ItemDataBase::Properties()->value(propId);
    QString description = value < 0 ? prop.descNegative : prop.descPositive, result;
    if (prop.descStringAdd.contains(tr("Based on", "'based on level' property; translate only if Median XL is translated into your language! (i.e. there's localized data in Resources/data/<language>)")))
    {
        const CharacterInfo::CharacterInfoBasic &basicInfo = CharacterInfo::instance().basicInfo;
        if (propId == Enums::ItemProperties::StrengthBasedOnBlessedLifeSlvl || propId == Enums::ItemProperties::DexterityBasedOnBlessedLifeSlvl)
            value = basicInfo.classCode == Enums::ClassName::Paladin ? (value * basicInfo.skills.last()) / 32 : 0; // TODO 0.3+: use ItemDataBase::Skills() to obtain Blessed Life index
        else // based on clvl
            value = (value * basicInfo.level) / 32;
    }

    char valueStringSigned[10];
#ifdef Q_WS_WIN32
    sprintf_s
#else
    sprintf
#endif
    (valueStringSigned, "%+d", value);


    switch (prop.descFunc) // it's described in http://phrozenkeep.hugelaser.com/index.php?ind=reviews&op=entry_view&iden=448 - ItemStatCost.txt tutorial
    {
    case 0:
        break;
    case 1:
        result = QString(prop.descVal == 1 ? "%1 %2" : "%2 %1").arg(valueStringSigned).arg(description); // 1 or 2
        break;
    case 2:
        result = QString(prop.descVal == 1 ? "%1% %2" : "%2 %1%").arg(value).arg(description); // 1 or 2
        break;
    case 3:
        result = prop.descVal ? QString("%1 %2").arg(description).arg(value) : description; // 0 or 2
        break;
    case 4:
        result = QString(prop.descVal == 1 ? "%1% %2" : "%2 %1%").arg(valueStringSigned).arg(description); // 1 or 2
        break;
    case 5: // HCMTF
        result = QString("%1 %2%").arg(description).arg(value * 100 / 128);
        break;
    case 6:
        result = QString(QString(prop.descVal == 1 ? "%1 %2" : "%2 %1") + " %3").arg(valueStringSigned).arg(description).arg(prop.descStringAdd); // 1 or 2
        break;
    case 7: case 8:
        result = QString(QString(prop.descVal == 1 ? "%1% %2" : "%2 %1%") + " %3").arg(prop.descFunc == 7 ? QString::number(value) : valueStringSigned, description, prop.descStringAdd);
        break;
    case 11:
        result = tr("Repairs 1 Durability in %1 Seconds").arg(100 / value);
        break;
    case 12:
        result = QString("%1 %2").arg(description).arg(valueStringSigned);
        break;
    case 20:
        result = QString("%1% %2").arg(value * -1).arg(description); // 1 or 2
        break;
    case 23: // reanimate
        {
            QString mosnterName = ItemDataBase::Monsters()->value(propDisplay.param);
            result = QString("%1% %2 %3").arg(value).arg(description).arg(shouldColor ? htmlStringFromDiabloColorString(mosnterName, ColorsManager::Blue) : mosnterName);
            break;
        }
    // 9, 10, 14, 16-19 - absent
    // everything else is constructed in ItemParser (mostly in parseItemProperties()), i.e. has displayString
    default:
        result = tr("[special case %1, please report] %2 '%3' (id %4)").arg(prop.descFunc).arg(value).arg(description).arg(propId);
    }
    return result;
}

PropertiesMap PropertiesDisplayManager::genericSocketableProperties(ItemInfo *socketableItem, qint8 socketableType)
{
    PropertiesMap props;
    const SocketableItemInfo &socketableItemInfo = ItemDataBase::Socketables()->value(socketableItem->itemType);
    const QList<SocketableItemInfo::Properties> &socketableProps = socketableItemInfo.properties[static_cast<SocketableItemInfo::PropertyType>(socketableType + 1)];
    foreach (const SocketableItemInfo::Properties &prop, socketableProps)
    {
        if (prop.code != -1)
            props[prop.code] = ItemProperty(prop.value, prop.param);
        if (prop.code == Enums::ItemProperties::EnhancedDamage)
            props[prop.code].displayString = ItemParser::enhancedDamageFormat().arg(prop.value);
    }
    return props;
}

void PropertiesDisplayManager::addChallengeNamesToClassCharm(PropertiesMap::iterator &iter)
{
    QString &desc = iter.value().displayString;
    switch (iter.key())
    {
    case Enums::ClassCharmChallenges::WindowsInHell:
        desc = tr("They have Windows in Hell");
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
