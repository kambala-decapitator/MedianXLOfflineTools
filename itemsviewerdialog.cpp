#include "itemsviewerdialog.h"
#include "itemspropertiessplitter.h"
#include "plugyitemssplitter.h"
#include "gearitemssplitter.h"
#include "itemstoragetableview.h"
#include "itemstoragetablemodel.h"
#include "itemdatabase.h"
#include "propertiesviewerwidget.h"
#include "itemparser.h"
#include "characterinfo.hpp"

#include <QTabWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QCloseEvent>
#include <QGroupBox>
#include <QPushButton>
#include <QCheckBox>

#include <QSettings>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


const int ItemsViewerDialog::kCellSize = 32;
const QList<int> ItemsViewerDialog::kRows = QList<int>() << 11 << 6 << 8 << 10 << 10 << 10 << 10;

ItemsViewerDialog::ItemsViewerDialog(const QHash<int, bool> &plugyStashesExistenceHash, QWidget *parent) : QDialog(parent), _tabWidget(new QTabWidget(this)), _itemManagementBox(new QGroupBox(tr("Item management"), this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(_tabWidget);
    mainLayout->addWidget(_itemManagementBox);

    // tabwidget setup
    for (int i = GearIndex; i <= LastIndex; ++i)
    {
        ItemsPropertiesSplitter *splitter;
        ItemStorageTableView *tableView = new ItemStorageTableView(this);
        if (i == GearIndex)
            splitter = new GearItemsSplitter(tableView, this);
        else if (i < PersonalStashIndex)
            splitter = new ItemsPropertiesSplitter(tableView, this);
        else
            splitter = new PlugyItemsSplitter(tableView, this);
        splitter->setModel(new ItemStorageTableModel(i == InventoryIndex && isUltimative5OrLater() ? 8 : kRows.at(i), i == GearIndex ? 8 : 10, splitter));
        _tabWidget->addTab(splitter, tabNameAtIndex(i));

        connect(splitter, SIGNAL(itemCountChanged(int)), SLOT(itemCountChangedInCurrentTab(int)));
        connect(splitter, SIGNAL(itemDeleted()), SLOT(decreaseItemCount()));
        connect(splitter, SIGNAL(itemsChanged(bool)), SIGNAL(itemsChanged(bool)));
        connect(splitter, SIGNAL(cubeDeleted(bool)), SIGNAL(cubeDeleted(bool)));
        connect(splitter, SIGNAL(cubeDeleted(bool)), SLOT(setCubeTabDisabled(bool)));
        if (isPlugyStorageIndex(i))
            connect(static_cast<PlugyItemsSplitter *>(splitter), SIGNAL(pageChanged()), SLOT(updateButtonsState()));
    }
    updateItems(plugyStashesExistenceHash);

    for (int i = GearIndex; i <= LastIndex; ++i)
    {
        ItemsPropertiesSplitter *splitter = splitterAtIndex(i);
        QAbstractTableModel *model = splitter->itemsModel();
        QTableView *tableView = splitter->itemsView();
        for (int j = 0; j < model->rowCount(); ++j)
            tableView->setRowHeight(j, kCellSize);
        for (int j = 0; j < model->columnCount(); ++j)
            tableView->setColumnWidth(j, kCellSize);
    }

    // item management groupbox setup
    _itemManagementBox->setDisabled(true);

    // disenchant box setup
    _disenchantBox = new QGroupBox(tr("Disenchant items here to:"), _itemManagementBox);

    _disenchantToShardsButton = new QPushButton(tr("Arcane Shards"), _disenchantBox);
    _upgradeToCrystalsCheckbox = new QCheckBox(tr("Upgrade to Crystals"), _disenchantBox);
    _uniquesCheckbox = new QCheckBox(tr("Uniques"), _disenchantBox);
    _upgradeToCrystalsCheckbox->setChecked(true);
    _uniquesCheckbox->setChecked(true);

    _disenchantToSignetButton = new QPushButton(tr("Signets of Learning"), _disenchantBox);
    _eatSignetsCheckbox = new QCheckBox(tr("Eat Signets"), _disenchantBox);
    _setsCheckbox = new QCheckBox(tr("Sets"), _disenchantBox);
    _eatSignetsCheckbox->setChecked(true);
    _setsCheckbox->setChecked(true);

    connect(_disenchantToShardsButton, SIGNAL(clicked()), SLOT(disenchantAllItems()));
    connect(_disenchantToSignetButton, SIGNAL(clicked()), SLOT(disenchantAllItems()));
    connect(_uniquesCheckbox, SIGNAL(toggled(bool)), SLOT(updateButtonsState()));
    connect(   _setsCheckbox, SIGNAL(toggled(bool)), SLOT(updateButtonsState()));

    QGridLayout *disenchantGridLayout = new QGridLayout(_disenchantBox);
    disenchantGridLayout->addWidget(_disenchantToShardsButton, 0, 0);
    disenchantGridLayout->addWidget(_upgradeToCrystalsCheckbox, 1, 0, Qt::AlignCenter);
    disenchantGridLayout->addWidget(_uniquesCheckbox, 2, 0, Qt::AlignCenter);
    disenchantGridLayout->addWidget(_disenchantToSignetButton, 0, 1);
    disenchantGridLayout->addWidget(_eatSignetsCheckbox, 1, 1, Qt::AlignCenter);
    disenchantGridLayout->addWidget(_setsCheckbox, 2, 1, Qt::AlignCenter);

    // upgrade box setup
    _upgradeBox = new QGroupBox(tr("Upgrade here all:"), _itemManagementBox);
    _upgradeGemsButton = new QPushButton(tr("Gems"), _upgradeBox);
    _upgradeRunesButton = new QPushButton(tr("Runes"), _upgradeBox);
    _upgradeBothButton = new QPushButton(tr("Both"), _upgradeBox);

    QVBoxLayout *upgradeBoxLayout = new QVBoxLayout(_upgradeBox);
    upgradeBoxLayout->addWidget(_upgradeGemsButton);
    upgradeBoxLayout->addWidget(_upgradeRunesButton);
    upgradeBoxLayout->addWidget(_upgradeBothButton);

    _applyActionToAllPagesCheckbox = new QCheckBox(tr("Apply to all pages"), this);
    _applyActionToAllPagesCheckbox->setToolTip(tr("Either action will be applied to all pages of the current PlugY stash"));
    _applyActionToAllPagesCheckbox->setChecked(true);
    connect(_applyActionToAllPagesCheckbox, SIGNAL(toggled(bool)), SLOT(applyActionToAllPagesChanged(bool)));

    // item management groupbox layout
    QHBoxLayout *itemManagementBoxLayout = new QHBoxLayout(_itemManagementBox);
    itemManagementBoxLayout->addWidget(_disenchantBox);
    itemManagementBoxLayout->addStretch();
    itemManagementBoxLayout->addWidget(_applyActionToAllPagesCheckbox/*, 0, Qt::AlignCenter*/);
    itemManagementBoxLayout->addStretch();
    itemManagementBoxLayout->addWidget(_upgradeBox);

    // misc
    connect(_tabWidget, SIGNAL(currentChanged(int)), SLOT(tabChanged(int)));

    loadSettings();
    _tabWidget->setCurrentIndex(0);
}

void ItemsViewerDialog::loadSettings()
{
    QSettings settings;
    restoreGeometry(settings.value("itemsViewerGeometry").toByteArray());

    for (int i = GearIndex; i <= LastIndex; ++i)
        splitterAtIndex(i)->restoreState(settings.value(QString("itemsTab%1_state").arg(i)).toByteArray());
}

ItemsPropertiesSplitter *ItemsViewerDialog::splitterAtIndex(int tabIndex)
{
    return static_cast<ItemsPropertiesSplitter *>(_tabWidget->widget(tabIndex));
}

void ItemsViewerDialog::saveSettings()
{
    QSettings settings;
    settings.setValue("itemsViewerGeometry", saveGeometry());

    for (int i = GearIndex; i <= LastIndex; ++i)
        settings.setValue(QString("itemsTab%1_state").arg(i), splitterAtIndex(i)->saveState());
}

void ItemsViewerDialog::reject()
{
    saveSettings();
    updateBeltItemsCoordinates(true, 0);
    emit closing();
    QDialog::reject();
}

void ItemsViewerDialog::tabChanged(int tabIndex)
{
    splitterAtIndex(tabIndex)->showFirstItem();
    _itemManagementBox->setEnabled(tabIndex > GearIndex);
    _applyActionToAllPagesCheckbox->setEnabled(isPlugyStorageIndex(tabIndex));
    if (tabIndex > GearIndex)
    {
        if (isPlugyStorageIndex(tabIndex))
            static_cast<PlugyItemsSplitter *>(_tabWidget->currentWidget())->setApplyActionToAllPages(_applyActionToAllPagesCheckbox->isChecked()); // must be explicitly set
        updateButtonsState();
    }
}

void ItemsViewerDialog::itemCountChangedInCurrentTab(int newCount)
{
    itemCountChangedInTab(_tabWidget->currentIndex(), newCount);
}

void ItemsViewerDialog::itemCountChangedInTab(int tabIndex, int newCount)
{
    QString newTabTitle = isPlugyStorageIndex(tabIndex) ? QString(" (%1/%2)").arg(splitterAtIndex(tabIndex)->itemsModel()->itemCount()).arg(newCount) : QString(" (%1)").arg(newCount);
    _tabWidget->setTabText(tabIndex, tabNameAtIndex(tabIndex) + newTabTitle);
}

void ItemsViewerDialog::updateItems(const QHash<int, bool> &plugyStashesExistenceHash)
{
    _itemsTotal = 0;
    for (int i = GearIndex; i <= LastIndex; ++i)
    {
        bool isGearTab = i == GearIndex;
        ItemsList items = ItemDataBase::itemsStoredIn(Enums::ItemStorage::metaEnum().value(i), isGearTab ? Enums::ItemLocation::Equipped : Enums::ItemLocation::Stored);
        if (isGearTab)
        {
            items += ItemDataBase::itemsStoredIn(Enums::ItemStorage::NotInStorage, Enums::ItemLocation::Merc);
            items += ItemDataBase::itemsStoredIn(Enums::ItemStorage::NotInStorage, Enums::ItemLocation::Corpse);

            foreach (ItemInfo *item, items)
            {
                switch (item->whereEquipped)
                {
                case 1: // helm
                    item->row = 0;
                    item->column = 3;
                    break;
                case 2: // amulet
                    item->row = 1;
                    item->column = 5;
                    break;
                case 3: // armor
                    item->row = 2;
                    item->column = 3;
                    break;
                case 4: // right hand (usually weapon)
                    item->row = 0;
                    item->column = 0;
                    break;
                case 5: // left hand (usually shield)
                    item->row = 0;
                    item->column = 6;
                    break;
                case 6: // right ring
                    item->row = 5;
                    item->column = 2;
                    break;
                case 7: // left ring
                    item->row = 5;
                    item->column = 5;
                    break;
                case 8: // belt
                    item->row = 5;
                    item->column = 3;
                    break;
                case 9: // boots
                    item->row = 4;
                    item->column = 6;
                    break;
                case 10: // gloves
                    item->row = 4;
                    item->column = 0;
                    break;
                case 11: // alt. right hand (usually weapon)
                    item->row = 7;
                    item->column = 0;
                    break;
                case 12: // alt. left hand (usually shield)
                    item->row = 7;
                    item->column = 6;
                    break;
                default:
                    qDebug("item->whereEquipped == %d!!!", item->whereEquipped);
                    break;
                }
            }

            ItemsList beltItems = ItemDataBase::itemsStoredIn(Enums::ItemStorage::NotInStorage, Enums::ItemLocation::Belt);
            updateBeltItemsCoordinates(false, &beltItems);
            items += beltItems;
        }
        else // itemCountChanged() signal is sent from GearItemsSplitter when setItems() is called
            itemCountChangedInTab(i, items.size());
        splitterAtIndex(i)->setItems(items);
        
        _itemsTotal += items.size();
    }

    setCubeTabDisabled(!CharacterInfo::instance().items.hasCube());
    for (QHash<int, bool>::const_iterator iter = plugyStashesExistenceHash.constBegin(); iter != plugyStashesExistenceHash.constEnd(); ++iter)
        _tabWidget->setTabEnabled(tabIndexFromItemStorage(iter.key()), iter.value());
    
    updateWindowTitle();
}

int ItemsViewerDialog::tabIndexFromItemStorage(int storage)
{
    return storage > Enums::ItemStorage::Inventory ? storage - 2 : storage;
}

const QString &ItemsViewerDialog::tabNameAtIndex(int i)
{
    // add elements here when adding new tab
    static const QStringList tabNames = QStringList() << tr("Gear") << tr("Inventory") << tr("Cube") << tr("Stash") << tr("Personal Stash") << tr("Shared Stash") << tr("Hardcore Stash");
    static const QString unknownName = tr("UNKNOWN STORAGE");
    return i >= 0 && i < tabNames.size() ? tabNames.at(i) : unknownName;
}

void ItemsViewerDialog::showItem(ItemInfo *item)
{
    _tabWidget->setCurrentIndex(tabIndexFromItemStorage(item->storage));
    splitterAtIndex(_tabWidget->currentIndex())->showItem(item);
}

void ItemsViewerDialog::setCubeTabDisabled(bool disabled)
{
    _tabWidget->setTabEnabled(CubeIndex, !disabled);
}

void ItemsViewerDialog::updateWindowTitle()
{
    setWindowTitle(tr("Items viewer (items total: %1)").arg(_itemsTotal));
}

void ItemsViewerDialog::decreaseItemCount()
{
    --_itemsTotal;
    updateWindowTitle();
}

void ItemsViewerDialog::applyActionToAllPagesChanged(bool b)
{
    static_cast<PlugyItemsSplitter *>(_tabWidget->currentWidget())->setApplyActionToAllPages(b);
    updateButtonsState();
}

void ItemsViewerDialog::updateButtonsState()
{
    updateDisenchantButtonsState();
    updateUpgradeButtonsState();
}

void ItemsViewerDialog::updateDisenchantButtonsState()
{
    QPair<bool, bool> allowDisenchantButtons = splitterAtIndex(_tabWidget->currentIndex())->updateDisenchantButtonsState(_uniquesCheckbox->isChecked(), _setsCheckbox->isChecked());
    _disenchantToShardsButton->setEnabled(allowDisenchantButtons.first);
    _disenchantToSignetButton->setEnabled(allowDisenchantButtons.second);
}

void ItemsViewerDialog::updateUpgradeButtonsState()
{
    QPair<bool, bool> allowUpgradeButtons = splitterAtIndex(_tabWidget->currentIndex())->updateUpgradeButtonsState();
    _upgradeGemsButton->setEnabled(allowUpgradeButtons.first);
    _upgradeRunesButton->setEnabled(allowUpgradeButtons.second);
    _upgradeBothButton->setEnabled(allowUpgradeButtons.first && allowUpgradeButtons.second);
}

void ItemsViewerDialog::disenchantAllItems()
{
    bool toShards = sender() == _disenchantToShardsButton;
    splitterAtIndex(_tabWidget->currentIndex())->disenchantAllItems(toShards, _upgradeToCrystalsCheckbox->isChecked(), _eatSignetsCheckbox->isChecked(), _uniquesCheckbox->isChecked(), _setsCheckbox->isChecked());
    if (toShards || !isUltimative())
    {
        _disenchantToShardsButton->setDisabled(true);
        _disenchantToSignetButton->setDisabled(true);
    }
    else // TUs may leave after disenchanting to signets in Ultimative
        updateDisenchantButtonsState();
}

void ItemsViewerDialog::updateBeltItemsCoordinates(bool restore, ItemsList *pBeltItems)
{
    ItemsList beltItems = pBeltItems ? *pBeltItems : ItemDataBase::itemsStoredIn(Enums::ItemStorage::NotInStorage, Enums::ItemLocation::Belt);
    int lastRowIndex = kRows.at(GearIndex) - 1;
    foreach (ItemInfo *item, beltItems)
    {
        item->row = lastRowIndex - item->row;
        item->column += restore ? -2 : 2;
    }
}
