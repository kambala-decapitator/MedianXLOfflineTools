#include "itemspropertiessplitter.h"
#include "propertiesviewerwidget.h"
#include "itemdatabase.h"
#include "itemstoragetableview.h"
#include "itemstoragetablemodel.h"
#include "itemparser.h"
#include "resourcepathmanager.hpp"
#include "itemsviewerdialog.h"
#include "reversebitwriter.h"
#include "characterinfo.hpp"

#include <QMenu>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif

static const int kShardsPerCrystal = 5;


ItemsPropertiesSplitter::ItemsPropertiesSplitter(ItemStorageTableView *itemsView, QWidget *parent /*= 0*/) : QSplitter(Qt::Horizontal, parent), _itemsView(itemsView), _propertiesWidget(new PropertiesViewerWidget(parent))
{
    addWidget(_itemsView);
    addWidget(_propertiesWidget);

    createItemActions();

    setChildrenCollapsible(false);
    setStretchFactor(1, 5);

    _itemsView->setFocus();

    connect(_itemsView, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(showContextMenu(const QPoint &)));
}

void ItemsPropertiesSplitter::setModel(ItemStorageTableModel *model)
{
    _itemsModel = model;
    _itemsView->setModel(model);
    // TODO: [0.4] change signal to selectionChanged
    connect(_itemsView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), SLOT(itemSelected(const QModelIndex &)));
    connect(_itemsModel, SIGNAL(itemMoved(const QModelIndex &, const QModelIndex &)), SLOT(moveItem(const QModelIndex &, const QModelIndex &)));
}

void ItemsPropertiesSplitter::itemSelected(const QModelIndex &index)
{
    ItemInfo *item = _itemsModel->itemAtIndex(index);
    _propertiesWidget->showItem(item);

    // correctly disable hotkeys
    _itemActions[DisenchantShards]->setEnabled(ItemDataBase::canDisenchantIntoArcaneShards(item));
    _itemActions[DisenchantSignet]->setEnabled(ItemDataBase::canDisenchantIntoSignetOfLearning(item));
    // in Ultimative, Character Orb and Sunstone of Elements use same stat IDs as MOs, but those can't be removed
    _itemActions[RemoveMO]->setEnabled(_propertiesWidget->hasMysticOrbs() && !(isUltimative() && isCharacterOrbOrSunstoneOfElements(item)));
}

void ItemsPropertiesSplitter::moveItem(const QModelIndex &newIndex, const QModelIndex &oldIndex)
{
    ItemInfo *item = _itemsModel->itemAtIndex(newIndex);
    ReverseBitWriter::replaceValueInBitString(item->bitString, Enums::ItemOffsets::Column, item->column);
    ReverseBitWriter::replaceValueInBitString(item->bitString, Enums::ItemOffsets::Row,    item->row);
    item->hasChanged = true;

    if (_itemsView->rowSpan(oldIndex.row(), oldIndex.column()) > 1 || _itemsView->columnSpan(oldIndex.row(), oldIndex.column()) > 1)
        _itemsView->setSpan(oldIndex.row(), oldIndex.column(), 1, 1);
    _itemsView->setCellSpanForItem(item);
    _itemsView->setCurrentIndex(newIndex);

    emit itemsChanged();
}

void ItemsPropertiesSplitter::showItem(ItemInfo *item)
{
    if (item)
        _itemsView->setCurrentIndex(_itemsModel->index(item->row, item->column));
}

void ItemsPropertiesSplitter::showFirstItem()
{
    // sometimes item is selected, but there's no visual selection
    if (_itemsView->selectionModel()->selectedIndexes().isEmpty() || !selectedItem(false))
        showItem(_itemsModel->firstItem());
}

void ItemsPropertiesSplitter::setItems(const ItemsList &newItems)
{
    _allItems = newItems;
    updateItems(_allItems);
}

