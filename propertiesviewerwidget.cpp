#include "propertiesviewerwidget.h"
#include "itemdatabase.h"
#include "enums.h"
#include "helpers.h"
#include "colors.hpp"
#include "itemparser.h"
#include "propertiesdisplaymanager.h"

#include <QSettings>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif

static const QString baseFormat("<html><body bgcolor = \"black\"><div align = \"center\" style = \"color: #ffffff\">%1</div></body></html>");


PropertiesViewerWidget::PropertiesViewerWidget(QWidget *parent) : QWidget(parent), htmlLine(htmlStringFromDiabloColorString("<hr>")), _item(0)
{
    ui.setupUi(this);

    ui.tabWidget->setCurrentIndex(0); // set tab icons
}

void PropertiesViewerWidget::showItem(ItemInfo *item)
{
    _itemMysticOrbs.clear();
    _rwMysticOrbs.clear();

    ui.allTextEdit->clear();
    ui.tabWidget->setEnabled(item != 0);

    if (!(_item = item))
    {
        ui.itemAndMysticOrbsTextEdit->clear();
        ui.rwAndMysticOrbsTextEdit->clear();
        ui.socketablesTextEdit->clear();
        return;
    }

    ui.tabWidget->setTabEnabled(0, true);
    ui.tabWidget->setTabEnabled(1, !item->props.isEmpty());
    ui.tabWidget->setTabEnabled(2, item->isRW);
    ui.tabWidget->setTabEnabled(3, !item->socketablesInfo.isEmpty());

    bool isClassCharm = ItemDataBase::isClassCharm(item);
    renderHtml(ui.itemAndMysticOrbsTextEdit, collectMysticOrbsDataFromProps(&_itemMysticOrbs, item->props, isClassCharm));

    PropertiesMap allProps = item->props;
    if (item->isRW)
    {
        renderHtml(ui.rwAndMysticOrbsTextEdit, collectMysticOrbsDataFromProps(&_rwMysticOrbs, item->rwProps, isClassCharm));
        PropertiesDisplayManager::addProperties(&allProps, item->rwProps);
    }

    PropertiesMap::iterator iter = allProps.begin();
    while (iter != allProps.end())
    {
        if (ItemDataBase::MysticOrbs()->contains(iter.key()))
        {
            if (isClassCharm)
            {
                PropertiesDisplayManager::addChallengeNamesToClassCharm(iter);
                ++iter;
            }
            else
                iter = allProps.erase(iter);
        }
        else
            ++iter;
    }

    const ItemBase &itemBase = ItemDataBase::Items()->value(item->itemType);
    ui.socketablesTextEdit->clear();
    if (!item->socketablesInfo.isEmpty())
    {
        QString html;
        foreach (ItemInfo *socketableItem, item->socketablesInfo)
        {
            PropertiesMap props = ItemDataBase::isGenericSocketable(socketableItem) ? PropertiesDisplayManager::genericSocketableProperties(socketableItem, itemBase.socketableType) : socketableItem->props;
            PropertiesDisplayManager::addProperties(&allProps, props);
            html += ItemDataBase::completeItemName(socketableItem, true) + htmlLineBreak + propertiesToHtml(props) + htmlLine;
        }
        renderHtml(ui.socketablesTextEdit, html);
    }

    // create full item description
    QString itemDescription = ItemDataBase::completeItemName(item, true) + htmlLineBreak + tr("Item Level: %1").arg(item->ilvl) + htmlLineBreak;
    if (!itemBase.spelldesc.isEmpty())
        itemDescription += htmlStringFromDiabloColorString(itemBase.spelldesc) + htmlLineBreak;
    if (item->isRW)
    {
        QString runes;
        foreach (ItemInfo *socketable, item->socketablesInfo)
            if (ItemDataBase::Items()->value(socketable->itemType).typeString == "rune")
                runes += ItemDataBase::Socketables()->value(socketable->itemType).letter;
        if (!runes.isEmpty()) // gem-/jewelwords don't have any letters
            itemDescription += htmlStringFromDiabloColorString(QString("'%1'").arg(runes), Gold) + htmlLineBreak;
    }

    if (itemBase.genericType == Enums::ItemTypeGeneric::Armor)
    {
        int baseDef = item->defense, totalDef = baseDef;
        ItemProperty foo;
        int ed = allProps.value(Enums::ItemProperties::EnhancedDefence, foo).value + (allProps.value(Enums::ItemProperties::EnhancedDefenceBasedOnClvl, foo).value * *ItemDataBase::clvl) / 32;
        if (ed)
            totalDef = (totalDef * (100 + ed)) / 100;
        totalDef += allProps.value(Enums::ItemProperties::Defence, foo).value + (allProps.value(Enums::ItemProperties::DefenceBasedOnClvl, foo).value * *ItemDataBase::clvl) / 32;
        if (totalDef < 0)
            totalDef = 0;

        QString defString = tr("Defense: %1");
        if (baseDef != totalDef)
            itemDescription += defString.arg(htmlStringFromDiabloColorString(QString::number(totalDef), Blue)) + QString(" (%1)").arg(baseDef);
        else
            itemDescription += defString.arg(baseDef);
        itemDescription += htmlLineBreak;
    }
    if (itemBase.genericType != Enums::ItemTypeGeneric::Misc && item->maxDurability)
    {
        itemDescription += tr("Durability") + ": ";
        bool isIndestructible = allProps.value(Enums::ItemProperties::Indestructible).value == 1;
        if (isIndestructible)
            itemDescription += QString("%1 [").arg(QChar(0x221e)); // infinity
        itemDescription += tr("%1 of %2", "durability").arg(item->currentDurability).arg(item->maxDurability);
        if (isIndestructible)
            itemDescription += "]";
        itemDescription += htmlLineBreak;
    }
    if (itemBase.isStackable)
        itemDescription += tr("Quantity: %1").arg(item->quantity) + htmlLineBreak;
    if (itemBase.classCode > -1)
    {
        QString text = tr("(%1 Only)", "class-specific item").arg(Enums::ClassName::classes().at(itemBase.classCode));
        if (itemBase.classCode != *ItemDataBase::charClass)
            itemDescription += htmlStringFromDiabloColorString(text, Red);
        else
            itemDescription += text;
        itemDescription += htmlLineBreak;
    }

    int rlvl;
    switch (item->quality)
    {
    case Enums::ItemQuality::Set:
        rlvl = 100;
        break;
    case Enums::ItemQuality::Unique:
        rlvl = ItemDataBase::Uniques()->contains(item->setOrUniqueId) ? ItemDataBase::Uniques()->value(item->setOrUniqueId).rlvl : ItemDataBase::Items()->value(item->itemType).rlvl;
        break;
//  case Enums::ItemQuality::Rare: case Enums::ItemQuality::Crafted: case Enums::ItemQuality::Magic:
        // TODO 0.3+: add support for affix rlvl
//      break;
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
        itemDescription += htmlStringFromDiabloColorString(tr("Required Level: %1").arg(actualRlvl > 555 ? 555 : actualRlvl), *ItemDataBase::clvl < actualRlvl ? Red : White) + htmlLineBreak;

    // add '+50% damage to undead' if item type matches
    bool shouldAddDamageToUndeadInTheBottom = false;
    if (ItemParser::itemTypeInheritsFromTypes(itemBase.typeString, PropertiesDisplayManager::damageToUndeadTypes))
    {
        if (allProps.contains(Enums::ItemProperties::DamageToUndead))
            allProps[Enums::ItemProperties::DamageToUndead].value += 50;
        else
            shouldAddDamageToUndeadInTheBottom = true;
    }

    if (!allProps.isEmpty())
        itemDescription += propertiesToHtml(allProps);
    else if (ItemDataBase::isGenericSocketable(item))
    {
        static const QStringList gearNames = QStringList() << tr("Armor") << tr("Shield") << tr("Weapon");
        QStringList propStrings;
        for (qint8 socketableType = SocketableItemInfo::Armor; socketableType <= SocketableItemInfo::Weapon; ++socketableType)
        {
            PropertiesMap props = PropertiesDisplayManager::genericSocketableProperties(item, socketableType - 1);
            QString propText = propertiesToHtml(props).replace(QRegExp(htmlLineBreak + "(?!</font>)"), ", "); // don't replace last <br>
            propStrings += QString("%1: %2").arg(gearNames.at(socketableType), propText);
        }
        // weapon properties should be first
        propStrings.move(propStrings.size() - 1, 0);
        itemDescription += htmlLineBreak + propStrings.join("");
    }

    if (shouldAddDamageToUndeadInTheBottom)
        itemDescription += htmlStringFromDiabloColorString(tr("+50% Damage to Undead"), Blue) + htmlLineBreak;
    if (item->isSocketed)
        itemDescription += htmlStringFromDiabloColorString(tr("Socketed: (%1), Inserted: (%2)").arg(item->socketsNumber).arg(item->socketablesNumber), Blue) + htmlLineBreak;
    if (item->isEthereal)
        itemDescription += htmlStringFromDiabloColorString(tr("Ethereal (Cannot be Repaired)"), Blue) + htmlLineBreak;

    // show existing set items from current set
    if (item->quality == Enums::ItemQuality::Set)
    {
        const QString &setName = ItemDataBase::Sets()->value(item->setOrUniqueId).setName;
        itemDescription += htmlLineBreak + htmlStringFromDiabloColorString(setName, Gold);

        foreach (const QString &setItemName, ItemDataBase::completeSetForName(setName))
        {
            bool found = false;
            foreach (ItemInfo *anItem, *ItemDataBase::currentCharacterItems)
            {
                if (anItem->quality == Enums::ItemQuality::Set && ItemDataBase::Sets()->value(anItem->setOrUniqueId).itemName == setItemName)
                {
                    found = true;
                    break;
                }
            }
            itemDescription += htmlLineBreak + htmlStringFromDiabloColorString(setItemName, found ? Green : Red);
        }
    }

    renderHtml(ui.allTextEdit, itemDescription);

    // awkward way to force center align
    QSize originalSize = size();
    resize(originalSize.width() + 1, originalSize.height());
    resize(originalSize);
}

QString PropertiesViewerWidget::propertiesToHtml(const PropertiesMap &properties)
{
    QMap<quint8, ItemPropertyDisplay> propsDisplayMap;
    PropertiesDisplayManager::constructPropertyStrings(properties, &propsDisplayMap, true);
    QString html;
    QMap<quint8, ItemPropertyDisplay>::const_iterator iter = propsDisplayMap.constEnd();
    while (iter != propsDisplayMap.constBegin())
    {
        --iter;
        html += htmlStringFromDiabloColorString(iter.value().displayString, NoColor) + htmlLineBreak;
    }
    return coloredText(html, Blue);
}

void PropertiesViewerWidget::renderHtml(QTextEdit *textEdit, const QString &description)
{
    textEdit->setText(baseFormat.arg(QString(description).replace('\n', htmlLineBreak)));
}

void PropertiesViewerWidget::removeAllMysticOrbs()
{
    removeMysticOrbsFromProperties(_itemMysticOrbs, &_item->props);
    removeMysticOrbsFromProperties(_rwMysticOrbs, &_item->rwProps);

    // byte align
    int extraBits = _item->bitString.length() % 8;
    if (extraBits)
    {
        int zerosBeforeFirst1 = _item->bitString.indexOf('1'), zerosToAppend = 8 - extraBits;
        if (zerosBeforeFirst1 + zerosToAppend < 8)
            _item->bitString.prepend(QString(zerosToAppend, zeroChar));
        else
            _item->bitString.remove(0, extraBits);
    }

    _item->hasChanged = true;
    showItem(_item);
}

void PropertiesViewerWidget::removeMysticOrbsFromProperties(const QSet<int> &mysticOrbs, PropertiesMultiMap *props)
{
    int moNumber = 0;
    foreach (int moCode, mysticOrbs)
    {
        modifyMysticOrbProperty(ItemDataBase::MysticOrbs()->value(moCode).statId, totalMysticOrbValue(moCode, props), props);

        // remove MO data
        const ItemPropertyTxt &propertyTxt = ItemDataBase::Properties()->value(moCode);
        int valueIndex = indexOfPropertyValue(moCode, props);
        if (valueIndex > -1)
            _item->bitString.remove(valueIndex, propertyTxt.bits + propertyTxt.saveParamBits + Enums::CharacterStats::StatCodeLength);

        moNumber += props->value(moCode).value;
        props->remove(moCode);
    }

    // decrease rlvl
    modifyMysticOrbProperty(Enums::ItemProperties::RequiredLevel, moNumber * 2, props);
}

int PropertiesViewerWidget::indexOfPropertyValue(int id, PropertiesMultiMap *props)
{
    bool isMaxEnhDamageProp = id == Enums::ItemProperties::EnhancedDamage;
    const ItemPropertyTxt &property = ItemDataBase::Properties()->value(id);
    qulonglong value = props->value(id).value + property.add;
    if (isMaxEnhDamageProp)
    {
        qulonglong oldValue = value;
        value <<= property.bits;
        value += oldValue;
    }

    int idIndex = -1, valueIndex = -1, bits = property.bits * (1 + isMaxEnhDamageProp);
    do
    {
        if ((idIndex = _item->bitString.indexOf(binaryStringFromNumber(id, false, Enums::CharacterStats::StatCodeLength), idIndex + 1)) == -1)
            return -1;
        valueIndex = idIndex - property.saveParamBits - bits;
    } while (_item->bitString.mid(valueIndex, bits).toULongLong(0, 2) != value);
    return valueIndex;
}

void PropertiesViewerWidget::modifyMysticOrbProperty(int id, int decrement, PropertiesMultiMap *props)
{
    int valueIndex = indexOfPropertyValue(id, props);
    if (valueIndex <= -1)
        return;

    // ED value is stored as a sequence of 2 equal values
    bool isEnhancedDamageProp = id == Enums::ItemProperties::EnhancedDamage;
    const ItemPropertyTxt &propertyTxt = ItemDataBase::Properties()->value(id);
    int bitsLength = (1 + isEnhancedDamageProp) * propertyTxt.bits;

    ItemProperty prop = props->value(id);
    prop.value -= decrement;
    if (prop.value)
    {
        QString newBits = binaryStringFromNumber(prop.value + propertyTxt.add, false, propertyTxt.bits); // it's not a mistake that I'm not using bitsLength here
        if (isEnhancedDamageProp)
        {
            newBits += newBits;
            prop.displayString = ItemParser::enhancedDamageFormat().arg(prop.value);
        }
        _item->bitString.replace(valueIndex, bitsLength, newBits); // place modified value
        props->replace(id, prop);
    }
    else
    {
        _item->bitString.remove(valueIndex, bitsLength + propertyTxt.saveParamBits + Enums::CharacterStats::StatCodeLength);
        props->remove(id);
    }
}

int PropertiesViewerWidget::totalMysticOrbValue(int moCode, PropertiesMap *props)
{
    quint8 multiplier = 1 + isMysticOrbEffectDoubled();
    return props->value(moCode).value * ItemDataBase::MysticOrbs()->value(moCode).value * multiplier;
}

QString PropertiesViewerWidget::collectMysticOrbsDataFromProps(QSet<int> *moSet, PropertiesMap &props, bool isClassCharm)
{
    PropertiesMap propsWithoutMO = props;
    PropertiesMap::iterator iter = propsWithoutMO.begin();
    while (iter != propsWithoutMO.end())
    {
        if (ItemDataBase::MysticOrbs()->contains(iter.key()))
        {
            if (!isClassCharm)
                moSet->insert(iter.key());
            iter = propsWithoutMO.erase(iter);
        }
        else
            ++iter;
    }
    QString html = propertiesToHtml(propsWithoutMO);

    if (!moSet->isEmpty())
    {
        html += htmlLine + htmlLineBreak;
        foreach (int moCode, *moSet)
        {
            if (isMysticOrbEffectDoubled())
                props[moCode].displayString += " x 2";
            // quick & dirty hack with const_cast
            html += QString("%1 = %2").arg(props[moCode].displayString).arg(totalMysticOrbValue(moCode, const_cast<PropertiesMap *>(&props))) + htmlLineBreak;
        }
    }
    return html;
}

bool PropertiesViewerWidget::isMysticOrbEffectDoubled()
{
    return _item->quality == Enums::ItemQuality::Crafted || _item->quality == Enums::ItemQuality::Honorific;
}
