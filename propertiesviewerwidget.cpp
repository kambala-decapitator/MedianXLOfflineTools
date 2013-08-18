#include    "propertiesviewerwidget.h"
#include "ui_propertiesviewerwidget.h"
#include "itemdatabase.h"
#include "enums.h"
#include "helpers.h"
#include "colorsmanager.hpp"
#include "itemparser.h"
#include "propertiesdisplaymanager.h"
#include "characterinfo.hpp"

#include <QSettings>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


static const QString kBaseFormat("<html><body bgcolor=\"black\"><div align=\"center\" style=\"color: #ffffff\">%1</div></body></html>");


PropertiesViewerWidget::PropertiesViewerWidget(QWidget *parent) : QWidget(parent), htmlLine(htmlStringFromDiabloColorString("<hr />")), ui(new Ui::PropertiesViewerWidget), _item(0)
{
    ui->setupUi(this);

    ui->tabWidget->setCurrentIndex(0); // set tab icons
}

PropertiesViewerWidget::~PropertiesViewerWidget()
{
    delete ui;
}

void PropertiesViewerWidget::showItem(ItemInfo *item)
{
    using namespace Enums;

    _itemMysticOrbs.clear();
    _rwMysticOrbs.clear();

    ui->allTextEdit->clear();
    ui->tabWidget->setEnabled(item != 0);

    if (!(_item = item))
    {
        ui->itemAndMysticOrbsTextEdit->clear();
        ui->rwAndMysticOrbsTextEdit->clear();
        ui->socketablesTextEdit->clear();
        return;
    }

    ui->tabWidget->setTabEnabled(0, true);
    ui->tabWidget->setTabEnabled(1, !item->props.isEmpty());
    ui->tabWidget->setTabEnabled(2, item->isRW);
    ui->tabWidget->setTabEnabled(3, !item->socketablesInfo.isEmpty());

    QString ilvlText = kHtmlLineBreak + qApp->translate("PropertiesDisplayManager", "Item Level: %1").arg(item->ilvl);
    if (item->isEar)
    {
        QString itemDescription = qApp->translate("PropertiesDisplayManager", "%1's Ear", "param is character name").arg(item->earInfo.name.constData()) + kHtmlLineBreak;
        itemDescription += ClassName::classes().at(item->earInfo.classCode) + kHtmlLineBreak;
        itemDescription += qApp->translate("PropertiesDisplayManager", "Level %1").arg(item->earInfo.level);
        renderHtml(ui->allTextEdit, itemDescription + ilvlText);
        return;
    }

    renderHtml(ui->itemAndMysticOrbsTextEdit, collectMysticOrbsDataFromProps(&_itemMysticOrbs, item->props));

    PropertiesMultiMap allProps;
    PropertiesMultiMap::const_iterator constIter = item->props.constBegin();
    while (constIter != item->props.constEnd())
    {
        allProps.insertMulti(constIter.key(), new ItemProperty(*constIter.value())); // original values mustn't be modified
        ++constIter;
    }

    if (item->isRW)
    {
        renderHtml(ui->rwAndMysticOrbsTextEdit, collectMysticOrbsDataFromProps(&_rwMysticOrbs, item->rwProps));
        PropertiesDisplayManager::addProperties(&allProps, item->rwProps);
    }

    bool isClassCharm = ItemDataBase::isClassCharm(item), isTradersChest_ = isTradersChest(item);
    PropertiesMultiMap::iterator iter = allProps.begin();
    while (iter != allProps.end())
    {
        if (ItemDataBase::MysticOrbs()->contains(iter.key()))
        {
            // !!!: add custom text for charms that use MO codes here
            if (isClassCharm)
            {
                PropertiesDisplayManager::addChallengeNamesToClassCharm(iter);
                ++iter;
            }
            else if (isTradersChest_)
            {
                ItemProperty *prop = iter.value();
                prop->displayString = QString("[%1]").arg(tr("%n gem(s) used", "for Trader's Chest", prop->value));
                ++iter;
            }
            else
                iter = allProps.erase(iter);
        }
        else
            ++iter;
    }

    ItemBase *itemBase = ItemDataBase::Items()->value(item->itemType);
    ui->socketablesTextEdit->clear();
    if (!item->socketablesInfo.isEmpty())
    {
        QString html;
        foreach (ItemInfo *socketableItem, item->socketablesInfo)
        {
            PropertiesMap socketableProps = PropertiesDisplayManager::socketableProperties(socketableItem, itemBase->socketableType);
            PropertiesDisplayManager::addProperties(&allProps, socketableProps);
            html += ItemDataBase::completeItemName(socketableItem, true) + kHtmlLineBreak + propertiesToHtml(socketableProps) + htmlLine;
        }
        renderHtml(ui->socketablesTextEdit, html);
    }

    // create full item description
    QString itemDescription = ItemDataBase::completeItemName(item, true) + ilvlText + kHtmlLineBreak;
    if (!itemBase->spelldesc.isEmpty())
        itemDescription += htmlStringFromDiabloColorString(itemBase->spelldesc) + kHtmlLineBreak;

    QString runes;
    foreach (ItemInfo *socketable, item->socketablesInfo)
    {
        if (ItemDataBase::Items()->value(socketable->itemType)->types.first() == "rune")
        {
            SocketableItemInfo *sock = ItemDataBase::Socketables()->value(socketable->itemType);
            if (sock)
                runes += sock->letter;
            else
            {
                runes += "WTF";
                qDebug("no socketable found for %s", socketable->itemType.constData());
            }
        }
    }
    if (!runes.isEmpty()) // gem-/jewelwords don't have any letters
        itemDescription += htmlStringFromDiabloColorString(QString("'%1'").arg(runes), ColorsManager::Gold) + kHtmlLineBreak;

    const CharacterInfo &charInfo = CharacterInfo::instance();
    const CharacterInfo::CharacterInfoBasic &charBasicInfo = charInfo.basicInfo;
    quint8 clvl = charBasicInfo.level;
    ItemProperty *foo = new ItemProperty;

    if (itemBase->genericType == ItemTypeGeneric::Armor)
    {
        int baseDef = item->defense, totalDef = baseDef;
        int ed =    allProps.value(ItemProperties::EnhancedDefence, foo)->value + (      allProps.value(ItemProperties::EnhancedDefenceBasedOnClvl, foo)->value * clvl) / 32;
        ed += item->setProps.value(ItemProperties::EnhancedDefence, foo)->value + (item->setProps.value(ItemProperties::EnhancedDefenceBasedOnClvl, foo)->value * clvl) / 32;
        if (ed)
            totalDef = (totalDef * (100 + ed)) / 100;
        totalDef +=       allProps.value(ItemProperties::Defence, foo)->value + (      allProps.value(ItemProperties::DefenceBasedOnClvl, foo)->value * clvl) / 32;
        totalDef += item->setProps.value(ItemProperties::Defence, foo)->value + (item->setProps.value(ItemProperties::DefenceBasedOnClvl, foo)->value * clvl) / 32;
        if (totalDef < 0)
            totalDef = 0;

        QString defString = qApp->translate("PropertiesDisplayManager", "Defense: %1");
        if (baseDef != totalDef)
            itemDescription += defString.arg(htmlStringFromDiabloColorString(QString::number(totalDef), ColorsManager::Blue)) + QString(" (%1)").arg(baseDef);
        else
            itemDescription += defString.arg(baseDef);
        itemDescription += kHtmlLineBreak;
    }
    else if (itemBase->genericType == ItemTypeGeneric::Weapon)
    {
        // TODO: [later] use lambda to calculate damage
        int ed = allProps.value(ItemProperties::EnhancedDamage, foo)->value;

        int minDmgTotal = allProps.value(ItemProperties::MinimumDamage, foo)->value, minDmgSecondary = allProps.value(ItemProperties::MinimumDamageSecondary, foo)->value;
        PropertiesDisplayManager::SecondaryDamageUsage u = PropertiesDisplayManager::secondaryDamageUsage(ItemProperties::MinimumDamageSecondary, minDmgSecondary, allProps, item);
        if (u != PropertiesDisplayManager::Unused)
            minDmgTotal += minDmgSecondary;

        int maxDmgTotal = allProps.value(ItemProperties::MaximumDamage, foo)->value, maxDmgSecondary = allProps.value(ItemProperties::MaximumDamageSecondary, foo)->value;
        u = PropertiesDisplayManager::secondaryDamageUsage(ItemProperties::MaximumDamageSecondary, maxDmgSecondary, allProps, item);
        if (u != PropertiesDisplayManager::Unused)
            maxDmgTotal += maxDmgSecondary;
        maxDmgTotal += (allProps.value(ItemProperties::MaximumDamageBasedOnClvl, foo)->value * clvl) / 32;

        ColorsManager::ColorIndex damageColor = ed + minDmgTotal + maxDmgTotal > 0 ? ColorsManager::Blue : ColorsManager::White; // blue if any property exists
        QString damageFormat = qApp->translate("PropertiesDisplayManager", "%1 to %2", "min-max damage");

        // if min == max, then increment max by 1
        if (itemBase->minThrowDmg && itemBase->maxThrowDmg)
        {
            int minDmg = itemBase->minThrowDmg, maxDmg = itemBase->maxThrowDmg;
            if (ed)
            {
                minDmg = (minDmg * (100 + ed)) / 100;
                maxDmg = (maxDmg * (100 + ed)) / 100;
            }
            minDmg += minDmgTotal;
            maxDmg += maxDmgTotal;

            itemDescription += qApp->translate("PropertiesDisplayManager", "Throw Damage") + ": " + htmlStringFromDiabloColorString(damageFormat.arg(minDmg).arg(maxDmg + (minDmg == maxDmg)), damageColor) + kHtmlLineBreak;
        }

        if (itemBase->min1hDmg && itemBase->max1hDmg)
        {
            int minDmg = itemBase->min1hDmg, maxDmg = itemBase->max1hDmg;
            if (ed)
            {
                minDmg = (minDmg * (100 + ed)) / 100;
                maxDmg = (maxDmg * (100 + ed)) / 100;
            }
            minDmg += minDmgTotal;
            maxDmg += maxDmgTotal;

            itemDescription += qApp->translate("PropertiesDisplayManager", "One-Hand Damage") + ": " + htmlStringFromDiabloColorString(damageFormat.arg(minDmg).arg(maxDmg + (minDmg == maxDmg)), damageColor) + kHtmlLineBreak;
        }

        if (itemBase->min2hDmg && itemBase->max2hDmg)
        {
            int minDmg = itemBase->min2hDmg, maxDmg = itemBase->max2hDmg;
            if (ed)
            {
                minDmg = (minDmg * (100 + ed)) / 100;
                maxDmg = (maxDmg * (100 + ed)) / 100;
            }
            minDmg += minDmgTotal;
            maxDmg += maxDmgTotal;

            itemDescription += qApp->translate("PropertiesDisplayManager", "Two-Hand Damage") + ": " + htmlStringFromDiabloColorString(damageFormat.arg(minDmg).arg(maxDmg + (minDmg == maxDmg)), damageColor) + kHtmlLineBreak;
        }
    }
    if (itemBase->genericType != ItemTypeGeneric::Misc && item->maxDurability)
    {
        itemDescription += qApp->translate("PropertiesDisplayManager", "Durability") + ": ";
        bool isIndestructible = allProps.value(ItemProperties::Indestructible, foo)->value == 1;
        if (isIndestructible)
            itemDescription += QString("%1 [").arg(QChar(0x221e)); // infinity
        itemDescription += qApp->translate("PropertiesDisplayManager", "%1 of %2", "durability").arg(item->currentDurability).arg(item->maxDurability);
        if (isIndestructible)
            itemDescription += "]";
        itemDescription += kHtmlLineBreak;
    }
    if (itemBase->isStackable)
        itemDescription += qApp->translate("PropertiesDisplayManager", "Quantity: %1").arg(item->quantity) + kHtmlLineBreak;
    if (itemBase->classCode > -1)
    {
        QString text = qApp->translate("PropertiesDisplayManager", "(%1 Only)", "class-specific item").arg(ClassName::classes().at(itemBase->classCode));
        if (itemBase->classCode != charBasicInfo.classCode)
            itemDescription += htmlStringFromDiabloColorString(text, ColorsManager::Red);
        else
            itemDescription += text;
        itemDescription += kHtmlLineBreak;
    }

    // TODO: [later] use lambda to calculate requirements
    if (itemBase->rdex)
        if (quint16 rdex = itemBase->rdex + (itemBase->rdex * allProps.value(ItemProperties::Requirements, foo)->value) / 100)
            itemDescription += htmlStringFromDiabloColorString(qApp->translate("PropertiesDisplayManager", "Required Dexterity: %1").arg(rdex), charInfo.valueOfStatistic(CharacterStats::Dexterity) < rdex ? ColorsManager::Red : ColorsManager::White)
                               + kHtmlLineBreak;

    if (itemBase->rstr)
        if (quint16 rstr = itemBase->rstr + (itemBase->rstr * allProps.value(ItemProperties::Requirements, foo)->value) / 100)
            itemDescription += htmlStringFromDiabloColorString(qApp->translate("PropertiesDisplayManager", "Required Strength: %1").arg(rstr),  charInfo.valueOfStatistic(CharacterStats::Strength)  < rstr ? ColorsManager::Red : ColorsManager::White)
                               + kHtmlLineBreak;

    int rlvl;
    switch (item->quality)
    {
    case ItemQuality::Set: case ItemQuality::Unique:
        {
            SetOrUniqueItemInfo *info;
            if (item->quality == ItemQuality::Set)
                info = ItemDataBase::Sets()->value(item->setOrUniqueId);
            else
                info = ItemDataBase::Uniques()->value(item->setOrUniqueId);
            rlvl = info ? info->rlvl : itemBase->rlvl;
        }
        break;
//  case ItemQuality::Rare: case ItemQuality::Crafted: case ItemQuality::Magic:
        // TODO: [0.5+] add support for affix rlvl
//      break;
    default:
        rlvl = itemBase->rlvl;
        break;
    }
    int maxSocketableRlvl = 0;
    foreach (ItemInfo *socketableItem, item->socketablesInfo)
    {
        int socketableRlvl = ItemDataBase::Items()->value(socketableItem->itemType)->rlvl;
        if (maxSocketableRlvl < socketableRlvl)
            maxSocketableRlvl = socketableRlvl;
    }
    if (int actualRlvl = qMax(rlvl, maxSocketableRlvl) + allProps.value(ItemProperties::RequiredLevel, foo)->value)
        itemDescription += htmlStringFromDiabloColorString(qApp->translate("PropertiesDisplayManager", "Required Level: %1").arg(actualRlvl), clvl < actualRlvl ? ColorsManager::Red : ColorsManager::White) + kHtmlLineBreak;
    delete foo;

    // add '+50% damage to undead' if item type matches
    bool shouldAddDamageToUndeadInTheBottom = false;
    if (ItemParser::itemTypesInheritFromTypes(itemBase->types, PropertiesDisplayManager::kDamageToUndeadTypes))
    {
        if (ItemProperty *damageToUndeadProp = allProps.value(ItemProperties::DamageToUndead))
            damageToUndeadProp->value += 50;
        else
            shouldAddDamageToUndeadInTheBottom = true;
    }

    if (!allProps.isEmpty())
    {
        if (!item->isIdentified)
            itemDescription += htmlStringFromDiabloColorString(qApp->translate("PropertiesDisplayManager", "[Unidentified]"), ColorsManager::Red) + kHtmlLineBreak;
        itemDescription += propertiesToHtml(allProps);
    }
    else if (ItemDataBase::isGenericSocketable(item))
    {
        static const QStringList gearNames = QStringList() << qApp->translate("PropertiesDisplayManager", "Armor") << qApp->translate("PropertiesDisplayManager", "Shield") << qApp->translate("PropertiesDisplayManager", "Weapon");
        QStringList propStrings;
        for (qint8 socketableType = SocketableItemInfo::Armor; socketableType <= SocketableItemInfo::Weapon; ++socketableType)
        {
            PropertiesMap props = PropertiesDisplayManager::genericSocketableProperties(item, socketableType - 1);
            QString propText = propertiesToHtml(props).replace(QRegExp(kHtmlLineBreak + "(?!</font>)"), ", "); // don't replace last <br>
            propStrings += QString("%1: %2").arg(gearNames.at(socketableType), propText);
        }
        // weapon properties should be first
        propStrings.move(propStrings.size() - 1, 0);
        itemDescription += kHtmlLineBreak + propStrings.join("");
    }

    if (shouldAddDamageToUndeadInTheBottom)
        itemDescription += htmlStringFromDiabloColorString(qApp->translate("PropertiesDisplayManager", "+50% Damage to Undead"), ColorsManager::Blue) + kHtmlLineBreak;
    if (item->isSocketed)
        itemDescription += htmlStringFromDiabloColorString(qApp->translate("PropertiesDisplayManager", "Socketed: (%1), Inserted: (%2)").arg(item->socketsNumber).arg(item->socketablesNumber), ColorsManager::Blue) + kHtmlLineBreak;
    if (item->isEthereal)
        itemDescription += htmlStringFromDiabloColorString(qApp->translate("PropertiesDisplayManager", "Ethereal (Cannot be Repaired)"), ColorsManager::Blue) + kHtmlLineBreak;

    if (item->quality == ItemQuality::Set)
    {
        // set item properties stored in item
        if (!item->setProps.isEmpty())
            itemDescription += propertiesToHtml(item->setProps, ColorsManager::Green);

        // set item properties from txt
        PropertiesMultiMap setItemFixedProps = collectSetFixedProps(ItemDataBase::Sets()->value(item->setOrUniqueId)->fixedProperties);
        if (!setItemFixedProps.isEmpty())
            itemDescription += propertiesToHtml(setItemFixedProps, ColorsManager::Green);
        qDeleteAll(setItemFixedProps);

        // set properties
        SetItemInfo *setItem = ItemDataBase::Sets()->value(item->setOrUniqueId);
        const FullSetInfo fullSetInfo = ItemDataBase::fullSetInfoForKey(setItem->key);
        if (item->location == ItemLocation::Equipped || item->location == ItemLocation::Corpse || item->location == ItemLocation::Merc)
        {
            int setItemsOnCharacter = 1;
            foreach (ItemInfo *anItem, ItemDataBase::itemsStoredIn(item->storage, item->location))
                if (anItem != item && anItem->quality == ItemQuality::Set && fullSetInfo.itemNames.contains(ItemDataBase::Sets()->value(anItem->setOrUniqueId)->itemName))
                    ++setItemsOnCharacter;
            if (setItemsOnCharacter > 1)
            {
                PropertiesMultiMap setFixedProps = collectSetFixedProps(fullSetInfo.partialSetProperties, setItemsOnCharacter - 1);
                if (setItemsOnCharacter == fullSetInfo.itemNames.size())
                {
                    PropertiesMultiMap fullSetProperties = collectSetFixedProps(fullSetInfo.fullSetProperties);
                    for (PropertiesMultiMap::const_iterator iter = fullSetProperties.constBegin(); iter != fullSetProperties.constEnd(); ++iter)
                    {
                        ItemProperty *prop = iter.value();
                        if (prop->param)
                            setFixedProps.insert(iter.key(), prop);
                        else
                        {
                            ItemProperty *existingProp = setFixedProps.value(iter.key());
                            if (existingProp)
                                existingProp->value += prop->value;
                            else
                                setFixedProps.insert(iter.key(), prop);
                        }
                    }
                }
                itemDescription += kHtmlLineBreak + propertiesToHtml(setFixedProps, ColorsManager::Gold);
                qDeleteAll(setFixedProps);
            }
        }

        // set item names from current set with correct color
        itemDescription += kHtmlLineBreak + htmlStringFromDiabloColorString(setItem->setName, ColorsManager::Gold);

        foreach (const QString &setItemName, fullSetInfo.itemNames)
        {
            bool found = false;
            foreach (ItemInfo *anItem, charInfo.items.character) //-V807
            {
                if (anItem->quality == ItemQuality::Set && ItemDataBase::Sets()->value(anItem->setOrUniqueId)->itemName == setItemName)
                {
                    found = true;
                    break;
                }
            }
            itemDescription += kHtmlLineBreak + htmlStringFromDiabloColorString(setItemName, found ? ColorsManager::Green : ColorsManager::Red);
        }
    }

    qDeleteAll(allProps);

    renderHtml(ui->allTextEdit, itemDescription);

    // awkward way to force center align
    QSize originalSize = size();
    resize(originalSize.width() + 1, originalSize.height());
    resize(originalSize);
}

