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
#include "disenchantpreviewdialog.h"
#include "stashsortingoptionsdialog.h"

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
#include <QInputDialog>
#include <QSpinBox>
#include <QLabel>

#include <QSettings>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


const int ItemsViewerDialog::kCellSize = 32, ItemsViewerDialog::kColumnsDefault = 15;

ItemsViewerDialog::ItemsViewerDialog(const QHash<int, bool> &plugyStashesExistenceHash, quint8 showDisenchantPreviewOption, QWidget *parent) : QDialog(parent), _tabWidget(new QTabWidget(this)),
    _showDisenchantPreviewOption(static_cast<ShowDisenchantPreviewOption>(showDisenchantPreviewOption))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Window);

    createLayout();

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
        splitter->setModel(new ItemStorageTableModel(kRows().at(i), i == GearIndex ? 8 : kColumnsDefault, splitter));
        _tabWidget->addTab(splitter, tabNameAtIndex(i));

        connect(splitter, SIGNAL(itemCountChanged(int)), SLOT(itemCountChangedInCurrentTab(int)));
        connect(splitter, SIGNAL(itemDeleted()), SLOT(decreaseItemCount()));
        connect(splitter, SIGNAL(itemsChanged(bool)), SIGNAL(itemsChanged(bool)));
        connect(splitter, SIGNAL(cubeDeleted(bool)), SIGNAL(cubeDeleted(bool)));
        connect(splitter, SIGNAL(cubeDeleted(bool)), SLOT(setCubeTabDisabled(bool)));
        connect(splitter, SIGNAL(signetsOfLearningEaten(int)), SIGNAL(signetsOfLearningEaten(int)));
        connect(splitter, SIGNAL(itemMovingBetweenStashes(ItemInfo *)), SLOT(moveItemBetweenStashes(ItemInfo *)));

        if (isPlugyStorageIndex(i))
        {
            PlugyItemsSplitter *plugySplitter = static_cast<PlugyItemsSplitter *>(splitter);
            plugySplitter->isSharedStash = i >= SharedStashIndex && i <= HCStashIndex;
            plugySplitter->isHcStash = i == HCStashIndex;
            connect(plugySplitter, SIGNAL(pageChanged()), SLOT(updateItemManagementButtonsState()));
            connect(plugySplitter, SIGNAL(stashSorted()), SIGNAL(stashSorted()));
        }
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

    // quick hack instead of storing ivar
    connect(static_cast<KExpandableGroupBox *>(layout()->itemAt(layout()->count() - 1)->widget()), SIGNAL(expanded(bool)), SLOT(adjustHeight(bool)));

    connect(_disenchantToShardsButton, SIGNAL(clicked()), SLOT(disenchantAllItems()));
    connect(_disenchantToSignetButton, SIGNAL(clicked()), SLOT(disenchantAllItems()));

    connect(       _uniquesRadioButton, SIGNAL(toggled(bool)), SLOT(updateDisenchantButtonsState()));
    connect(          _setsRadioButton, SIGNAL(toggled(bool)), SLOT(updateDisenchantButtonsState()));
    connect( _bothQualitiesRadioButton, SIGNAL(toggled(bool)), SLOT(updateDisenchantButtonsState()));
    connect(_upgradeToCrystalsCheckbox, SIGNAL(toggled(bool)), SLOT(updateDisenchantButtonsState()));

    connect(_upgradeGemsButton,  SIGNAL(clicked()), SLOT(upgradeGems()));
    connect(_upgradeRunesButton, SIGNAL(clicked()), SLOT(upgradeRunes()));
    connect(_upgradeBothButton,  SIGNAL(clicked()), SLOT(upgradeGemsAndRunes()));

    connect(_sortStashButton, SIGNAL(clicked()), SLOT(sortStash()));
    connect(_insertBlankPagesBeforeButton, SIGNAL(clicked()), SLOT(insertBlankPages()));
    connect(_insertBlankPagesAfterButton,  SIGNAL(clicked()), SLOT(insertBlankPages()));
    connect(_removeCurrentBlankPage,       SIGNAL(clicked()), SLOT(removeCurrentPage()));

    connect(_applyActionToAllPagesCheckbox, SIGNAL(toggled(bool)), SLOT(applyActionToAllPagesChanged(bool)));
    connect(_tabWidget, SIGNAL(currentChanged(int)), SLOT(tabChanged(int)));
    connect(_moveCurrentItemsToSharedStashButton, SIGNAL(clicked()), SLOT(moveCurrentItemsToSharedStash()));

    _stashBox->hide();
    _itemManagementWidget->setDisabled(true);

    loadSettings();
    _tabWidget->setCurrentIndex(0);
}

