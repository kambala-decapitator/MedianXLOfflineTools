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
#include "kexpandablegroupbox.h"

#include <QTabWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QCloseEvent>
#include <QGroupBox>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>

#include <QSettings>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


const int ItemsViewerDialog::kCellSize = 32;

ItemsViewerDialog::ItemsViewerDialog(const QHash<int, bool> &plugyStashesExistenceHash, QWidget *parent) : QDialog(parent), _tabWidget(new QTabWidget(this)), _itemManagementWidget(new QWidget(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

    KExpandableGroupBox *expandableBox = new KExpandableGroupBox(tr("Item management"), this);
    expandableBox->setAnimateExpansion(false);
    expandableBox->setWidget(_itemManagementWidget);
    expandableBox->setExpanded(true);

    // main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(_tabWidget);
    mainLayout->addWidget(expandableBox);

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
        splitter->setModel(new ItemStorageTableModel(kRows().at(i), i == GearIndex ? 8 : 10, splitter));
        _tabWidget->addTab(splitter, tabNameAtIndex(i));

        connect(splitter, SIGNAL(itemCountChanged(int)), SLOT(itemCountChangedInCurrentTab(int)));
        connect(splitter, SIGNAL(itemDeleted()), SLOT(decreaseItemCount()));
        connect(splitter, SIGNAL(itemsChanged(bool)), SIGNAL(itemsChanged(bool)));
        connect(splitter, SIGNAL(cubeDeleted(bool)), SIGNAL(cubeDeleted(bool)));
        connect(splitter, SIGNAL(cubeDeleted(bool)), SLOT(setCubeTabDisabled(bool)));
        connect(splitter, SIGNAL(signetsOfLearningEaten(int)), SIGNAL(signetsOfLearningEaten(int)));
        if (isPlugyStorageIndex(i))
            connect(static_cast<PlugyItemsSplitter *>(splitter), SIGNAL(pageChanged()), SLOT(updateButtonsState()));
    }
    updateItems(plugyStashesExistenceHash, true);

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
    _itemManagementWidget->setDisabled(true);

    // disenchant box setup
    _disenchantBox = new QGroupBox(tr("Mass Disenchant"), _itemManagementWidget);

    _disenchantToShardsButton = new QPushButton(tr("Arcane Shards"), _disenchantBox);
    _upgradeToCrystalsCheckbox = new QCheckBox(tr("Upgrade to Crystals"), _disenchantBox);
    _uniquesRadioButton = new QRadioButton(tr("Uniques"), _disenchantBox);
    _upgradeToCrystalsCheckbox->setChecked(true);

    _disenchantToSignetButton = new QPushButton(tr("Signets of Learning"), _disenchantBox);
    _eatSignetsCheckbox = new QCheckBox(tr("Eat Signets"), _disenchantBox);
    _setsRadioButton = new QRadioButton(tr("Sets"), _disenchantBox);
    _eatSignetsCheckbox->setChecked(true);

    _bothQualitiesRadioButton = new QRadioButton(tr("Both"), _disenchantBox);
    _bothQualitiesRadioButton->setChecked(true);

    QGroupBox *box = new QGroupBox(tr("Disenchant:"), this);
    QHBoxLayout *hlayout = new QHBoxLayout(box);
    hlayout->addWidget(_uniquesRadioButton);
    hlayout->addStretch();
    hlayout->addWidget(_setsRadioButton);
    hlayout->addStretch();
    hlayout->addWidget(_bothQualitiesRadioButton);

    connect(_disenchantToShardsButton, SIGNAL(clicked()), SLOT(disenchantAllItems()));
    connect(_disenchantToSignetButton, SIGNAL(clicked()), SLOT(disenchantAllItems()));

    connect(       _uniquesRadioButton, SIGNAL(toggled(bool)), SLOT(updateDisenchantButtonsState()));
    connect(          _setsRadioButton, SIGNAL(toggled(bool)), SLOT(updateDisenchantButtonsState()));
    connect( _bothQualitiesRadioButton, SIGNAL(toggled(bool)), SLOT(updateDisenchantButtonsState()));
    connect(_upgradeToCrystalsCheckbox, SIGNAL(toggled(bool)), SLOT(updateDisenchantButtonsState()));

    QVBoxLayout *vboxLayout = new QVBoxLayout(_disenchantBox);
    vboxLayout->addWidget(box);

    box = new QGroupBox(tr("To:"), this);

    QGridLayout *disenchantGridLayout = new QGridLayout(box);
    disenchantGridLayout->addWidget(_disenchantToShardsButton, 0, 0);
    disenchantGridLayout->addWidget(_upgradeToCrystalsCheckbox, 1, 0, Qt::AlignCenter);
    disenchantGridLayout->addWidget(_disenchantToSignetButton, 0, 1);
    disenchantGridLayout->addWidget(_eatSignetsCheckbox, 1, 1, Qt::AlignCenter);

    vboxLayout->addWidget(box);

    // upgrade box setup
    _upgradeBox = new QGroupBox(tr("Upgrade here all:"), _itemManagementWidget);
    _upgradeGemsButton = new QPushButton(tr("Gems"), _upgradeBox);
    _upgradeRunesButton = new QPushButton(tr("Runes"), _upgradeBox);
    _upgradeBothButton = new QPushButton(tr("Both"), _upgradeBox);

    connect(_upgradeGemsButton,  SIGNAL(clicked()), SLOT(upgradeGems()));
    connect(_upgradeRunesButton, SIGNAL(clicked()), SLOT(upgradeRunes()));
    connect(_upgradeBothButton,  SIGNAL(clicked()), SLOT(upgradeGemsAndRunes()));

    QVBoxLayout *upgradeBoxLayout = new QVBoxLayout(_upgradeBox);
    upgradeBoxLayout->addWidget(_upgradeGemsButton);
    upgradeBoxLayout->addWidget(_upgradeRunesButton);
    upgradeBoxLayout->addWidget(_upgradeBothButton);

    _applyActionToAllPagesCheckbox = new QCheckBox(tr("Apply to all pages"), this);
    _applyActionToAllPagesCheckbox->setToolTip(tr("Either action will be applied to all pages of the current PlugY stash"));
    _applyActionToAllPagesCheckbox->setChecked(true);
    connect(_applyActionToAllPagesCheckbox, SIGNAL(toggled(bool)), SLOT(applyActionToAllPagesChanged(bool)));

    // item management groupbox layout
    QHBoxLayout *itemManagementBoxLayout = new QHBoxLayout(_itemManagementWidget);
    itemManagementBoxLayout->addWidget(_disenchantBox);
    itemManagementBoxLayout->addStretch();
    itemManagementBoxLayout->addWidget(_applyActionToAllPagesCheckbox);
    itemManagementBoxLayout->addStretch();
    itemManagementBoxLayout->addWidget(_upgradeBox);

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

ItemsPropertiesSplitter *ItemsViewerDialog::currentSplitter()
{
    return splitterAtIndex(_tabWidget->currentIndex());
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
    _itemManagementWidget->setEnabled(tabIndex > GearIndex);
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
    QString newTabTitle = tabNameAtIndex(tabIndex);
    if (tabIndex == GearIndex)
        newTabTitle += " - " + static_cast<GearItemsSplitter *>(_tabWidget->widget(tabIndex))->currentGearTitle();
    _tabWidget->setTabText(tabIndex, newTabTitle + (isPlugyStorageIndex(tabIndex) ? QString(" (%1/%2)").arg(splitterAtIndex(tabIndex)->itemsModel()->itemCount()).arg(newCount) : QString(" (%1)").arg(newCount)));
}

void ItemsViewerDialog::updateItems(const QHash<int, bool> &plugyStashesExistenceHash, bool isCreatingTabs)
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
        
        // itemCountChanged() signal is sent from GearItemsSplitter::setItems() -> updateItemsForCurrentGear()
        if (isGearTab)
            static_cast<GearItemsSplitter *>(_tabWidget->widget(i))->setItems(items, isCreatingTabs);
        else
        {
            splitterAtIndex(i)->setItems(items);
            itemCountChangedInTab(i, items.size());
        }
        _itemsTotal += items.size();
    }

    setCubeTabDisabled(!CharacterInfo::instance().items.hasCube());
    for (QHash<int, bool>::const_iterator iter = plugyStashesExistenceHash.constBegin(); iter != plugyStashesExistenceHash.constEnd(); ++iter)
        _tabWidget->setTabEnabled(tabIndexFromItemStorage(iter.key()), iter.value());

    updateWindowTitle();
}