QPair<bool, bool> ItemsPropertiesSplitter::updateDisenchantButtonsState(bool includeUniques, bool includeSets, bool toCrystals, ItemsList *items /*= 0*/)
{
    bool allowShards = false, allowSignets = false;
    int shards = 0;
    foreach (ItemInfo *item, items ? *items : _allItems)
    {
        if (toCrystals)
        {
            if (isArcaneShard(item))
                ++shards;
            else if (isArcaneShard2(item))
                shards += 2;
            else if (isArcaneShard3(item))
                shards += 3;
            else if (isArcaneShard4(item))
                shards += 4;
        }

        if ((includeUniques && item->quality == Enums::ItemQuality::Unique) || (includeSets && item->quality == Enums::ItemQuality::Set))
        {
            if (!allowShards)
                allowShards = ItemDataBase::canDisenchantIntoArcaneShards(item);
            if (!allowSignets)
                allowSignets = ItemDataBase::canDisenchantIntoSignetOfLearning(item);
            if (allowShards && allowSignets)
                break;
        }
    }
    // allow just upgrading shards to crystals
    if (toCrystals && !allowShards && shards >= kShardsPerCrystal)
        allowShards = true;
    return qMakePair(allowShards, allowSignets);
}

QPair<bool, bool> ItemsPropertiesSplitter::updateUpgradeButtonsState(ItemsList *items /*= 0*/)
{
    return qMakePair(false, false);
}

void ItemsPropertiesSplitter::updateItems(const ItemsList &newItems)
{
    _propertiesWidget->clear();
    _itemsModel->setItems(newItems);

    _itemsView->clearSpans();
    foreach (ItemInfo *item, newItems)
        _itemsView->setCellSpanForItem(item);

    showFirstItem();
}

void ItemsPropertiesSplitter::showContextMenu(const QPoint &pos)
{
    ItemInfo *item = selectedItem(false);
    if (item)
    {
        QList<QAction *> actions;

        // TODO: 0.4
        //QMenu *menuExport = new QMenu(tr("Export as"), _itemsView);
        //menuExport->addActions(QList<QAction *>() << _itemActions[ExportBbCode] << _itemActions[ExportHtml]);
        //actions << menuExport->menuAction() << separator();

        if (_itemActions[DisenchantShards]->isEnabled())
        {
            QMenu *menuDisenchant = new QMenu(tr("Disenchant into"), _itemsView);
            if (_itemActions[DisenchantSignet]->isEnabled())
                menuDisenchant->addAction(_itemActions[DisenchantSignet]);
            if (_itemActions[DisenchantShards]->isEnabled())
                menuDisenchant->addAction(_itemActions[DisenchantShards]);
            actions << menuDisenchant->menuAction();
        }

        // TODO: 0.4
//        if (item->isSocketed && item->socketablesNumber)
//            actions << _itemActions[Unsocket];

        // no need
        //if (item->isEthereal)
        //{
        //    QAction *actionMakeNonEthereal = new QAction(tr("Make Non-Ethereal"), _itemsView);
        //    connect(actionMakeNonEthereal, SIGNAL(triggered()), SLOT(makeNonEthereal()));
        //    actions << actionMakeNonEthereal;
        //}

        if (_itemActions[RemoveMO]->isEnabled())
        {
            QAction *actionToAdd = 0;
            if (_propertiesWidget->mysticOrbsTotal() > 1)
            {
                QMenu *menuMO = new QMenu(_itemsView);
                _itemActions[RemoveMO]->setText(tr("All"));
                menuMO->addActions(QList<QAction *>() << _itemActions[RemoveMO] << separator());

                createActionsForMysticOrbs(menuMO, true, item);
                menuMO->addAction(separator());
                createActionsForMysticOrbs(menuMO, false, item);

                actionToAdd = menuMO->menuAction();
            }
            else
                actionToAdd = _itemActions[RemoveMO];

            actionToAdd->setText(tr("Remove Mystic Orbs"));
            actions << actionToAdd;
        }

        actions << separator() << _itemActions[Delete];

        QMenu::exec(actions, _itemsView->mapToGlobal(pos));
    }
}

