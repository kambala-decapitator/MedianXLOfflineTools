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

#include <QPushButton>
#include <QDoubleSpinBox>
#include <QKeyEvent>
#include <QMenu>

#include <qmath.h>

#include <limits>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


static const QString iconPathFormat(":/PlugyArrows/icons/plugy/%1.png");

ItemsPropertiesSplitter::ItemsPropertiesSplitter(ItemStorageTableView *itemsView, bool shouldCreateNavigation, QWidget *parent) : QSplitter(Qt::Horizontal, parent), _itemsView(itemsView)
{
    if (shouldCreateNavigation)
    {
        _left10Button = new QPushButton(this);
        _leftButton = new QPushButton(this);
        _rightButton = new QPushButton(this);
        _right10Button = new QPushButton(this);

        QList<QPushButton *> buttons = QList<QPushButton *>() << _left10Button << _leftButton << _rightButton << _right10Button;
        foreach (QPushButton *button, buttons)
            button->setIconSize(QSize(32, 20));
        // hacky way to set button icons
        QKeyEvent keyEvent(QEvent::KeyRelease, Qt::Key_Shift, 0);
        keyReleaseEvent(&keyEvent);

        _pageSpinBox = new QDoubleSpinBox(this);
        _pageSpinBox->setDecimals(0);
        _pageSpinBox->setPrefix(tr("Page #"));
        _pageSpinBox->setRange(1, (std::numeric_limits<quint32>::max)());
        _pageSpinBox->setValue(1);
        _pageSpinBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        QHBoxLayout *hlayout = new QHBoxLayout;
        hlayout->addWidget(_left10Button);
        hlayout->addWidget(_leftButton);
        hlayout->addWidget(_pageSpinBox);
        hlayout->addWidget(_rightButton);
        hlayout->addWidget(_right10Button);
        // glue everything together (used mainly for Mac OS X)
        hlayout->setSpacing(0);
        hlayout->setContentsMargins(QMargins());

        QWidget *w = new QWidget(this);
        QVBoxLayout *vlayout = new QVBoxLayout(w);
        vlayout->addWidget(_itemsView);
        vlayout->setSpacing(0);
        vlayout->setContentsMargins(QMargins());
        vlayout->addLayout(hlayout);

        addWidget(w);
    }
    else
    {
        _left10Button = _leftButton = _rightButton = _right10Button = 0;
        addWidget(_itemsView);
    }

    _propertiesWidget = new PropertiesViewerWidget(parent);
    addWidget(_propertiesWidget);

    createItemActions();

    setChildrenCollapsible(false);
    setStretchFactor(1, 5);

    connect(_itemsView, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(showContextMenu(const QPoint &)));

    if (shouldCreateNavigation)
    {
        connect(_pageSpinBox, SIGNAL(valueChanged(double)), SLOT(updateItemsForCurrentPage()));

        connect(_left10Button,  SIGNAL(clicked()), SLOT(left10Clicked()));
        connect(_leftButton,    SIGNAL(clicked()), SLOT(leftClicked()));
        connect(_rightButton,   SIGNAL(clicked()), SLOT(rightClicked()));
        connect(_right10Button, SIGNAL(clicked()), SLOT(right10Clicked()));
    }

    _itemsView->setFocus();
}

void ItemsPropertiesSplitter::setModel(ItemStorageTableModel *model)
{
    _itemsModel = model;
    _itemsView->setModel(model);
    connect(_itemsView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), SLOT(itemSelected(const QModelIndex &)));
}

void ItemsPropertiesSplitter::keyPressEvent(QKeyEvent *keyEvent)
{
    if (_left10Button && keyEventHasShift(keyEvent))
    {
        _left10Button->setIcon(QIcon(iconPathFormat.arg("left100")));
        setShortcutTextInButtonTooltip(_left10Button, Qt::ALT + Qt::SHIFT + Qt::Key_Left);

        _leftButton->setIcon(QIcon(iconPathFormat.arg("first")));
        setShortcutTextInButtonTooltip(_leftButton, Qt::CTRL + Qt::SHIFT + Qt::Key_Left);

        _rightButton->setIcon(QIcon(iconPathFormat.arg("last")));
        setShortcutTextInButtonTooltip(_rightButton, Qt::CTRL + Qt::SHIFT + Qt::Key_Right);

        _right10Button->setIcon(QIcon(iconPathFormat.arg("right100")));
        setShortcutTextInButtonTooltip(_right10Button, Qt::ALT + Qt::SHIFT + Qt::Key_Right);

        _isShiftPressed = true;
    }
    QSplitter::keyPressEvent(keyEvent);
}