void ItemsViewerDialog::createLayout()
{
    _itemManagementWidget = new QWidget(this);
    KExpandableGroupBox *expandableBox = new KExpandableGroupBox(tr("Item management"), this);
    expandableBox->setAnimateExpansion(false);
    expandableBox->setWidget(_itemManagementWidget);
    expandableBox->setExpanded(true);

    // disenchant box setup
    _disenchantBox = new QGroupBox(tr("Mass Disenchant"), _itemManagementWidget);

    _disenchantToShardsButton = new QPushButton(tr("Arcane Shards"), _disenchantBox);
    _upgradeToCrystalsCheckbox = new QCheckBox(tr("Upgrade to Crystals"), _disenchantBox);
    _upgradeToCrystalsCheckbox->setChecked(true);
    _uniquesRadioButton = new QRadioButton(tr("Uniques"), _disenchantBox);

    _disenchantToSignetButton = new QPushButton(tr("Signets of Learning"), _disenchantBox);
    _eatSignetsCheckbox = new QCheckBox(tr("Eat Signets"), _disenchantBox);
    _eatSignetsCheckbox->setChecked(true);
    _setsRadioButton = new QRadioButton(tr("Sets"), _disenchantBox);

    _bothQualitiesRadioButton = new QRadioButton(tr("Both"), _disenchantBox);
    _bothQualitiesRadioButton->setChecked(true);

    QGroupBox *box = new QGroupBox(tr("Disenchant:"), this);
    QHBoxLayout *hlayout = new QHBoxLayout(box);
    hlayout->addWidget(_uniquesRadioButton);
    hlayout->addStretch();
    hlayout->addWidget(_setsRadioButton);
    hlayout->addStretch();
    hlayout->addWidget(_bothQualitiesRadioButton);

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
    _reserveRunesSpinBox = new QSpinBox(_upgradeBox);
    _reserveRunesSpinBox->setRange(0, 10);
    _reserveRunesSpinBox->setToolTip(tr("Minimum number of each rune type to reserve"));
    _upgradeBothButton = new QPushButton(tr("Both"), _upgradeBox);

    QLabel *reserveRunesLabel = new QLabel(tr("Reserve:"), _upgradeBox);
    reserveRunesLabel->setBuddy(_reserveRunesSpinBox);
    reserveRunesLabel->setToolTip(_reserveRunesSpinBox->toolTip());

    QFrame *runesFrame = new QFrame(_upgradeBox);
    runesFrame->setFrameShape(QFrame::StyledPanel);

    QGridLayout *runesGridLayout = new QGridLayout(runesFrame);
    runesGridLayout->addWidget(_upgradeRunesButton, 0, 0, 1, 2);
    runesGridLayout->addWidget(reserveRunesLabel, 1, 0);
    runesGridLayout->addWidget(_reserveRunesSpinBox, 1, 1);

    vboxLayout = new QVBoxLayout(_upgradeBox);
    vboxLayout->addWidget(_upgradeGemsButton);
    vboxLayout->addWidget(runesFrame);
    vboxLayout->addWidget(_upgradeBothButton);

    _applyActionToAllPagesCheckbox = new QCheckBox(tr("Apply to all pages"), this);
    _applyActionToAllPagesCheckbox->setToolTip(tr("Either action will be applied to all pages of the current PlugY stash"));
    _applyActionToAllPagesCheckbox->setChecked(true);
    _applyActionToAllPagesCheckbox->hide();

    _moveCurrentItemsToSharedStashButton = new QPushButton(tr("Move items here\nto shared stash"), this);

    // stash box setup
    _stashBox = new QGroupBox(tr("PlugY Stash"), _itemManagementWidget);
    _sortStashButton = new QPushButton(tr("Sort"), _stashBox);
    QGroupBox *blankPagesBox = new QGroupBox(tr("Blank pages:"), _stashBox);
    _insertBlankPagesBeforeButton = new QPushButton(tr("Insert before"),  blankPagesBox);
    _insertBlankPagesAfterButton  = new QPushButton(tr("Insert after"),   blankPagesBox);
    _removeCurrentBlankPage       = new QPushButton(tr("Remove current"), blankPagesBox);

    vboxLayout = new QVBoxLayout(blankPagesBox);
    vboxLayout->addWidget(_insertBlankPagesBeforeButton);
    vboxLayout->addWidget(_insertBlankPagesAfterButton);
    vboxLayout->addWidget(_removeCurrentBlankPage);

    vboxLayout = new QVBoxLayout(_stashBox);
    vboxLayout->addWidget(_sortStashButton);
    vboxLayout->addWidget(blankPagesBox);

    vboxLayout = new QVBoxLayout;
    vboxLayout->addWidget(_applyActionToAllPagesCheckbox);
    vboxLayout->addWidget(_moveCurrentItemsToSharedStashButton);

    // item management groupbox layout
    QHBoxLayout *itemManagementBoxLayout = new QHBoxLayout(_itemManagementWidget);
    itemManagementBoxLayout->addWidget(_disenchantBox);
    itemManagementBoxLayout->addLayout(vboxLayout);
    itemManagementBoxLayout->addWidget(_upgradeBox);
    itemManagementBoxLayout->addStretch();
    itemManagementBoxLayout->addWidget(_stashBox);

    // main layout
    vboxLayout = new QVBoxLayout(this);
    vboxLayout->addWidget(_tabWidget);
    vboxLayout->addWidget(expandableBox);
}