ItemInfo *ItemsPropertiesSplitter::selectedItem(bool showError /*= true*/)
{
    //QModelIndexList selectedIndexes = _itemsView->selectionModel()->selectedIndexes();
    ItemInfo *item = _itemsModel->itemAtIndex(_itemsView->selectionModel()->currentIndex());
    if (!item && showError)
        ERROR_BOX("TROLOLOL no item selection found");
    return item;
}

void ItemsPropertiesSplitter::exportText()
{

}

void ItemsPropertiesSplitter::disenchantSelectedItem()
{
    QAction *action = qobject_cast<QAction *>(sender());
    ItemInfo *item = selectedItem();
    if (!action || !item)
    {
        ERROR_BOX("TROLOLOL I can't disenchant the item");
        return;
    }

    ItemInfo *newItem = ItemDataBase::loadItemFromFile(action->objectName() == "signet" ? "signet_of_learning" : "arcane_shard");
    ItemInfo *newItemStored = disenchantItemIntoItem(item, newItem);
    delete newItem;

    if (newItemStored) // let's be safe
        _itemsView->setCurrentIndex(_itemsModel->index(newItemStored->row, newItemStored->column));
}

//void ItemsPropertiesSplitter::unsocketItem()
//{

//}

//void ItemsPropertiesSplitter::makeNonEthereal()
//{
//    ItemInfo *item = selectedItem();
//    if (item)
//    {
//        item->hasChanged = true;
//
//        item->isEthereal = false;
//        ReverseBitWriter::replaceValueInBitString(item->bitString, Enums::ItemOffsets::Ethereal, 0);
//
//        if (ItemDataBase::Items()->operator[](item->itemType).genericType == Enums::ItemTypeGeneric::Armor)
//        {
//            // TODO if will bring back: find the correct position for replacement (the defense offset isn't static)
//            item->defense /= 1.5;
//            const ItemPropertyTxt &defenceProp = ItemDataBase::Properties()->operator[](Enums::ItemProperties::Defence);
//            ReverseBitWriter::replaceValueInBitString(item->bitString, Enums::ItemOffsets::Ethereal, defenceProp.bits, item->defense + defenceProp.add);
//        }
//
//        _propertiesWidget->displayItemProperties(item);
//    }
//}

void ItemsPropertiesSplitter::deleteItem()
{
    ItemInfo *item = selectedItem(false);
    if (item && QUESTION_BOX_YESNO(tr("Are you sure you want to delete this item?"), QMessageBox::No) == QMessageBox::Yes)
    {
        bool isCube = ItemDataBase::isCube(item);
        if (isCube && !ItemDataBase::itemsStoredIn(Enums::ItemStorage::Cube).isEmpty())
            if (QUESTION_BOX_YESNO(tr("Cube is not empty. Do you really want to delete it?\nNote: items inside will be preserved. You can recover them by getting new Cube."), QMessageBox::No) == QMessageBox::No)
                return;

        performDeleteItem(item);
        showFirstItem();

        emit itemCountChanged(_allItems.size());
        emit itemDeleted();
        if (isCube)
            emit cubeDeleted();

        // a hack to make stash modified
        ItemInfo *someItem = _itemsModel->firstItem();
        if (someItem)
            someItem->hasChanged = true;
    }
}

void ItemsPropertiesSplitter::performDeleteItem(ItemInfo *item, bool emitSignal /*= true*/)
{
    // TODO: [0.4] add option to unsocket at first
    removeItemFromList(item, emitSignal);
    qDeleteAll(item->socketablesInfo);
    delete item;
}

