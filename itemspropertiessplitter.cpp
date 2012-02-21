#include "itemspropertiessplitter.h"
#include "propertiesviewerwidget.h"
#include "itemdatabase.h"
#include "itemstoragetableview.h"
#include "itemstoragetablemodel.h"
#include "itemparser.h"
#include "resourcepathmanager.hpp"
#include "itemsviewerdialog.h"
#include "reversebitwriter.h"

#include <QPushButton>
#include <QDoubleSpinBox>
#include <QKeyEvent>
#include <QMenu>

#include <qmath.h>

#include <limits>


static const QString iconPathFormat(":/PlugyArrows/icons/plugy/%1.png");

ItemsPropertiesSplitter::ItemsPropertiesSplitter(ItemStorageTableView *itemsView, ItemStorageTableModel *itemsModel, bool shouldCreateNavigation, QWidget *parent)
    : QSplitter(Qt::Horizontal, parent), _itemsView(itemsView), _itemsModel(itemsModel)
{
    _itemsView->setContextMenuPolicy(Qt::CustomContextMenu);
    _itemsView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _itemsView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    _itemsView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    _itemsView->setSelectionMode(QAbstractItemView::SingleSelection);
    _itemsView->setStyleSheet("QTableView { background-color: black; gridline-color: green; }"
                              "QTableView::item:selected { background-color: black; border: 1px solid #d9d9d9; }"
                              "QTableView::icon:selected { right: 1px; }"
                             );
    _itemsView->setGridStyle(Qt::SolidLine);
    _itemsView->setCornerButtonEnabled(false);
    _itemsView->horizontalHeader()->hide();
    _itemsView->verticalHeader()->hide();
    _itemsView->setModel(_itemsModel);

    _itemsView->installEventFilter(this);
    installEventFilter(this);

    if (shouldCreateNavigation)
    {
        _left10Button = new QPushButton(this);
        _leftButton = new QPushButton(this);
        _rightButton = new QPushButton(this);
        _right10Button = new QPushButton(this);

        QList<QPushButton *> buttons = QList<QPushButton *>() << _left10Button << _leftButton << _rightButton << _right10Button;
        foreach (QPushButton *button, buttons)
        {
            button->setIconSize(QSize(32, 20));
            button->resize(button->minimumSizeHint());
        }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Waddress-of-temporary"
        keyReleaseEvent(&QKeyEvent(QEvent::KeyRelease, Qt::Key_Shift, 0)); // hacky way to set button icons
#pragma clang diagnostic pop

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

    setChildrenCollapsible(false);
    setStretchFactor(1, 5);

    connect(_itemsView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), SLOT(itemSelected(const QModelIndex &)));
    connect(_itemsView, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(showContextMenu(const QPoint &)));
    connect(_itemsView, SIGNAL(deleteSelectedItem()), SLOT(deleteItem()));

    if (shouldCreateNavigation)
    {
        connect(_pageSpinBox, SIGNAL(valueChanged(double)), SLOT(updateItemsForCurrentPage()));

        connect(_left10Button, SIGNAL(clicked()), SLOT(left10Clicked()));
        connect(_leftButton, SIGNAL(clicked()), SLOT(leftClicked()));
        connect(_rightButton, SIGNAL(clicked()), SLOT(rightClicked()));
        connect(_right10Button, SIGNAL(clicked()), SLOT(right10Clicked()));
    }

    _itemsView->setFocus();
}