void ItemsViewerDialog::loadSettings()
{
    QSettings settings;
    restoreGeometry(settings.value("itemsViewerGeometry").toByteArray());

    for (int i = GearIndex; i <= LastIndex; ++i)
        splitterAtIndex(i)->restoreState(settings.value(QString("itemsTab%1_state").arg(i)).toByteArray());

    _reserveRunesSpinBox->setValue(settings.value("reserveRunes").toInt());
}

void ItemsViewerDialog::saveSettings()
{
    QSettings settings;
    settings.setValue("itemsViewerGeometry", saveGeometry());

    for (int i = GearIndex; i <= LastIndex; ++i)
        settings.setValue(QString("itemsTab%1_state").arg(i), splitterAtIndex(i)->saveState());

    settings.setValue("reserveRunes", _reserveRunesSpinBox->value());
}

void ItemsViewerDialog::reject()
{
    saveSettings();
    updateBeltItemsCoordinates(true, 0);
    emit closing();
    QDialog::reject();
}

ItemsPropertiesSplitter *ItemsViewerDialog::splitterAtIndex(int tabIndex)
{
    return static_cast<ItemsPropertiesSplitter *>(_tabWidget->widget(tabIndex));
}

ItemsPropertiesSplitter *ItemsViewerDialog::currentSplitter()
{
    return splitterAtIndex(_tabWidget->currentIndex());
}

PlugyItemsSplitter *ItemsViewerDialog::currentPlugySplitter()
{
    return static_cast<PlugyItemsSplitter *>(_tabWidget->currentWidget());
}