void ItemsPropertiesSplitter::addItemToList(ItemInfo *item, bool emitSignal /*= true*/)
{
    if (!CharacterInfo::instance().items.character.contains(item))
        CharacterInfo::instance().items.character.append(item);

    if (!_allItems.contains(item))
        _allItems.append(item);
    if (isItemInCurrentStorage(item))
    {
        _itemsModel->addItem(item);
        if (selectedItem(false) == item) // signal is emitted only when single item is disenchanted (through context menu), so we don't need extra parameter for just another name
            _propertiesWidget->showItem(item);
    }

    if (emitSignal)
        emit itemsChanged();
}

void ItemsPropertiesSplitter::removeItemFromList(ItemInfo *item, bool emitSignal /*= true*/)
{
    CharacterInfo::instance().items.character.removeOne(item);

    _allItems.removeOne(item);
    if (selectedItem(false) == item)
        _propertiesWidget->clear();
    if (isItemInCurrentStorage(item))
    {
        _itemsModel->removeItem(item);
        if (_itemsView->rowSpan(item->row, item->column) > 1 || _itemsView->columnSpan(item->row, item->column) > 1)
            _itemsView->setSpan(item->row, item->column, 1, 1);
    }

    if (emitSignal)
        emit itemsChanged();
}

void ItemsPropertiesSplitter::disenchantAllItems(bool toShards, bool upgradeToCrystals, bool eatSignets, bool includeUniques, bool includeSets, ItemsList *items /*= 0*/)
{
    ItemInfo *disenchantedItem = ItemDataBase::loadItemFromFile(toShards ? "arcane_shard" : "signet_of_learning");
    ItemsList &items_ = items ? *items : _allItems;
    foreach (ItemInfo *item, items_)
    {
        if ((item->quality == Enums::ItemQuality::Unique && includeUniques) || (item->quality == Enums::ItemQuality::Set && includeSets))
        {
            if ((toShards && ItemDataBase::canDisenchantIntoArcaneShards(item)) || (!toShards && ItemDataBase::canDisenchantIntoSignetOfLearning(item)))
            {
                // if (!toShards && eatSignets) // TODO: don't create new items in this case
                disenchantItemIntoItem(item, disenchantedItem, false);
            }
        }
    }

    if (toShards && upgradeToCrystals)
    {
        int shards = 0;
        foreach (ItemInfo *item, items_)
        {
            if (isArcaneShard(item))
                ++shards;
            else if (isArcaneShard2(item))
                shards += 2;
            else if (isArcaneShard3(item))
                shards += 3;
            else if (isArcaneShard4(item))
                shards += 4;
        }

        int crystals = shards / kShardsPerCrystal;
        if (crystals)
        {
            int storage = items_.first()->storage;
            int i = 0;
            foreach (ItemInfo *item, items_)
                if (isArcaneShard(item) || isArcaneShard2(item) || isArcaneShard3(item) || isArcaneShard4(item))
                    performDeleteItem(item, false);

            ItemInfo *crystal = ItemDataBase::loadItemFromFile("arcane_crystal");
            for (int i = 0; i < crystals; ++i)
            {
                ItemInfo *crystalCopy = new ItemInfo(*crystal);
                storeItemInStorage(crystalCopy, storage);
            }
            delete crystal;

            int shardsLeft = shards - crystals * kShardsPerCrystal;
            for (int i = 0; i < shardsLeft; ++i)
            {
                ItemInfo *shard = new ItemInfo(*disenchantedItem);
                storeItemInStorage(shard, storage);
            }

            emit itemCountChanged(items_.size());
        }
    }

    delete disenchantedItem;
    emit itemsChanged();
    _itemsView->viewport()->update();
}

