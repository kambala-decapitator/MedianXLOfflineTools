#include "propertiesviewerwidget.h"
#include "itemdatabase.h"
#include "enums.h"
#include "helpers.h"
#include "colors.hpp"
#include "itemparser.h"

#include <QDebug>
#include <QSettings>

static const QString baseFormat("<html><body bgcolor = \"black\"><table><tr><td align = \"center\" width = \"100%\"><font color = \"#ffffff\">%1</font></td></tr></table></body></html>");
static const QList<QByteArray> damageToUndeadTypes = QList<QByteArray>() << "mace" << "hamm" << "staf" << "scep" << "club" << "wand";


PropertiesViewerWidget::PropertiesViewerWidget(QWidget *parent) : QWidget(parent), _item(0)
{
    ui.setupUi(this);

    connect(ui.removeAllMysticOrbsPushButton, SIGNAL(clicked()), SLOT(removeAllMysticOrbs()));
	connect(ui.tabWidget, SIGNAL(currentChanged(int)), SLOT(currentItemTabChanged(int)));
}

void PropertiesViewerWidget::displayItemProperties(ItemInfo *item)
{
    for (int i = 1; i < ui.tabWidget->count(); ++i)
        ui.tabWidget->setTabEnabled(i, false);

    ui.allTextEdit->clear();
    if (!(_item = item))
        return;

    QMap<int, ItemProperty> allProps = item->props;
    if (item->isRW)
    {
        ui.tabWidget->setTabEnabled(2, true);
        setProperties(ui.runewordTextEdit, item->rwProps);
        renderItemDescription(ui.runewordTextEdit);
        addProperties(&allProps, item->rwProps);
    }

    ui.mysticOrbsTextEdit->clear();
    _mysticOrbs.clear();

    bool isClassCharm = QString(ItemDataBase::Items()->value(_item->itemType).typeString).startsWith("ara");
    QMap<int, ItemProperty>::iterator iter = allProps.begin();
    while (iter != allProps.end())
    {
        if (ItemDataBase::MysticOrbs()->contains(iter.key()))
        {
            if (!isClassCharm)
            {
                ui.tabWidget->setTabEnabled(1, true);
                ui.mysticOrbsTextEdit->append(QString("%1 = %2").arg(iter.value().displayString).arg(totalMysticOrbValue(iter.key())));
                _mysticOrbs += iter.key();
                iter = allProps.erase(iter);
            }
            else
            {
                QString &desc = iter.value().displayString;
                switch (iter.key())
                {
                case 313:
                    desc = tr("They have Windows in Hell");
                    break;
                case 314:
                    desc = tr("Mirror Mirror");
                    break;
                case 400:
                    desc = tr("Countess");
                    break;
                case 401:
                    desc = tr("LC2");
                    break;
                case 403:
                    desc = tr("Crowned");
                    break;
                default:
                    desc = tr("Challenge with id %1").arg(iter.key());
                    break;
                }
                ++iter;
            }
        }
        else
            ++iter;
    }
    renderItemDescription(ui.mysticOrbsTextEdit);
    ui.removeAllMysticOrbsPushButton->setEnabled(_mysticOrbs.size() > 0);

    const ItemBase &itemBase = ItemDataBase::Items()->value(item->itemType);
    ui.socketablesTextEdit->clear();
    if (item->socketablesInfo.size())
    {
        ui.tabWidget->setTabEnabled(3, true);
        foreach (ItemInfo *socketableItem, item->socketablesInfo)
        {
            ui.socketablesTextEdit->append(ItemDataBase::completeItemName(socketableItem, true));

            QMap<int, ItemProperty> props;
            if (ItemDataBase::Socketables()->contains(socketableItem->itemType)) // it's a gem or a rune
            {
                const SocketableItemInfo &socketableItemInfo = ItemDataBase::Socketables()->value(socketableItem->itemType);
                int index = itemBase.socketableType + 1;
                const QList<SocketableItemInfo::Properties> &socketableProps = socketableItemInfo.properties[static_cast<SocketableItemInfo::PropertyType>(index)];
                foreach (const SocketableItemInfo::Properties &prop, socketableProps)
                {
                    if (prop.code == 17) // ED
                        props[prop.code].displayString = tr("+%1% Enhanced Damage").arg(prop.value);
                    if (prop.code != -1)
                        props[prop.code] = ItemProperty(prop.value, prop.param);
                }
            }
            else // it's a jewel
                props = socketableItem->props;
            setProperties(ui.socketablesTextEdit, props, false);
            addProperties(&allProps, props);

            ui.socketablesTextEdit->append(htmlStringFromDiabloColorString("----------"));
        }
        ui.socketablesTextEdit->undo(); // remove last separator
        renderItemDescription(ui.socketablesTextEdit);
    }

    // create full item description
    QString itemDescription = ItemDataBase::completeItemName(item, true) + colorReplacementString(White) + "<br>" + tr("Item Level: %1").arg(item->ilvl);
    if (item->isRW)
    {
        QString runes;
        foreach (ItemInfo *socketable, item->socketablesInfo)
            if (ItemDataBase::Items()->value(socketable->itemType).typeString == "rune")
                runes += ItemDataBase::Socketables()->value(socketable->itemType).letter;
        if (!runes.isEmpty()) // gem-/jewelwords don't have any letters
            itemDescription += "<br>" + htmlStringFromDiabloColorString(QString("'%1'").arg(runes), Gold) + colorReplacementString(White);
    }

    if (itemBase.genericType == Enums::ItemTypeGeneric::Armor)
    {
        int baseDef = item->defense, totalDef = baseDef;
		ItemProperty foo;
		int ed = allProps.value(Enums::ItemProperties::EnhancedDefence, foo).value + (allProps.value(Enums::ItemProperties::EnhancedDefenceBoCL, foo).value * *ItemDataBase::clvl) / 32;
		if (ed)
			totalDef = (totalDef * (100 + ed)) / 100;
		totalDef += allProps.value(Enums::ItemProperties::Defence, foo).value + (allProps.value(Enums::ItemProperties::DefenceBoCL, foo).value * *ItemDataBase::clvl) / 32;
        if (totalDef < 0)
            totalDef = 0;

        QString defString = "<br>" + tr("Defense: %1");
        if (baseDef != totalDef)
            itemDescription += defString.arg(htmlStringFromDiabloColorString(QString::number(totalDef), Blue)) + colorReplacementString(White) + QString(" (%1)").arg(baseDef);
        else
            itemDescription += defString.arg(baseDef);
    }
    if (itemBase.genericType != Enums::ItemTypeGeneric::Misc && item->maxDurability)
    {
        itemDescription += "<br>" + tr("Durability: ", "don't forget the trailing space!");
        bool isIndestructible = allProps.value(Enums::ItemProperties::Indestructible).value == 1;
        if (isIndestructible)
            itemDescription += QString("%1 [").arg(QChar(0x221e)); // infinity
        itemDescription += tr("%1 of %2", "durability").arg(item->currentDurability).arg(item->maxDurability);
        if (isIndestructible)
            itemDescription += "]";
    }
    if (itemBase.isStackable)
        itemDescription += "<br>" + tr("Quantity: %1").arg(item->quantity);
    if (itemBase.classCode > -1)
	{
		itemDescription += "<br>";
		QString text = tr("(%1 Only)", "class-specific item").arg(Enums::ClassName::classes().at(itemBase.classCode));
		if (itemBase.classCode != *ItemDataBase::charClass)
			itemDescription += htmlStringFromDiabloColorString(text, Red) + colorReplacementString(White);
		else
			itemDescription += text;
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
//    case Enums::ItemQuality::Rare: case Enums::ItemQuality::Crafted: case Enums::ItemQuality::Magic:
        // TODO 0.3: add support for affix rlvl in some next version
//        break;
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
        itemDescription += "<br>" + tr("Required Level: %1").arg(actualRlvl > 555 ? 555 : actualRlvl);

    // add '+50% damage to undead' if item type matches
    bool shouldAddDamageToUndeadInTheBottom = false;
    if (ItemParser::itemTypesInheritFromTypes(QList<QByteArray>() << itemBase.typeString, damageToUndeadTypes))
    {
        if (allProps.contains(Enums::ItemProperties::DamageToUndead))
            allProps[Enums::ItemProperties::DamageToUndead].value += 50;
        else
            shouldAddDamageToUndeadInTheBottom = true;
    }

    if (allProps.size())
    {
        setProperties(ui.allTextEdit, allProps, false);
        itemDescription += "<br>" + ui.allTextEdit->toPlainText();
    }
    if (shouldAddDamageToUndeadInTheBottom)
        itemDescription += "<br>" + tr("+50% Damage to Undead");

    if (item->isSocketed)
        itemDescription += "<br>" + htmlStringFromDiabloColorString(tr("Socketed: (%1), Inserted: (%2)").arg(item->socketsNumber).arg(item->socketablesNumber), Blue);
	if (item->isEthereal)
		itemDescription += "<br>" + htmlStringFromDiabloColorString(tr("Ethereal (Cannot be Repaired)"), Blue);

    // show existing set items from current set
    if (item->quality == Enums::ItemQuality::Set)
    {
        const QString &setName = ItemDataBase::Sets()->value(item->setOrUniqueId).setName;
        itemDescription += "<br><br>" + htmlStringFromDiabloColorString(setName, Gold);

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
            itemDescription += QString("<br>%1").arg(htmlStringFromDiabloColorString(setItemName, found ? Green : Red));
        }
    }

    renderItemDescription(ui.allTextEdit, &itemDescription);
}