void ItemsViewerDialog::tabChanged(int tabIndex)
{
    bool isStorage = tabIndex > GearIndex, isPlugyStash = isPlugyStorageIndex(tabIndex);

    splitterAtIndex(tabIndex)->showFirstItem();
    _itemManagementWidget->setEnabled(isStorage);

    if (isStorage)
    {
        if (isPlugyStash)
            currentPlugySplitter()->setApplyActionToAllPages(_applyActionToAllPagesCheckbox->isChecked()); // must be explicitly set
        updateItemManagementButtonsState();
    }
    _applyActionToAllPagesCheckbox->setVisible(isPlugyStash);
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
            updateGearItems(0, &items, isCreatingTabs);
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

void ItemsViewerDialog::updateGearItems(ItemsList *pBeltItems /*= 0*/, ItemsList *pEquippedItems /*= 0*/, bool isCreatingTabs /*= false*/)
{
    ItemsList itemsFoo, &items = pEquippedItems ? *pEquippedItems : itemsFoo;
    if (!pEquippedItems)
        items = ItemDataBase::itemsStoredIn(Enums::ItemStorage::NotInStorage, Enums::ItemLocation::Equipped);
    items += ItemDataBase::itemsStoredIn(Enums::ItemStorage::NotInStorage, Enums::ItemLocation::Merc);
    items += ItemDataBase::itemsStoredIn(Enums::ItemStorage::NotInStorage, Enums::ItemLocation::IronGolem);

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

    ItemsList beltItemsFoo, &beltItems = pBeltItems ? *pBeltItems : beltItemsFoo;
    if (!pBeltItems)
        beltItems = ItemDataBase::itemsStoredIn(Enums::ItemStorage::NotInStorage, Enums::ItemLocation::Belt);
    updateBeltItemsCoordinates(false, &beltItems);
    items += beltItems;

    // itemCountChanged() signal is sent from GearItemsSplitter::setItems() -> updateItemsForCurrentGear()
    static_cast<GearItemsSplitter *>(_tabWidget->widget(GearIndex))->setItems(items, isCreatingTabs);
}

const QList<int> &ItemsViewerDialog::kRows()
{
    static QList<int> rows = QList<int>() << 11 << 10 << 16 << 16 << 16 << 16 << 16;
    return rows;
}

int ItemsViewerDialog::rowsInStorageAtIndex(int storage)
{
    return kRows().at(tabIndexFromItemStorage(storage));
}

int ItemsViewerDialog::colsInStorageAtIndex(int /*storage*/)
{
    return kColumnsDefault;
}

int ItemsViewerDialog::tabIndexFromItemStorage(int storage)
{
    return storage > Enums::ItemStorage::Inventory ? storage - 2 : storage;
}

const QString &ItemsViewerDialog::tabNameAtIndex(int i)
{
    // add elements here when adding new tab
    static const QStringList tabNames = QStringList() << tr("Gear") << tr("Inventory") << tr("Cube") << tr("Stash") << tr("Personal Stash") << tr("Shared Stash") << tr("Hardcore Stash");
    static const QString unknownName("WTF");
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
    if (isPlugyStorageIndex(_tabWidget->currentIndex()))
        updateRemoveCurrentBlankPageButtonState();
}

void ItemsViewerDialog::applyActionToAllPagesChanged(bool b)
{
    currentPlugySplitter()->setApplyActionToAllPages(b);
    updateItemManagementButtonsState();
}

void ItemsViewerDialog::updateItemManagementButtonsState()
{
    updateDisenchantButtonsState();
    updateUpgradeButtonsState();
    updateStashButtonsState();
    _moveCurrentItemsToSharedStashButton->setVisible(!_stashBox->isVisible());
}

#ifdef Q_OS_MAC
void ItemsViewerDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_W)
        reject();
    else
        QDialog::keyPressEvent(e);
}
#endif

