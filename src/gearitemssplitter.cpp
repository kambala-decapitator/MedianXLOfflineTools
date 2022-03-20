#include "gearitemssplitter.h"
#include "itemstoragetableview.h"
#include "itemdatabase.h"
#include "characterinfo.hpp"

#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>


GearItemsSplitter::GearItemsSplitter(ItemStorageTableView *itemsView, QWidget *parent) : ItemsPropertiesSplitter(itemsView, parent), _button1(new QPushButton(this)), _button2(new QPushButton(this)),
    kButtonNames(QStringList() << tr("Character") << tr("Mercenary") << tr("Iron Golem")), _currentGearButtonNameIndex(CharacterNameIndex)
{
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(_button1);
    hlayout->addWidget(_button2);

    QVBoxLayout *vlayout = static_cast<QVBoxLayout *>(widget(0)->layout());
    vlayout->addLayout(hlayout);

    connect(_button1, SIGNAL(clicked()), SLOT(changeGear()));
    connect(_button2, SIGNAL(clicked()), SLOT(changeGear()));
}

void GearItemsSplitter::setItems(const ItemsList &newItems, bool isCreatingTab /*= false*/)
{
    _allItems = newItems;
    _gearItems[CharacterNameIndex]  = ItemDataBase::itemsStoredIn(Enums::ItemStorage::NotInStorage, Enums::ItemLocation::Equipped,  0, &_allItems);
    _gearItems[CharacterNameIndex] += ItemDataBase::itemsStoredIn(Enums::ItemStorage::NotInStorage, Enums::ItemLocation::Belt,      0, &_allItems);
    _gearItems[MercenaryNameIndex]  = ItemDataBase::itemsStoredIn(Enums::ItemStorage::NotInStorage, Enums::ItemLocation::Merc,      0, &_allItems);
    _gearItems[IronGolemNameIndex]  = ItemDataBase::itemsStoredIn(Enums::ItemStorage::NotInStorage, Enums::ItemLocation::IronGolem, 0, &_allItems);

    if (isCreatingTab)
    {
        changeButtonText(_button1, _currentGearButtonNameIndex == MercenaryNameIndex ? CharacterNameIndex : MercenaryNameIndex);
        if (_currentGearButtonNameIndex != MercenaryNameIndex)
            _button1->setEnabled(CharacterInfo::instance().mercenary.exists);

        changeButtonText(_button2, _currentGearButtonNameIndex == IronGolemNameIndex ? CharacterNameIndex : IronGolemNameIndex);
        if (_currentGearButtonNameIndex != IronGolemNameIndex)
            _button2->setDisabled(_gearItems[IronGolemNameIndex].isEmpty());
    }

    updateItemsForCurrentGear();
}

void GearItemsSplitter::removeItemFromList(ItemInfo *item, bool emitSignal)
{
    ItemsPropertiesSplitter::removeItemFromList(item, emitSignal);
    _gearItems[_currentGearButtonNameIndex].removeOne(item);
}

void GearItemsSplitter::changeGear()
{
    QPushButton *pressedButton = qobject_cast<QPushButton *>(sender());
    int pressedButtonNameIndex = kButtonNames.indexOf(pressedButton->text().left(pressedButton->text().lastIndexOf(' '))); // number of items goes after the space
    changeButtonText(pressedButton, _currentGearButtonNameIndex);
    _currentGearButtonNameIndex = static_cast<GearItemsSplitter::GearNameIndex>(pressedButtonNameIndex);

    updateItemsForCurrentGear();
}

void GearItemsSplitter::updateItemsForCurrentGear()
{
    const ItemsList &items = _gearItems[_currentGearButtonNameIndex];
    updateItems(items);
    emit itemCountChanged(items.size());
}

void GearItemsSplitter::changeButtonText(QPushButton *button, GearItemsSplitter::GearNameIndex nameIndex)
{
    button->setText(QString("%1 (%2)").arg(kButtonNames.at(nameIndex)).arg(_gearItems[nameIndex].size()));
}
