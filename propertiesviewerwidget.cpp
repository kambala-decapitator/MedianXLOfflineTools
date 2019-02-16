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


static const QString kBaseFormat("<html><body bgcolor=\"black\" align=\"center\" style=\"color: white\">%1</body></html>");
static const char *kTranslationContext = "PropertiesDisplayManager";


PropertiesViewerWidget::PropertiesViewerWidget(QWidget *parent) : QWidget(parent), htmlLine("<hr />"), ui(new Ui::PropertiesViewerWidget), _item(0)
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

    QString ilvlText = kHtmlLineBreak + qApp->translate(kTranslationContext, "Item Level: %1").arg(item->ilvl);
    if (item->isEar)
    {
        QString itemDescription = qApp->translate(kTranslationContext, "%1's Ear", "param is character name").arg(item->earInfo.name.constData()) + kHtmlLineBreak;
        itemDescription += ClassName::classes().at(item->earInfo.classCode) + kHtmlLineBreak;
        itemDescription += qApp->translate(kTranslationContext, "Level %1").arg(item->earInfo.level);
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
            html += ItemDataBase::completeItemName(socketableItem, true) + kHtmlLineBreak;
            if (socketableItem->itemType == ItemDataBase::kJewelType)
                html += qApp->translate(kTranslationContext, "Item Level: %1").arg(socketableItem->ilvl) + kHtmlLineBreak;
            html += propertiesToHtml(socketableProps) + htmlLine;

            if (socketableProps != socketableItem->props)
                qDeleteAll(socketableProps);
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
        QList<QByteArray> baseTypes = ItemDataBase::Items()->value(socketable->itemType)->types;
        if (baseTypes.first() == "rune" || baseTypes.last() == "xrun")
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
        if (int ed = allProps.value(ItemProperties::EnhancedDefence, foo)->value + (allProps.value(ItemProperties::EnhancedDefenceBasedOnClvl, foo)->value * clvl) / 32)
            totalDef = (totalDef * (100 + ed)) / 100;
        totalDef += allProps.value(ItemProperties::Defence, foo)->value + (allProps.value(ItemProperties::DefenceBasedOnClvl, foo)->value * clvl) / 32;
        if (totalDef < 0)
            totalDef = 0;

        QString defString = qApp->translate(kTranslationContext, "Defense: %1");
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
        QString damageFormat = qApp->translate(kTranslationContext, "%1 to %2", "min-max damage");

        // if min == max, then increment max by 1
        if (itemBase->minThrowDmg && itemBase->maxThrowDmg)
        {
            int minDmg = itemBase->minThrowDmg, maxDmg = itemBase->maxThrowDmg;
            if (item->isEthereal)
            {
                minDmg *= 1.5;
                maxDmg *= 1.5;
            }
            if (ed)
            {
                minDmg = (minDmg * (100 + ed)) / 100;
                maxDmg = (maxDmg * (100 + ed)) / 100;
            }
            minDmg += minDmgTotal;
            maxDmg += maxDmgTotal;

            itemDescription += qApp->translate(kTranslationContext, "Throw Damage") + ": " + htmlStringFromDiabloColorString(damageFormat.arg(minDmg).arg(maxDmg + (minDmg == maxDmg)), damageColor) + kHtmlLineBreak;
        }

        if (itemBase->min1hDmg && itemBase->max1hDmg)
        {
            int minDmg = itemBase->min1hDmg, maxDmg = itemBase->max1hDmg;
            if (item->isEthereal)
            {
                minDmg *= 1.5;
                maxDmg *= 1.5;
            }
            if (ed)
            {
                minDmg = (minDmg * (100 + ed)) / 100;
                maxDmg = (maxDmg * (100 + ed)) / 100;
            }
            minDmg += minDmgTotal;
            maxDmg += maxDmgTotal;

            itemDescription += qApp->translate(kTranslationContext, "One-Hand Damage") + ": " + htmlStringFromDiabloColorString(damageFormat.arg(minDmg).arg(maxDmg + (minDmg == maxDmg)), damageColor) + kHtmlLineBreak;
        }

        if (itemBase->min2hDmg && itemBase->max2hDmg)
        {
            int minDmg = itemBase->min2hDmg, maxDmg = itemBase->max2hDmg;
            if (item->isEthereal)
            {
                minDmg *= 1.5;
                maxDmg *= 1.5;
            }
            if (ed)
            {
                minDmg = (minDmg * (100 + ed)) / 100;
                maxDmg = (maxDmg * (100 + ed)) / 100;
            }
            minDmg += minDmgTotal;
            maxDmg += maxDmgTotal;

            itemDescription += qApp->translate(kTranslationContext, "Two-Hand Damage") + ": " + htmlStringFromDiabloColorString(damageFormat.arg(minDmg).arg(maxDmg + (minDmg == maxDmg)), damageColor) + kHtmlLineBreak;
        }
    }
    if (itemBase->genericType != ItemTypeGeneric::Misc && item->maxDurability)
    {
        itemDescription += qApp->translate(kTranslationContext, "Durability") + ": ";
        bool isIndestructible = allProps.value(ItemProperties::Indestructible, foo)->value == 1;
        if (isIndestructible)
            itemDescription += QString("%1 [").arg(QChar(0x221e)); // infinity
        itemDescription += qApp->translate(kTranslationContext, "%1 of %2", "durability").arg(item->currentDurability).arg(item->maxDurability);
        if (isIndestructible)
            itemDescription += "]";
        itemDescription += kHtmlLineBreak;
    }
    if (itemBase->isStackable)
        itemDescription += qApp->translate(kTranslationContext, "Quantity: %1").arg(item->quantity) + kHtmlLineBreak;
    if (itemBase->classCode > -1)
    {
        QString text = qApp->translate(kTranslationContext, "(%1 Only)", "class-specific item").arg(ClassName::classes().at(itemBase->classCode));
        if (itemBase->classCode != charBasicInfo.classCode)
            itemDescription += htmlStringFromDiabloColorString(text, ColorsManager::Red);
        else
            itemDescription += text;
        itemDescription += kHtmlLineBreak;
    }

    // TODO: [later] use lambda to calculate requirements
    if (itemBase->rdex)
        if (quint16 rdex = itemBase->rdex + (itemBase->rdex * allProps.value(ItemProperties::Requirements, foo)->value) / 100)
            itemDescription += htmlStringFromDiabloColorString(qApp->translate(kTranslationContext, "Required Dexterity: %1").arg(rdex), charInfo.valueOfStatistic(CharacterStats::Dexterity) < rdex ? ColorsManager::Red : ColorsManager::White)
                               + kHtmlLineBreak;

    if (itemBase->rstr)
        if (quint16 rstr = itemBase->rstr + (itemBase->rstr * allProps.value(ItemProperties::Requirements, foo)->value) / 100)
            itemDescription += htmlStringFromDiabloColorString(qApp->translate(kTranslationContext, "Required Strength: %1").arg(rstr),  charInfo.valueOfStatistic(CharacterStats::Strength)  < rstr ? ColorsManager::Red : ColorsManager::White)
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
        int socketableRlvl = socketableItem->quality == ItemQuality::Unique ? ItemDataBase::Uniques()->value(socketableItem->setOrUniqueId)->rlvl : ItemDataBase::Items()->value(socketableItem->itemType)->rlvl;
        if (maxSocketableRlvl < socketableRlvl)
            maxSocketableRlvl = socketableRlvl;
    }
    if (int actualRlvl = qMax(rlvl, maxSocketableRlvl) + allProps.value(ItemProperties::RequiredLevel, foo)->value)
        itemDescription += htmlStringFromDiabloColorString(qApp->translate(kTranslationContext, "Required Level: %1").arg(actualRlvl), clvl < actualRlvl ? ColorsManager::Red : ColorsManager::White) + kHtmlLineBreak;
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
            itemDescription += htmlStringFromDiabloColorString(qApp->translate(kTranslationContext, "[Unidentified]"), ColorsManager::Red) + kHtmlLineBreak;
        itemDescription += propertiesToHtml(allProps);
    }
    else if (ItemDataBase::isGenericSocketable(item))
    {
        static const QStringList gearNames = QStringList() << qApp->translate(kTranslationContext, "Armor") << qApp->translate(kTranslationContext, "Shield") << qApp->translate(kTranslationContext, "Weapon");
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
        itemDescription += htmlStringFromDiabloColorString(qApp->translate(kTranslationContext, "+50% Damage to Undead"), ColorsManager::Blue) + kHtmlLineBreak;
    if (item->isSocketed)
        itemDescription += htmlStringFromDiabloColorString(qApp->translate(kTranslationContext, "Socketed: (%1), Inserted: (%2)").arg(item->socketsNumber).arg(item->socketablesNumber), ColorsManager::Blue) + kHtmlLineBreak;
    if (item->isEthereal)
        itemDescription += htmlStringFromDiabloColorString(qApp->translate(kTranslationContext, "Ethereal (Cannot be Repaired)"), ColorsManager::Blue) + kHtmlLineBreak;

    if (item->quality == ItemQuality::Set)
    {
        SetItemInfo *setItem = ItemDataBase::Sets()->value(item->setOrUniqueId);
        const FullSetInfo fullSetInfo = ItemDataBase::fullSetInfoForKey(setItem->key);

        if (item->location == ItemLocation::Equipped || item->location == ItemLocation::Corpse || item->location == ItemLocation::Merc)
        {
            // set item properties stored in item (seems that they're not needed)
            //if (!item->setProps.isEmpty())
            //    itemDescription += propertiesToHtml(item->setProps, ColorsManager::Green);

            // count equipped set items
            quint8 setItemsOnCharacter = 1;
            foreach (ItemInfo *anItem, ItemDataBase::itemsStoredIn(item->storage, item->location))
                if (anItem != item && anItem->quality == ItemQuality::Set && fullSetInfo.itemNames.contains(ItemDataBase::Sets()->value(anItem->setOrUniqueId)->itemName))
                    ++setItemsOnCharacter;
            
            if (quint8 partialPropsNumber = (setItemsOnCharacter - 1) * 2)
            {
                // set item properties from txt
                PropertiesMultiMap setItemFixedProps = PropertiesDisplayManager::collectSetFixedProps(setItem->fixedProperties, partialPropsNumber);
                if (!setItemFixedProps.isEmpty())
                    itemDescription += propertiesToHtml(setItemFixedProps, ColorsManager::Green);
                qDeleteAll(setItemFixedProps);

                // set properties from txt
                PropertiesMultiMap setFixedProps = PropertiesDisplayManager::collectSetFixedProps(fullSetInfo.partialSetProperties, partialPropsNumber);
                if (setItemsOnCharacter == fullSetInfo.itemNames.size())
                    PropertiesDisplayManager::addTemporaryPropertiesAndDelete(&setFixedProps, PropertiesDisplayManager::collectSetFixedProps(fullSetInfo.fullSetProperties));
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
    return PropertiesDisplayManager::propertiesToHtml(properties, _item, textColor);
}

void PropertiesViewerWidget::renderHtml(QTextEdit *textEdit, const QString &description)
{
    textEdit->setText(kBaseFormat.arg(QString(description).replace('\n', kHtmlLineBreak)));
}

void PropertiesViewerWidget::removeAllMysticOrbs()
{
    removeMysticOrbsFromProperties(_itemMysticOrbs, &_item->props);
    removeMysticOrbsFromProperties(_rwMysticOrbs, &_item->rwProps);

    ReverseBitWriter::byteAlignBits(_item->bitString);
    updateItem();
}

void PropertiesViewerWidget::removeMysticOrb()
{
    QAction *action = qobject_cast<QAction *>(sender());
    int moCode = action->property("moCode").toInt();
    PropertiesMultiMap *props = &(action->property("isItemMO").toBool() ? _item->props : _item->rwProps);

    MysticOrb *mo = ItemDataBase::MysticOrbs()->value(moCode);
    int moNumber = props->value(moCode)->value; // must be queried before calling removeMysticOrbData()
    removeMysticOrbData(moCode, props);
    decreaseRequiredLevel(moNumber * mo->rlvl, props);
    ReverseBitWriter::byteAlignBits(_item->bitString);

    int id = mo->statIds.first();
    if (ItemProperty *prop = getProperty(id, mo->param, props))
        ItemParser::createDisplayStringForPropertyWithId(id, prop); // update displayString for some properties (like ctc)
    updateItem();
}

void PropertiesViewerWidget::removeMysticOrbsFromProperties(const QSet<int> &mysticOrbs, PropertiesMultiMap *props)
{
    int rlvlDecrease = 0;
    foreach (int moCode, mysticOrbs)
    {
        rlvlDecrease += props->value(moCode)->value * ItemDataBase::MysticOrbs()->value(moCode)->rlvl;
        removeMysticOrbData(moCode, props);
    }
    decreaseRequiredLevel(rlvlDecrease, props);
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
        _item->bitString.remove(valueIndex, propertyTxt->bits + propertyTxt->paramBits + Enums::CharacterStats::StatCodeLength);

    delete props->take(moCode);
}

int PropertiesViewerWidget::indexOfPropertyValue(int id, const PropertiesMultiMap *const props, quint32 param /*= 0*/) const
{
    ItemProperty *prop = getProperty(id, param, props);
    if (!prop)
        return -1;

    ItemPropertyTxt *property = ItemDataBase::Properties()->value(id);
    qulonglong value = prop->value + property->add;
    int bits = property->bits;
    if (id == Enums::ItemProperties::EnhancedDamage)
    {
        qulonglong oldValue = value;
        value <<= property->bits;
        value += oldValue;

        bits *= 2;
    }

    int idIndex = -1, valueIndex = -1;
    qulonglong valueInString = 0;
    quint32 paramInString = 0;
    do
    {
        if ((idIndex = _item->bitString.indexOf(binaryStringFromNumber(id, false, Enums::CharacterStats::StatCodeLength), idIndex + 1)) == -1)
            return -1;

        int paramIndex = idIndex - property->paramBits;
        valueIndex = paramIndex - bits;

        valueInString = _item->bitString.mid(valueIndex, bits).toULongLong(0, 2);
        if (param)
            paramInString = _item->bitString.mid(paramIndex, property->paramBits).toUInt(0, 2);
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
        _item->bitString.remove(valueIndex, bitsLength + propertyTxt->paramBits + Enums::CharacterStats::StatCodeLength);
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

void PropertiesViewerWidget::decreaseRequiredLevel(int decrement, PropertiesMultiMap *props)
{
    modifyMysticOrbProperty(Enums::ItemProperties::RequiredLevel, decrement, props);
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
    return _item->props.contains(Enums::ItemProperties::MysticOrbsEffectDoubled) || _item->rwProps.contains(Enums::ItemProperties::MysticOrbsEffectDoubled);// || _item->setProps.contains(Enums::ItemProperties::MysticOrbsEffectDoubled);
}