QString PropertiesViewerWidget::propertiesToHtml(const PropertiesMap &properties, int textColor /*= ColorsManager::Blue*/)
{
    QMap<quint8, ItemPropertyDisplay> propsDisplayMap;
    PropertiesDisplayManager::constructPropertyStrings(properties, &propsDisplayMap, true, _item);
    QString html;
    QMap<quint8, ItemPropertyDisplay>::const_iterator iter = propsDisplayMap.constEnd();
    while (iter != propsDisplayMap.constBegin())
    {
        --iter;
        html += htmlStringFromDiabloColorString(iter.value().displayString, ColorsManager::NoColor) + kHtmlLineBreak;
    }
    return coloredText(html, textColor);
}

void PropertiesViewerWidget::renderHtml(QTextEdit *textEdit, const QString &description)
{
    textEdit->setText(kBaseFormat.arg(QString(description).replace('\n', kHtmlLineBreak)));
}

void PropertiesViewerWidget::removeAllMysticOrbs()
{
    removeMysticOrbsFromProperties(_itemMysticOrbs, &_item->props);
    removeMysticOrbsFromProperties(_rwMysticOrbs, &_item->rwProps);

    byteAlignBits();
    updateItem();
}

void PropertiesViewerWidget::removeMysticOrb()
{
    QAction *action = qobject_cast<QAction *>(sender());
    int moCode = action->property("moCode").toInt();
    PropertiesMultiMap *props = &(action->property("isItemMO").toBool() ? _item->props : _item->rwProps);

    int moNumber = props->value(moCode)->value; // must be queried before calling removeMysticOrbData()
    removeMysticOrbData(moCode, props);
    decreaseRequiredLevel(moNumber, props);
    byteAlignBits();

    MysticOrb *mo = ItemDataBase::MysticOrbs()->value(moCode);
    int id = mo->statIds.first();
    if (ItemProperty *prop = getProperty(id, mo->param, props))
        ItemParser::createDisplayStringForPropertyWithId(id, prop); // update displayString for some properties (like ctc)
    updateItem();
}