void ItemsPropertiesSplitter::itemSelected(const QModelIndex &index)
{
    _propertiesWidget->showItem(_itemsModel->itemAt(index));
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

        // TODO: uncomment when implementing
        //QAction *actionHtml = new QAction("HTML", _itemsView), *actionBbCode = new QAction("BBCode", _itemsView);
        //connect(actionHtml, SIGNAL(triggered()), SLOT(exportHtml()));
        //connect(actionBbCode, SIGNAL(triggered()), SLOT(exportBbCode()));
        //QMenu *menuExport = new QMenu(tr("Export as"), _itemsView);
        //menuExport->addActions(QList<QAction *>() << actionHtml << actionBbCode);
        //actions << menuExport->menuAction();

        //QAction *separator = new QAction(_itemsView);
        //separator->setSeparator(true);
        //actions << separator;

        if (item->quality == Enums::ItemQuality::Set || (item->quality == Enums::ItemQuality::Unique && !ItemDataBase::isUberCharm(item)))
        {
            QAction *actionShards = new QAction(QIcon(ResourcePathManager::pathForImageName("invfary4")), tr("Arcane Shards"), _itemsView);
            //actionShards->setShortcut(QKeySequence("Alt+D"));
            actionShards->setObjectName("shards");
            connect(actionShards, SIGNAL(triggered()), SLOT(disenchantItem()));

            QAction *actionSol = new QAction(QIcon(ResourcePathManager::pathForImageName("sigil1b")), tr("Signet of Learning"), _itemsView);
            //actionSol->setShortcut(QKeySequence("Ctrl+D"));
            actionSol->setObjectName("signet");
            connect(actionSol, SIGNAL(triggered()), SLOT(disenchantItem()));

            QMenu *menuDisenchant = new QMenu(tr("Disenchant into"), _itemsView);
            menuDisenchant->addActions(QList<QAction *>() << actionShards << actionSol);
            menuDisenchant->menuAction()->setDisabled(item->location == Enums::ItemLocation::Equipped); // you can't disenchant equipped items
            actions << menuDisenchant->menuAction();
        }
        // TODO 0.3
        //if (item->isSocketed && item->socketablesNumber)
        //{
        //    QAction *actionUnsocket = new QAction(tr("Unsocket"), _itemsView);
        //    connect(actionUnsocket, SIGNAL(triggered()), SLOT(unsocketItem()));
        //    actions << actionUnsocket;
        //}

        // no need
        //if (item->isEthereal)
        //{
        //    QAction *actionMakeNonEthereal = new QAction(tr("Make Non-Ethereal"), _itemsView);
        //    connect(actionMakeNonEthereal, SIGNAL(triggered()), SLOT(makeNonEthereal()));
        //    actions << actionMakeNonEthereal;
        //}

        if (_propertiesWidget->hasMysticOrbs())
        {
            QAction *actionRemoveMO = new QAction(tr("Remove Mystic Orbs"), _itemsView);
            connect(actionRemoveMO, SIGNAL(triggered()), _propertiesWidget, SLOT(removeAllMysticOrbs()));
            actions << actionRemoveMO;
        }

        QAction *actionDelete = new QAction(tr("Delete"), _itemsView);
        actionDelete->setShortcut(
#ifdef Q_WS_MACX
                    Qt::Key_Backspace
#else
                    QKeySequence::Delete
#endif
                    );
        connect(actionDelete, SIGNAL(triggered()), SLOT(deleteItem()));
        actions << actionDelete;

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

void ItemsPropertiesSplitter::exportHtml()
{

}

void ItemsPropertiesSplitter::exportBbCode()
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
    if (!ItemDataBase::canStoreItemAt(item->row, item->column, newItem->itemType, items, ItemsViewerDialog::rows.at(ItemsViewerDialog::tabIndexFromItemStorage(item->storage)), item->plugyPage))
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
        {
            //ERROR_BOX(tr("You can't delete Cube that has items inside"));
            if (QUESTION_BOX_YESNO(tr("Cube is not empty. Do you really want to delete it?\nNote: its items will be preserved. You can recover them by getting new Cube."), QMessageBox::No) == QMessageBox::No)
                return;
        }

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
    ItemDataBase::currentCharacterItems->append(item);
    
    if (currentStorage)
    {
        _allItems.append(item);
        _itemsModel->addItem(item);
        _propertiesWidget->showItem(item);
    }
}

void ItemsPropertiesSplitter::removeItemFromList(ItemInfo *item, bool currentStorage /*= true*/)
{
    ItemDataBase::currentCharacterItems->removeOne(item);
    
    if (currentStorage)
    {
        _propertiesWidget->clear();

        _allItems.removeOne(item);
        _itemsModel->removeItem(item);
        _itemsView->setSpan(item->row, item->column, 1, 1);
    }
}