void PropertiesViewerWidget::setProperties(QTextEdit *textEdit, const QMap<int, ItemProperty> &properties, bool shouldClearText)
{
    if (shouldClearText)
        textEdit->clear();

    QMap<quint8, ItemPropertyDisplay> propsDisplayMap;
    for (QMap<int, ItemProperty>::const_iterator iter = properties.constBegin(); iter != properties.constEnd(); ++iter)
    {
        const ItemProperty &prop = iter.value();
        int propId = iter.key();
        if (propId == 23 && properties.contains(21) || propId == 24 && properties.contains(22)) // don't include secondary_(min/max)damage
            continue;

        QString displayString = prop.displayString.isEmpty() ? propertyDisplay(prop, propId) : prop.displayString;
        if (!displayString.isEmpty())
            propsDisplayMap.insertMulti(ItemDataBase::Properties()->value(propId).descPriority,
                                        ItemPropertyDisplay(displayString, ItemDataBase::Properties()->value(propId).descPriority, propId));
    }

    // group properties
    for (QMap<quint8, ItemPropertyDisplay>::iterator iter = propsDisplayMap.begin(); iter != propsDisplayMap.end(); ++iter)
    {
        ItemPropertyDisplay &itemPropDisplay = iter.value();
        const ItemPropertyTxt &itemPropertyTxt = ItemDataBase::Properties()->value(itemPropDisplay.propertyId);
        if (itemPropertyTxt.groupIDs.size())
        {
            QList<quint16> availableGroupIDs;
            int propValue = properties[itemPropDisplay.propertyId].value;
            for (QMap<quint8, ItemPropertyDisplay>::const_iterator  jter = propsDisplayMap.constBegin(); jter != propsDisplayMap.constEnd(); ++jter)
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

                QMap<quint8, ItemPropertyDisplay>::iterator  jter = propsDisplayMap.begin();
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

    QMap<quint8, ItemPropertyDisplay>::const_iterator  iter = propsDisplayMap.constEnd();
    while (iter != propsDisplayMap.constBegin())
    {
        --iter;
        textEdit->append(htmlStringFromDiabloColorString(iter.value().displayString, Blue));
    }
}

void PropertiesViewerWidget::renderItemDescription(QTextEdit *textEdit, QString *description)
{
    textEdit->setText(baseFormat.arg((description ? *description : textEdit->toPlainText()).replace('\n', "<br>")));
}

void PropertiesViewerWidget::removeAllMysticOrbs()
{
    int moNumber = 0;
    foreach (int moCode, _mysticOrbs)
    {
        modifyMysticOrbProperty(ItemDataBase::MysticOrbs()->value(moCode).statId, totalMysticOrbValue(moCode));

        // remove MO data
        const ItemPropertyTxt &property = ItemDataBase::Properties()->value(moCode);
        int valueIndex = indexOfPropertyValue(moCode);
        if (valueIndex > -1)
			_item->bitString.remove(valueIndex, property.bits + property.saveParamBits + Enums::CharacterStats::StatCodeLength);
		
		moNumber += _item->props.value(moCode).value;
        _item->props.remove(moCode);
    }

    modifyMysticOrbProperty(Enums::ItemProperties::RequiredLevel, moNumber * 2); // decrease rlvl
    qDebug("---------");

    // align
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
    displayItemProperties(_item);
}

int PropertiesViewerWidget::indexOfPropertyValue(int id)
{
    bool isMaxEnhDamageProp = id == 17;
    const ItemPropertyTxt &property = ItemDataBase::Properties()->value(id);
    qulonglong value = _item->props.value(id).value + property.add;
    qDebug() << "searching for property" << id << property.descPositive << "with value" << value << "actual value" << value - property.add;
    if (isMaxEnhDamageProp)
    {
        qulonglong oldValue = value;
        value <<= property.bits;
        qDebug() << "value after shift" << value;
        value += oldValue;
        qDebug() << "value after add" << value;
    }
    int idIndex = -1, valueIndex = -1, bits = property.bits * (1 + isMaxEnhDamageProp);
    do
    {
        if ((idIndex = _item->bitString.indexOf(binaryStringFromNumber(id, false, Enums::CharacterStats::StatCodeLength), idIndex + 1)) == -1)
            break;
        valueIndex = idIndex - property.saveParamBits - bits;
//    } while (_item->bitString.mid(idIndex - property.bits, property.bits) != binaryStringFromNumber(value + property.add, false, property.bits));
    } while (_item->bitString.mid(valueIndex, bits).toULongLong(0, 2) != value);
    if (idIndex == -1)
        qDebug("not found");
    return valueIndex;
}

void PropertiesViewerWidget::modifyMysticOrbProperty(int id, int decrement)
{
    int valueIndex = indexOfPropertyValue(id);
    if (valueIndex <= -1)
        return;

    const ItemPropertyTxt &propertyTxt = ItemDataBase::Properties()->value(id);
	ItemProperty prop = _item->props.value(id);
    prop.value -= decrement;
    if (prop.value)
	{
        _item->bitString.replace(valueIndex, propertyTxt.bits, binaryStringFromNumber(prop.value + propertyTxt.add, false, propertyTxt.bits)); // place modified value
		_item->props.replace(id, prop);
	}
    else
    {
        _item->props.remove(id);
        _item->bitString.remove(valueIndex, propertyTxt.bits + propertyTxt.saveParamBits + Enums::CharacterStats::StatCodeLength);
    }
}

int PropertiesViewerWidget::totalMysticOrbValue(int moCode) const
{
	quint8 multiplier = 1 + (_item->quality == Enums::ItemQuality::Crafted || _item->quality == Enums::ItemQuality::Honorific);
	return _item->props.value(moCode).value * ItemDataBase::MysticOrbs()->value(moCode).value * multiplier;
}

QString PropertiesViewerWidget::propertyDisplay(const ItemProperty &propDisplay, int propId)
{
    // TODO: maybe add indication of trophy/bless if propId == 219
    int value = propDisplay.value;
	if (!value)
		return QString();

    const ItemPropertyTxt &prop = ItemDataBase::Properties()->value(propId);
    QString description = value < 0 ? prop.descNegative : prop.descPositive, result;
    if (prop.descStringAdd.contains(tr("Based on Character Level")))
        value = (value * *ItemDataBase::clvl) / 32;

    char valueStringSigned[10];
    ::sprintf(valueStringSigned, "%+d", value);

    switch (prop.descFunc)
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
        result = QString(QString(prop.descVal == 1 ? "%1% %2" : "%2 %1%") + " %3").arg(prop.descFunc == 7 ? QString::number(value) : valueStringSigned)
                 .arg(description).arg(prop.descStringAdd); // 1 or 2
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
        result = QString("%1% %2 %3").arg(value).arg(description).arg(ItemDataBase::Monsters()->value(propDisplay.param));
        break;
    // 9, 10, 14, 16-19 - absent
    // everything else is constructed in parseItemProperties() (has displayString)
    default:
        result = tr("[special case %4, please report] %1 '%2' (ID %3)").arg(value).arg(description).arg(propId).arg(prop.descFunc);
    }
    return result;
}

void PropertiesViewerWidget::addProperties(QMap<int, ItemProperty> *mutableProps, const QMap<int, ItemProperty> &propsToAdd)
{
    for (QMap<int, ItemProperty>::const_iterator iter = propsToAdd.constBegin(); iter != propsToAdd.constEnd(); ++iter)
    {
        if (mutableProps->contains(iter.key()))
            (*mutableProps)[iter.key()].value += iter.value().value;
        else
            mutableProps->insert(iter.key(), iter.value());
    }
}

void PropertiesViewerWidget::currentItemTabChanged(int index)
{
	for (int i = 0; i < ui.tabWidget->count(); ++i)
		ui.tabWidget->setTabIcon(i, QIcon(QString(":/PropertiesViewerWidget/Resources/icons/arrow_%1").arg(i == index ? "down" : "right")));
}