void PropertiesViewerWidget::removeMysticOrbsFromProperties(const QSet<int> &mysticOrbs, PropertiesMultiMap *props)
{
    int moNumber = 0;
    foreach (int moCode, mysticOrbs)
    {
        moNumber += props->value(moCode)->value;
        removeMysticOrbData(moCode, props);
    }
    decreaseRequiredLevel(moNumber, props);
}

void PropertiesViewerWidget::removeMysticOrbData(int moCode, PropertiesMultiMap *props)
{
    int moValue = totalMysticOrbValue(moCode, props);
    MysticOrb *mo = ItemDataBase::MysticOrbs()->value(moCode);
    foreach (quint16 statId, mo->statIds) //-V807
        modifyMysticOrbProperty(statId, moValue, props, mo->param);

    // remove MO data
    ItemPropertyTxt *propertyTxt = ItemDataBase::Properties()->value(moCode);
    int valueIndex = indexOfPropertyValue(moCode, props);
    if (valueIndex > -1)
        _item->bitString.remove(valueIndex, propertyTxt->bits + propertyTxt->saveParamBits + Enums::CharacterStats::StatCodeLength);

    delete props->take(moCode);
}

int PropertiesViewerWidget::indexOfPropertyValue(int id, const PropertiesMultiMap *const props, quint32 param /*= 0*/) const
{
    ItemProperty *prop = getProperty(id, param, props);
    if (!prop)
        return -1;

    bool isMaxEnhDamageProp = id == Enums::ItemProperties::EnhancedDamage;
    ItemPropertyTxt *property = ItemDataBase::Properties()->value(id);
    qulonglong value = prop->value + property->add;
    if (isMaxEnhDamageProp)
    {
        qulonglong oldValue = value;
        value <<= property->bits;
        value += oldValue;
    }

    int idIndex = -1, valueIndex = -1, bits = property->bits * (1 + isMaxEnhDamageProp);
    qulonglong valueInString = 0;
    quint32 paramInString = 0;
    do
    {
        if ((idIndex = _item->bitString.indexOf(binaryStringFromNumber(id, false, Enums::CharacterStats::StatCodeLength), idIndex + 1)) == -1)
            return -1;

        int paramIndex = idIndex - property->saveParamBits;
        valueIndex = paramIndex - bits;

        valueInString = _item->bitString.mid(valueIndex, bits).toULongLong(0, 2);
        if (param)
            paramInString = _item->bitString.mid(paramIndex, property->saveParamBits).toUInt(0, 2);
    } while (valueInString != value || paramInString != param);
    return valueIndex;
}