ItemInfo *ItemsPropertiesSplitter::disenchantItemIntoItem(ItemInfo *oldItem, ItemInfo *newItem, bool emitSignal /*= true*/)
{
    ItemsList items = ItemDataBase::itemsStoredIn(oldItem->storage, oldItem->location, oldItem->plugyPage ? &oldItem->plugyPage : 0);
    items.removeOne(oldItem);
    ItemInfo *newItemCopy = new ItemInfo(*newItem); // it's safe because there're no properties and no socketables
    if (!ItemDataBase::canStoreItemAt(oldItem->row, oldItem->column, newItemCopy->itemType, items, ItemsViewerDialog::rowsInStorageAtIndex(oldItem->storage)))
    {
        ERROR_BOX("If you see this text (which you shouldn't), please tell me which item you've just tried to disenchant");
        delete newItemCopy;
        return 0;
    }

    newItemCopy->row = oldItem->row;
    newItemCopy->column = oldItem->column;
    newItemCopy->storage = oldItem->storage;
    newItemCopy->whereEquipped = oldItem->whereEquipped;
    newItemCopy->plugyPage = oldItem->plugyPage;

    // update bits
    bool isPlugyStorage = newItemCopy->storage == Enums::ItemStorage::PersonalStash || newItemCopy->storage == Enums::ItemStorage::SharedStash || newItemCopy->storage == Enums::ItemStorage::HCStash;
    ReverseBitWriter::replaceValueInBitString(newItemCopy->bitString, Enums::ItemOffsets::Storage, isPlugyStorage ? Enums::ItemStorage::Stash : newItemCopy->storage);
    ReverseBitWriter::replaceValueInBitString(newItemCopy->bitString, Enums::ItemOffsets::Column,     newItemCopy->column);
    ReverseBitWriter::replaceValueInBitString(newItemCopy->bitString, Enums::ItemOffsets::Row,        newItemCopy->row);
//    ReverseBitWriter::replaceValueInBitString(newItemCopy->bitString, Enums::ItemOffsets::EquipIndex, newItemCopy->whereEquipped);
//    ReverseBitWriter::replaceValueInBitString(newItem->bitString, Enums::ItemOffsets::Location, newItem->location);

    //if (newItem->storage == Enums::ItemStorage::Stash)
    //{
    //    // plugy saves last opened page as the stash page, so we should find the correct page
    //    ItemInfo *plugyItem = 0;
    //    for (int i = Enums::ItemStorage::PersonalStash; i <= Enums::ItemStorage::HCStash; ++i)
    //    {
    //        ItemsList plugyStashItems = ItemParser::itemsStoredIn(i);
    //        foreach (ItemInfo *plugyStashItem, plugyStashItems)
    //        {
    //            if (plugyStashItem->bitString == newItem->bitString)
    //            {
    //                plugyItem = plugyStashItem;
    //                break;
    //            }
    //        }
    //        if (plugyItem)
    //            break;
    //    }

    //    if (plugyItem)
    //    {
    //        ItemInfo *copy = new ItemInfo(*newItem);
    //        copy->storage = plugyItem->storage;
    //        copy->plugyPage = plugyItem->plugyPage;

    //        performDeleteItem(plugyItem, false);
    //        addItemToList(copy, false);
    //        //emit storageModified(copy->storage);
    //    }
    //}
    //else if (newItem->storage >= Enums::ItemStorage::PersonalStash && newItem->storage <= Enums::ItemStorage::HCStash)
    //{
    //    ItemsList stashItems = ItemParser::itemsStoredIn(Enums::ItemStorage::Stash);
    //    if (stashItems.indexOf(item) != -1)
    //    {
    //        ItemInfo *copy = new ItemInfo(*newItem);
    //        copy->storage = Enums::ItemStorage::Stash;
    //        copy->plugyPage = 0;
    //    }
    //}

    performDeleteItem(oldItem, emitSignal);
    addItemToList(newItemCopy, emitSignal);

    return newItemCopy;
}

bool ItemsPropertiesSplitter::storeItemInStorage(ItemInfo *item, int storage)
{
    bool result = ItemDataBase::storeItemIn(item, static_cast<Enums::ItemStorage::ItemStorageEnum>(storage), ItemsViewerDialog::rowsInStorageAtIndex(storage));
    if (!result)
        qDebug() << "failed to store" << ItemDataBase::Items()->operator [](item->itemType).name;
    else
        addItemToList(item, false);
    return result;
}

