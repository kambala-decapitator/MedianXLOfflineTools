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


ItemsPropertiesSplitter::ItemsPropertiesSplitter(ItemStorageTableView *itemsView, QWidget *parent) : QSplitter(Qt::Horizontal, parent), _itemsView(itemsView)
{
    addWidget(_itemsView);
    _propertiesWidget = new PropertiesViewerWidget(parent);
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
    // TODO 0.3: change signal to selectionChanged
    connect(_itemsView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), SLOT(itemSelected(const QModelIndex &)));
    connect(_itemsModel, SIGNAL(itemMoved(const QModelIndex &, const QModelIndex &)), SLOT(moveItem(const QModelIndex &, const QModelIndex &)));
}

void ItemsPropertiesSplitter::itemSelected(const QModelIndex &index)
{
    ItemInfo *item = _itemsModel->itemAtIndex(index);
    _propertiesWidget->showItem(item);

    // correctly disable hotkeys
    bool isUltimative_ = isUltimative();

    if (item && item->location != Enums::ItemLocation::Equipped && (item->quality == Enums::ItemQuality::Set || (item->quality == Enums::ItemQuality::Unique && !ItemDataBase::isUberCharm(item) && ItemDataBase::Uniques()->contains(item->setOrUniqueId))))
    {
        _itemActions[DisenchantShards]->setDisabled(isUltimative_ && item->props.contains(276)); // prohibit disenchanting TUs from the Gift Box into shards. 276 is ID of item-duped property
        // Ultimative prohibits disenchanting TUs into signets
        _itemActions[DisenchantSignet]->setDisabled(isUltimative_ && item->quality == Enums::ItemQuality::Unique && ItemDataBase::Items()->value(item->itemType).genericType != Enums::ItemTypeGeneric::Misc && !isSacred(item));
    }
    else
    {
        _itemActions[DisenchantShards]->setDisabled(true);
        _itemActions[DisenchantSignet]->setDisabled(true);
    }

    // in Ultimative, Character Orb and Sunstone of Elements use same stat IDs as MOs, but those can't be removed
    _itemActions[RemoveMO]->setEnabled(_propertiesWidget->hasMysticOrbs() && !(isUltimative_ && isCharacterOrbOrSunstoneOfElements(item)));
}