void PropertiesViewerWidget::modifyMysticOrbProperty(int id, int decrement, PropertiesMultiMap *props, quint32 param /*= 0*/)
{
    if (!decrement)
        return;

    int valueIndex = indexOfPropertyValue(id, props, param);
    if (valueIndex < 0)
    {
        bool fail = true;
        if (id == Enums::ItemProperties::MaximumDamage)
        {
            id = Enums::ItemProperties::MaximumDamageSecondary;
            fail = (valueIndex = indexOfPropertyValue(id, props, param)) < 0;
        }
        if (fail)
            return;
    }

    // ED value is stored as a sequence of 2 equal values
    bool isEnhancedDamageProp = id == Enums::ItemProperties::EnhancedDamage;
    ItemPropertyTxt *propertyTxt = ItemDataBase::Properties()->value(id);
    int bitsLength = (1 + isEnhancedDamageProp) * propertyTxt->bits;

    ItemProperty *prop = getProperty(id, param, props);
    if (!prop) // shouldn't be possible actually
        ERROR_BOX("WTF property not found. Gonna crash now, sorry...");

    prop->value -= decrement;
    if (prop->value > 0) // this value can become negative if removing 15x ED MO
    {
        QString newBits = binaryStringFromNumber(prop->value + propertyTxt->add, false, propertyTxt->bits); // it's not a mistake that I'm not using bitsLength here
        if (isEnhancedDamageProp)
        {
            newBits += newBits;
            prop->displayString = ItemParser::kEnhancedDamageFormat().arg(prop->value);
        }
        _item->bitString.replace(valueIndex, bitsLength, newBits); // place modified value
    }
    else
    {
        _item->bitString.remove(valueIndex, bitsLength + propertyTxt->saveParamBits + Enums::CharacterStats::StatCodeLength);
        props->remove(id, prop);
        delete prop;
    }
}