void ItemsViewerDialog::updateDisenchantButtonsState()
{
    bool areUniquesSelected = _uniquesRadioButton->isChecked(), areSetsSelected = _setsRadioButton->isChecked();
    if (_bothQualitiesRadioButton->isChecked())
        areUniquesSelected = areSetsSelected = true;
    QPair<bool, bool> allowDisenchantButtons = currentSplitter()->updateDisenchantButtonsState(areUniquesSelected, areSetsSelected, _upgradeToCrystalsCheckbox->isChecked());
    _disenchantToShardsButton->setEnabled(allowDisenchantButtons.first);
    _disenchantToSignetButton->setEnabled(allowDisenchantButtons.second);
}

void ItemsViewerDialog::disenchantAllItems()
{
    bool toShards = sender() == _disenchantToShardsButton, areUniquesSelected = _uniquesRadioButton->isChecked(), areSetsSelected = _setsRadioButton->isChecked();
    if (_bothQualitiesRadioButton->isChecked())
        areUniquesSelected = areSetsSelected = true;

    ItemsPropertiesSplitter *splitter = currentSplitter();
    ItemsList *items = splitter->getItems(), filteredItems;
    foreach (ItemInfo *item, *items)
        if ((item->quality == Enums::ItemQuality::Unique && areUniquesSelected) || (item->quality == Enums::ItemQuality::Set && areSetsSelected))
            if ((toShards && ItemDataBase::canDisenchantIntoArcaneShards(item)) || (!toShards && ItemDataBase::canDisenchantIntoSignetOfLearning(item)))
                filteredItems += item;

    ItemsList selectedItems;
    bool allItemsDisenchanted = true;
    if (_showDisenchantPreviewOption == Always || (_showDisenchantPreviewOption == OnlyCurrentPage && !_applyActionToAllPagesCheckbox->isChecked()))
    {
        DisenchantPreviewDialog dialog(filteredItems, !_applyActionToAllPagesCheckbox->isChecked(), this);
        connect(dialog.selectItemDelegate, SIGNAL(showItem(ItemInfo *)), SLOT(showItem(ItemInfo *)));
        if (dialog.exec())
        {
            selectedItems = dialog.selectedItems();
            allItemsDisenchanted = selectedItems.size() == filteredItems.size();
        }
    }
    else
        selectedItems = filteredItems;

    if (!selectedItems.isEmpty())
    {
        currentSplitter()->disenchantAllItems(toShards, _upgradeToCrystalsCheckbox->isChecked(), _eatSignetsCheckbox->isChecked(), &selectedItems);

        if (allItemsDisenchanted && toShards)
        {
            _disenchantToShardsButton->setDisabled(true);
            _disenchantToSignetButton->setDisabled(true);
        }
        else // TUs may leave after disenchanting to signets in Ultimative
            updateDisenchantButtonsState();
    }
}

void ItemsViewerDialog::upgradeGems()
{
    currentSplitter()->upgradeGems();

    _upgradeGemsButton->setDisabled(true);
    _upgradeBothButton->setDisabled(true);
}

void ItemsViewerDialog::upgradeRunes()
{
    currentSplitter()->upgradeRunes(_reserveRunesSpinBox->value());

    _upgradeRunesButton->setDisabled(true);
    _upgradeBothButton->setDisabled(true);
}

void ItemsViewerDialog::sortStash()
{
    PlugyItemsSplitter *plugySplitter = currentPlugySplitter();
    StashSortingOptionsDialog dlg(plugySplitter->lastNotEmptyPage(), this);
    if (dlg.exec())
        plugySplitter->sortStash(dlg.sortOptions());
}

void ItemsViewerDialog::insertBlankPages()
{
    bool isAfter = sender() == _insertBlankPagesAfterButton;
    PlugyItemsSplitter *plugySplitter = currentPlugySplitter();
    if (isAfter && plugySplitter->currentPage() == plugySplitter->lastNotEmptyPage())
    {
        ERROR_BOX(tr("There's no sense in inserting blank pages after the last one."));
        return;
    }

    bool ok;
    int pages = QInputDialog::getInt(this, qApp->applicationName(), isAfter ? tr("Blank pages after current:") : tr("Blank pages before current:"), 1, 1, 1000, 1, &ok);
    if (ok)
        plugySplitter->insertBlankPages(pages, isAfter);
}