int ItemsViewerDialog::rowsInStorageAtIndex(int storage)
{
    return kRows().at(tabIndexFromItemStorage(storage));
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
    currentSplitter()->showItem(item);
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
    bool areUniquesSelected = _uniquesRadioButton->isChecked(), areSetsSelected = _setsRadioButton->isChecked();
    if (_bothQualitiesRadioButton->isChecked())
        areUniquesSelected = areSetsSelected = true;
    QPair<bool, bool> allowDisenchantButtons = currentSplitter()->updateDisenchantButtonsState(areUniquesSelected, areSetsSelected, _upgradeToCrystalsCheckbox->isChecked());
    _disenchantToShardsButton->setEnabled(allowDisenchantButtons.first);
    _disenchantToSignetButton->setEnabled(allowDisenchantButtons.second);
}

void ItemsViewerDialog::updateUpgradeButtonsState()
{
    QPair<bool, bool> allowUpgradeButtons = currentSplitter()->updateUpgradeButtonsState();
    _upgradeGemsButton->setEnabled(allowUpgradeButtons.first);
    _upgradeRunesButton->setEnabled(allowUpgradeButtons.second);
    _upgradeBothButton->setEnabled(allowUpgradeButtons.first && allowUpgradeButtons.second);
}

void ItemsViewerDialog::disenchantAllItems()
{
    bool toShards = sender() == _disenchantToShardsButton, areUniquesSelected = _uniquesRadioButton->isChecked(), areSetsSelected = _setsRadioButton->isChecked();
    if (_bothQualitiesRadioButton->isChecked())
        areUniquesSelected = areSetsSelected = true;
    // TODO: show dialog with item names and checkboxes
    currentSplitter()->disenchantAllItems(toShards, _upgradeToCrystalsCheckbox->isChecked(), _eatSignetsCheckbox->isChecked(), areUniquesSelected, areSetsSelected);
    if (toShards || !isUltimative())
    {
        _disenchantToShardsButton->setDisabled(true);
        _disenchantToSignetButton->setDisabled(true);
    }
    else // TUs may leave after disenchanting to signets in Ultimative
        updateDisenchantButtonsState();
}

void ItemsViewerDialog::upgradeGems()
{
    currentSplitter()->upgradeGems();

    _upgradeGemsButton->setDisabled(true);
    _upgradeBothButton->setDisabled(true);
}

void ItemsViewerDialog::upgradeRunes()
{
    currentSplitter()->upgradeRunes();

    _upgradeRunesButton->setDisabled(true);
    _upgradeBothButton->setDisabled(true);
}

void ItemsViewerDialog::updateBeltItemsCoordinates(bool restore, ItemsList *pBeltItems)
{
    ItemsList beltItems = pBeltItems ? *pBeltItems : ItemDataBase::itemsStoredIn(Enums::ItemStorage::NotInStorage, Enums::ItemLocation::Belt);
    int lastRowIndex = kRows().at(GearIndex) - 1;
    foreach (ItemInfo *item, beltItems)
    {
        item->row = lastRowIndex - item->row;
        item->column += restore ? -2 : 2;
    }
}

QList<int> &ItemsViewerDialog::kRows()
{
    static QList<int> rows;
    if (rows.isEmpty())
        rows = QList<int>() << 11 << (isUltimative5OrLater() ? 8 : 6) << 8 << 10 << 10 << 10 << 10;
    return rows;
}