ItemProperty *PropertiesViewerWidget::getProperty(int id, quint32 param, const PropertiesMultiMap *const props) const
{
    // ctc MOs have same id but different params
    if (param)
    {
        foreach (ItemProperty *aProp, props->values(id))
            if (aProp->param == param)
                return aProp;
        return 0;
    }
    return props->value(id);
}

int PropertiesViewerWidget::totalMysticOrbValue(int moCode, PropertiesMap *props) const
{
    quint8 multiplier = 1 + isMysticOrbEffectDoubled();
    return props->value(moCode)->value * ItemDataBase::MysticOrbs()->value(moCode)->value * multiplier;
}

void PropertiesViewerWidget::byteAlignBits()
{
    int extraBits = _item->bitString.length() % 8;
    if (extraBits)
    {
        int zerosBeforeFirst1 = _item->bitString.indexOf('1'), zerosToAppend = 8 - extraBits;
        if (zerosBeforeFirst1 + zerosToAppend < 8)
            _item->bitString.prepend(QString(zerosToAppend, kZeroChar));
        else
            _item->bitString.remove(0, extraBits);
    }
}

QString PropertiesViewerWidget::collectMysticOrbsDataFromProps(QSet<int> *moSet, PropertiesMap &props)
{
    PropertiesMap propsWithoutMO = props;
    PropertiesMap::iterator iter = propsWithoutMO.begin();
    while (iter != propsWithoutMO.end())
    {
        if (ItemDataBase::MysticOrbs()->contains(iter.key()))
        {
            if (!ItemDataBase::isUberCharm(_item))
                moSet->insert(iter.key());
            iter = propsWithoutMO.erase(iter);
        }
        else
            ++iter;
    }
    QString html = propertiesToHtml(propsWithoutMO);

    if (!moSet->isEmpty())
    {
        html += htmlLine + kHtmlLineBreak;
        foreach (int moCode, *moSet)
        {
            QString &displayString = props.value(moCode)->displayString;
            if (isMysticOrbEffectDoubled() && !displayString.endsWith(" x 2"))
                displayString += " x 2";
            // quick & dirty hack with const_cast
            html += QString("%1 = %2").arg(displayString).arg(totalMysticOrbValue(moCode, &props)) + kHtmlLineBreak;
        }
    }
    return html;
}