void ItemsPropertiesSplitter::keyReleaseEvent(QKeyEvent *keyEvent)
{
    if (_left10Button && keyEventHasShift(keyEvent))
    {
        _left10Button->setIcon(QIcon(iconPathFormat.arg("left10")));
        setShortcutTextInButtonTooltip(_left10Button, Qt::ALT + Qt::Key_Left);

        _leftButton->setIcon(QIcon(iconPathFormat.arg("left")));
        setShortcutTextInButtonTooltip(_leftButton, Qt::CTRL + Qt::Key_Left);

        _rightButton->setIcon(QIcon(iconPathFormat.arg("right")));
        setShortcutTextInButtonTooltip(_rightButton, Qt::CTRL + Qt::Key_Right);

        _right10Button->setIcon(QIcon(iconPathFormat.arg("right10")));
        setShortcutTextInButtonTooltip(_right10Button, Qt::ALT + Qt::Key_Right);

        _isShiftPressed = false;
    }
    QSplitter::keyPressEvent(keyEvent);
}

bool ItemsPropertiesSplitter::keyEventHasShift(QKeyEvent *keyEvent)
{
    return keyEvent->key() == Qt::Key_Shift || keyEvent->modifiers() & Qt::SHIFT;
}

void ItemsPropertiesSplitter::setShortcutTextInButtonTooltip(QPushButton *button, const QKeySequence &keySequence)
{
    button->setToolTip(keySequence.toString(QKeySequence::NativeText));
}

void ItemsPropertiesSplitter::itemSelected(const QModelIndex &index)
{
    ItemInfo *item = _itemsModel->itemAt(index);
    _propertiesWidget->showItem(item);

    bool allowDisnechant = !item ? false : (item->quality == Enums::ItemQuality::Set || (item->quality == Enums::ItemQuality::Unique && !ItemDataBase::isUberCharm(item))) && item->location != Enums::ItemLocation::Equipped;
    _itemActions[DisenchantShards]->setEnabled(allowDisnechant);
    _itemActions[DisenchantSignet]->setEnabled(allowDisnechant);
    _itemActions[RemoveMO]->setEnabled(_propertiesWidget->hasMysticOrbs());
}

void ItemsPropertiesSplitter::showItem(ItemInfo *item)
{
    if (item)
    {
        if (item->plugyPage)
            _pageSpinBox->setValue(item->plugyPage);
        _itemsView->setCurrentIndex(_itemsModel->index(item->row, item->column));
    }
}

void ItemsPropertiesSplitter::showFirstItem()
{
    // sometimes item is selected, but there's no visual selection
    if (_itemsView->selectionModel()->selectedIndexes().isEmpty() || !selectedItem(false))
        showItem(_itemsModel->firstItem());
}

bool compareItemsByPlugyPage(ItemInfo *a, ItemInfo *b)
{
    return a->plugyPage < b->plugyPage;
}

void ItemsPropertiesSplitter::setItems(const ItemsList &newItems)
{
    _allItems = newItems;
    if (_left10Button)
    {
        // using _allItems.last()->plugyPage would've been easy, but it's not always correct (new items obtained in the app are added to the end)
        ItemsList::iterator maxPageIter = std::max_element(_allItems.begin(), _allItems.end(), compareItemsByPlugyPage);
        _lastNotEmptyPage = maxPageIter == _allItems.end() ? 0 : (*maxPageIter)->plugyPage;//!_allItems.isEmpty() ? _allItems.last()->plugyPage : 0;
        
        _pageSpinBox->setSuffix(QString(" / %1").arg(_lastNotEmptyPage));
        _pageSpinBox->setRange(1, _lastNotEmptyPage);
        
        updateItemsForCurrentPage(false);
    }
    else
        updateItems(_allItems);
}

void ItemsPropertiesSplitter::updateItems(const ItemsList &newItems)
{
    _propertiesWidget->clear();
    _itemsModel->setItems(newItems);

    _itemsView->clearSpans();
    foreach (ItemInfo *item, newItems)
    {
        const ItemBase &itemBase = ItemDataBase::Items()->value(item->itemType);
        if (itemBase.height > 1 || itemBase.width > 1) // to prevent warnings to the console
            _itemsView->setSpan(item->row, item->column, itemBase.height, itemBase.width);
    }

    showFirstItem();
}

void ItemsPropertiesSplitter::updateItemsForCurrentPage(bool pageChanged /*= true*/)
{
    ItemsList pagedItems;
    foreach (ItemInfo *item, _allItems)
        if (item->plugyPage == static_cast<quint32>(_pageSpinBox->value()))
            pagedItems += item;
    updateItems(pagedItems);

    if (pageChanged)
        emit itemCountChanged(_allItems.size());
}