void ItemsViewerDialog::removeCurrentPage()
{
    currentPlugySplitter()->removeCurrentPage();
    updateRemoveCurrentBlankPageButtonState();
}

#define CURRENT_SHARED_STASH (CharacterInfo::instance().basicInfo.isHardcore ? Enums::ItemStorage::HCStash : Enums::ItemStorage::SharedStash)

void ItemsViewerDialog::moveItemBetweenStashes(ItemInfo *item)
{
    QList<Enums::ItemStorage::ItemStorageEnum> newStoragesToTry;
    if (qobject_cast<PlugyItemsSplitter *>(sender()))
        newStoragesToTry << Enums::ItemStorage::Stash << Enums::ItemStorage::Inventory << Enums::ItemStorage::Cube;
    else
        newStoragesToTry << CURRENT_SHARED_STASH;

    int oldStorage = item->storage;
    foreach (Enums::ItemStorage::ItemStorageEnum newStorage, newStoragesToTry)
    {
        item->storage = newStorage;
        int tab = tabIndexFromItemStorage(item->storage);
        ItemsPropertiesSplitter *splitter = splitterAtIndex(tab);
        if (!splitter->storeItemInStorage(item, item->storage, true))
            continue;

        splitter->setCellSpanForItem(item);
        _tabWidget->setTabEnabled(tab, true);
        itemCountChangedInTab(tab, splitter->itemCount());
        return;
    }

    item->storage = oldStorage;
    ERROR_BOX(tr("Unable to move selected item: not enough free space"));
}

void ItemsViewerDialog::moveCurrentItemsToSharedStash()
{
    ItemsList itemsToMove = currentSplitter()->itemsModel()->items();
    if (itemsToMove.isEmpty())
        return;

    Enums::ItemStorage::ItemStorageEnum storage = CURRENT_SHARED_STASH;
    int tab = tabIndexFromItemStorage(storage);
    PlugyItemsSplitter *plugySplitter = qobject_cast<PlugyItemsSplitter *>(splitterAtIndex(tab));
    plugySplitter->addItemsToLastPage(itemsToMove, storage);
    _tabWidget->setTabEnabled(tab, true);
    itemCountChangedInTab(tab, plugySplitter->itemCount());

    currentSplitter()->setItems(ItemsList());
    itemCountChangedInCurrentTab(0);
}

void ItemsViewerDialog::adjustHeight(bool isBoxExpanded)
{
    int heightDiff = (_itemManagementWidget->height() + 6) * (isBoxExpanded ? 1 : -1);
    resize(size() + QSize(0, heightDiff));
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

void ItemsViewerDialog::updateUpgradeButtonsState()
{
    QPair<bool, bool> allowUpgradeButtons = currentSplitter()->updateUpgradeButtonsState(_reserveRunesSpinBox->value());
    _upgradeGemsButton->setEnabled(allowUpgradeButtons.first);
    _upgradeRunesButton->setEnabled(allowUpgradeButtons.second);
    _upgradeBothButton->setEnabled(allowUpgradeButtons.first && allowUpgradeButtons.second);
}

void ItemsViewerDialog::updateStashButtonsState()
{
    _stashBox->setVisible(isPlugyStorageIndex(_tabWidget->currentIndex()));
    if (_stashBox->isVisible())
    {
        _sortStashButton->setEnabled(currentPlugySplitter()->itemCount() > 0);
        updateRemoveCurrentBlankPageButtonState();
    }
}

void ItemsViewerDialog::updateRemoveCurrentBlankPageButtonState()
{
    _removeCurrentBlankPage->setEnabled(!currentPlugySplitter()->pageItemCount());
}