bool PropertiesViewerWidget::isMysticOrbEffectDoubled() const
{
    return _item->props.contains(Enums::ItemProperties::MysticOrbsEffectDoubled) || _item->rwProps.contains(Enums::ItemProperties::MysticOrbsEffectDoubled) || _item->setProps.contains(Enums::ItemProperties::MysticOrbsEffectDoubled);
}

PropertiesMultiMap PropertiesViewerWidget::collectSetFixedProps(const QList<SetFixedProperty> &setProps, quint8 propsNumber /*= 0*/)
{
    PropertiesMultiMap setFixedProps;
    for (quint8 i = 0, n = propsNumber ? qMin(propsNumber, static_cast<quint8>(setProps.size())) : setProps.size(); i < n; ++i)
    {
        const SetFixedProperty &setProp = setProps.at(i);
        foreach (int propId, setProp.ids)
        {
            ItemProperty *prop = new ItemProperty(0, setProp.param);
            switch (propId)
            {
            case Enums::ItemProperties::MinimumDamageFire: case Enums::ItemProperties::MinimumDamageLightning: case Enums::ItemProperties::MinimumDamageMagic: case Enums::ItemProperties::MinimumDamageCold:
                prop->value = setProp.minValue;
                break;
            default:
                prop->value = setProp.maxValue;
                break;
            }
            if (propId == Enums::ItemProperties::MinimumDamageMagic || propId == Enums::ItemProperties::MaximumDamageMagic)
                ItemParser::convertParamsInMagicDamageString(prop, ItemDataBase::Properties()->value(propId));
            else
                ItemParser::createDisplayStringForPropertyWithId(propId, prop);
            setFixedProps.insert(propId, prop);
        }
    }
    return setFixedProps;
}