void ItemsPropertiesSplitter::leftClicked()
{
    if (_isShiftPressed)
        _pageSpinBox->setValue(1);
    else
        _pageSpinBox->stepDown();
}

void ItemsPropertiesSplitter::rightClicked()
{
    if (_isShiftPressed)
        _pageSpinBox->setValue(_lastNotEmptyPage);
    else
        _pageSpinBox->stepUp();
}

void ItemsPropertiesSplitter::left10Clicked()
{
    quint32 step = _isShiftPressed ? 100 : 10;
    _pageSpinBox->setValue(qFloor((_pageSpinBox->value() - 1) / step) * step);
}

void ItemsPropertiesSplitter::right10Clicked()
{
    quint32 step = _isShiftPressed ? 100 : 10;
    _pageSpinBox->setValue(qCeil((_pageSpinBox->value() + 1) / step) * step);
}

void ItemsPropertiesSplitter::showContextMenu(const QPoint &pos)
{
    ItemInfo *item = selectedItem(false);
    if (item)
    {
        QList<QAction *> actions;

        QMenu *menuExport = new QMenu(tr("Export as"), _itemsView);
        menuExport->addActions(QList<QAction *>() << _itemActions[ExportBbCode] << _itemActions[ExportHtml]);
        actions << menuExport->menuAction();

        QAction *separator = new QAction(_itemsView);
        separator->setSeparator(true);
        actions << separator;

        if (item->quality == Enums::ItemQuality::Set || (item->quality == Enums::ItemQuality::Unique && !ItemDataBase::isUberCharm(item)))
        {
            QMenu *menuDisenchant = new QMenu(tr("Disenchant into"), _itemsView);
            menuDisenchant->addActions(QList<QAction *>() << _itemActions[DisenchantSignet] << _itemActions[DisenchantShards]);
            menuDisenchant->menuAction()->setDisabled(item->location == Enums::ItemLocation::Equipped); // you can't disenchant equipped items
            actions << menuDisenchant->menuAction();
        }

        // TODO 0.4
//        if (item->isSocketed && item->socketablesNumber)
//            actions << _itemActions[Unsocket];

        // no need
        //if (item->isEthereal)
        //{
        //    QAction *actionMakeNonEthereal = new QAction(tr("Make Non-Ethereal"), _itemsView);
        //    connect(actionMakeNonEthereal, SIGNAL(triggered()), SLOT(makeNonEthereal()));
        //    actions << actionMakeNonEthereal;
        //}

        if (_propertiesWidget->hasMysticOrbs())
            actions << _itemActions[RemoveMO];

        separator = new QAction(_itemsView);
        separator->setSeparator(true);
        actions << separator << _itemActions[Delete];

        QMenu::exec(actions, _itemsView->mapToGlobal(pos));
    }
}

ItemInfo *ItemsPropertiesSplitter::selectedItem(bool showError /*= true*/)
{
    ItemInfo *item = _itemsModel->itemAt(_itemsView->selectionModel()->currentIndex());
    if (!item && showError)
        ERROR_BOX("TROLOLOL no item selection found");
    return item;
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
    if (!ItemDataBase::canStoreItemAt(item->row, item->column, newItem->itemType, items, ItemsViewerDialog::rows.at(ItemsViewerDialog::tabIndexFromItemStorage(item->storage))))
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
    ReverseBitWriter::replaceValueInBitString(newItem->bitString, Enums::ItemOffsets::Storage, isPlugyStorage ? Enums::ItemStorage::Stash : newItem->storage);
    ReverseBitWriter::replaceValueInBitString(newItem->bitString, Enums::ItemOffsets::Column, newItem->column);
    ReverseBitWriter::replaceValueInBitString(newItem->bitString, Enums::ItemOffsets::Row, newItem->row);
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
    QAction *actionBbCode = new QAction("BBCode", _itemsView);
    actionBbCode->setShortcut(QKeySequence("Ctrl+E"));
    actionBbCode->setObjectName("bbcode");
    connect(actionBbCode, SIGNAL(triggered()), SLOT(exportText()));
    _itemsView->addAction(actionBbCode);
    _itemActions[ExportBbCode] = actionBbCode;

    QAction *actionHtml = new QAction("HTML", _itemsView);
    actionHtml->setShortcut(QKeySequence("Alt+E"));
    actionHtml->setObjectName("html");
    connect(actionHtml, SIGNAL(triggered()), SLOT(exportText()));
    _itemsView->addAction(actionHtml);
    _itemActions[ExportHtml] = actionHtml;

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

    QAction *actionRemoveMO = new QAction(tr("Remove Mystic Orbs"), _itemsView);
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