void ItemsPropertiesSplitter::moveItem(const QModelIndex &newIndex, const QModelIndex &oldIndex)
{
    ItemInfo *item = _itemsModel->itemAtIndex(newIndex);
    ReverseBitWriter::replaceValueInBitString(item->bitString, Enums::ItemOffsets::Column, item->column);
    ReverseBitWriter::replaceValueInBitString(item->bitString, Enums::ItemOffsets::Row,    item->row);
    item->hasChanged = true;

    _itemsView->setSpan(oldIndex.row(), oldIndex.column(), 1, 1);
    setCellSpanForItem(item);
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

void ItemsPropertiesSplitter::updateItems(const ItemsList &newItems)
{
    _propertiesWidget->clear();
    _itemsModel->setItems(newItems);

    _itemsView->clearSpans();
    foreach (ItemInfo *item, newItems)
        setCellSpanForItem(item);

    showFirstItem();
}

void ItemsPropertiesSplitter::showContextMenu(const QPoint &pos)
{
    ItemInfo *item = selectedItem(false);
    if (item)
    {
        QList<QAction *> actions;

        // TODO 0.3
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

        // TODO 0.3
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
            {
                actionToAdd = _itemActions[RemoveMO];
            }

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

void ItemsPropertiesSplitter::setCellSpanForItem(ItemInfo *item)
{
    const ItemBase &itemBase = ItemDataBase::Items()->value(item->itemType);
    if (itemBase.height > 1 || itemBase.width > 1) // to prevent warnings to the console
        _itemsView->setSpan(item->row, item->column, itemBase.height, itemBase.width);
}

void ItemsPropertiesSplitter::exportText()
{

}

void ItemsPropertiesSplitter::disenchantItem()
{
    QAction *action = qobject_cast<QAction *>(sender());
    ItemInfo *item = selectedItem();
    if (!action || !item)
    {
        ERROR_BOX("TROLOLOL I can't disenchant the item");
        return;
    }

    ItemsList items = ItemDataBase::itemsStoredIn(item->storage, item->location, item->plugyPage ? &item->plugyPage : 0);
    items.removeOne(item);
    ItemInfo *newItem = ItemDataBase::loadItemFromFile(action->objectName() == "signet" ? "signet_of_learning" : "arcane_shard");
    if (!ItemDataBase::canStoreItemAt(item->row, item->column, newItem->itemType, items, ItemsViewerDialog::kRows.at(ItemsViewerDialog::tabIndexFromItemStorage(item->storage))))
    {
        ERROR_BOX("If you see this text (which you shouldn't), please tell me which item you've just tried to disenchant");
        delete newItem;
        return;
    }

    newItem->row = item->row;
    newItem->column = item->column;
    newItem->storage = item->storage;
    newItem->whereEquipped = item->whereEquipped;
    //newItem->location = Enums::ItemLocation::Stored;
    newItem->plugyPage = item->plugyPage;

    // update bits
    bool isPlugyStorage = newItem->storage == Enums::ItemStorage::PersonalStash || newItem->storage == Enums::ItemStorage::SharedStash || newItem->storage == Enums::ItemStorage::HCStash;
    ReverseBitWriter::replaceValueInBitString(newItem->bitString, Enums::ItemOffsets::Storage,    isPlugyStorage ? Enums::ItemStorage::Stash : newItem->storage);
    ReverseBitWriter::replaceValueInBitString(newItem->bitString, Enums::ItemOffsets::Column,     newItem->column);
    ReverseBitWriter::replaceValueInBitString(newItem->bitString, Enums::ItemOffsets::Row,        newItem->row);
    ReverseBitWriter::replaceValueInBitString(newItem->bitString, Enums::ItemOffsets::EquipIndex, newItem->whereEquipped);
    //ReverseBitWriter::replaceValueInBitString(newItem->bitString, Enums::ItemOffsets::Location, newItem->location);

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

    performDeleteItem(item);
    addItemToList(newItem);

    _itemsView->setCurrentIndex(_itemsModel->index(newItem->row, newItem->column));
}

void ItemsPropertiesSplitter::unsocketItem()
{

}

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
//        if (ItemDataBase::Items()->value(item->itemType).genericType == Enums::ItemTypeGeneric::Armor)
//        {
//            // TODO if will bring back: find the correct position for replacement (the defense offset isn't static)
//            item->defense /= 1.5;
//            const ItemPropertyTxt &defenceProp = ItemDataBase::Properties()->value(Enums::ItemProperties::Defence);
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

void ItemsPropertiesSplitter::performDeleteItem(ItemInfo *item, bool currentStorage /*= true*/)
{
    // TODO 0.3: add option to unsocket at first
    removeItemFromList(item, currentStorage);
    qDeleteAll(item->socketablesInfo);
    delete item;
}

void ItemsPropertiesSplitter::addItemToList(ItemInfo *item, bool currentStorage /*= true*/)
{
    CharacterInfo::instance().items.character.append(item);
    
    if (currentStorage)
    {
        _allItems.append(item);
        _itemsModel->addItem(item);
        _propertiesWidget->showItem(item);
    }

    emit itemsChanged();
}

void ItemsPropertiesSplitter::removeItemFromList(ItemInfo *item, bool currentStorage /*= true*/)
{
    CharacterInfo::instance().items.character.removeOne(item);
    
    if (currentStorage)
    {
        _propertiesWidget->clear();

        _allItems.removeOne(item);
        _itemsModel->removeItem(item);
        _itemsView->setSpan(item->row, item->column, 1, 1);
    }

    emit itemsChanged();
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
    connect(actionSol, SIGNAL(triggered()), SLOT(disenchantItem()));
    _itemsView->addAction(actionSol);
    _itemActions[DisenchantSignet] = actionSol;

    QAction *actionShards = new QAction(QIcon(ResourcePathManager::pathForImageName("invfary4")), tr("Arcane Shards"), _itemsView);
    actionShards->setShortcut(QKeySequence("Alt+D"));
    actionShards->setObjectName("shards");
    connect(actionShards, SIGNAL(triggered()), SLOT(disenchantItem()));
    _itemsView->addAction(actionShards);
    _itemActions[DisenchantShards] = actionShards;

    // TODO 0.4: unsocket
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