void ItemsPropertiesSplitter::createItemActions()
{
    //QAction *actionBbCode = new QAction("BBCode", _itemsView);
    //actionBbCode->setShortcut(QKeySequence("Ctrl+E"));
    //actionBbCode->setObjectName("bbcode");
    //connect(actionBbCode, SIGNAL(triggered()), SLOT(exportText()));
    //_itemsView->addAction(actionBbCode);
    //_itemActions[ExportBbCode] = actionBbCode;

    //QAction *actionHtml = new QAction("HTML", _itemsView);
    //actionHtml->setShortcut(QKeySequence("Alt+E"));
    //actionHtml->setObjectName("html");
    //connect(actionHtml, SIGNAL(triggered()), SLOT(exportText()));
    //_itemsView->addAction(actionHtml);
    //_itemActions[ExportHtml] = actionHtml;

    QAction *actionSol = new QAction(QIcon(ResourcePathManager::pathForImageName("sigil1b")), tr("Signet of Learning"), _itemsView);
    actionSol->setShortcut(QKeySequence("Ctrl+D"));
    actionSol->setObjectName("signet");
    connect(actionSol, SIGNAL(triggered()), SLOT(disenchantSelectedItem()));
    _itemsView->addAction(actionSol);
    _itemActions[DisenchantSignet] = actionSol;

    QAction *actionShards = new QAction(QIcon(ResourcePathManager::pathForImageName("invfary4")), tr("Arcane Shards"), _itemsView);
    actionShards->setShortcut(QKeySequence("Alt+D"));
    actionShards->setObjectName("shards");
    connect(actionShards, SIGNAL(triggered()), SLOT(disenchantSelectedItem()));
    _itemsView->addAction(actionShards);
    _itemActions[DisenchantShards] = actionShards;

    // TODO: [0.4] unsocket
//    QAction *actionUnsocket = new QAction(tr("Unsocket"), _itemsView);
//    connect(actionUnsocket, SIGNAL(triggered()), SLOT(unsocketItem()));
//    _itemsView->addAction(actionUnsocket);
//    _itemActions[Unsocket] = actionUnsocket;

    QAction *actionRemoveMO = new QAction(_itemsView);
    actionRemoveMO->setShortcut(QKeySequence("Ctrl+M"));
    connect(actionRemoveMO, SIGNAL(triggered()), _propertiesWidget, SLOT(removeAllMysticOrbs()));
    connect(actionRemoveMO, SIGNAL(triggered()), SIGNAL(itemsChanged()));
    _itemsView->addAction(actionRemoveMO);
    _itemActions[RemoveMO] = actionRemoveMO;

    QAction *actionDelete = new QAction(tr("Delete"), _itemsView);
    actionDelete->setShortcut(
#ifdef Q_WS_MACX
                Qt::Key_Backspace
#else
                QKeySequence::Delete
#endif
                );
    connect(actionDelete, SIGNAL(triggered()), SLOT(deleteItem()));
    _itemsView->addAction(actionDelete);
    _itemActions[Delete] = actionDelete;
}

QAction *ItemsPropertiesSplitter::separator()
{
    QAction *sep = new QAction(_itemsView);
    sep->setSeparator(true);
    return sep;
}

void ItemsPropertiesSplitter::createActionsForMysticOrbs(QMenu *parentMenu, bool isItemMO, ItemInfo *item)
{
    foreach (int moCode, _propertiesWidget->mysticOrbs(isItemMO))
    {
        QAction *moAction = new QAction((isItemMO ? item->props : item->rwProps).value(moCode).displayString, _itemsView);
        moAction->setProperty("isItemMO", isItemMO);
        moAction->setProperty("moCode", moCode);
        connect(moAction, SIGNAL(triggered()), _propertiesWidget, SLOT(removeMysticOrb()));
        connect(moAction, SIGNAL(triggered()), SIGNAL(itemsChanged()));
        parentMenu->addAction(moAction);
    }
}
