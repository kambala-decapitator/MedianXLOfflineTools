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

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


static const QString iconPathFormat(":/PlugyArrows/Resources/icons/plugy/%1.png");

ItemsPropertiesSplitter::ItemsPropertiesSplitter(ItemStorageTableView *itemsView, ItemStorageTableModel *itemsModel, bool shouldCreateNavigation, QWidget *parent)
    : QSplitter(Qt::Horizontal, parent), _itemsView(itemsView), _itemsModel(itemsModel)
{
	_itemsView->setContextMenuPolicy(Qt::CustomContextMenu);
    _itemsView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _itemsView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    _itemsView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    _itemsView->setSelectionMode(QAbstractItemView::SingleSelection);
//    _itemsView->setStyleSheet("QAbstractItemView::item:selected:active { background:black } QAbstractItemView::item:hover { color:green; background-color: yellow }");
    _itemsView->setStyleSheet("QAbstractItemView::item:selected:active { background:green }"); // TODO: change to black
    _itemsView->setGridStyle(Qt::DashLine);
    _itemsView->setCornerButtonEnabled(false);
    _itemsView->horizontalHeader()->hide();
    _itemsView->verticalHeader()->hide();
    _itemsView->setModel(_itemsModel);

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
        keyReleaseEvent(&QKeyEvent(QEvent::KeyRelease, Qt::Key_Shift, 0)); // hacky way to set button icons

        _pageSpinBox = new QDoubleSpinBox(this);
        _pageSpinBox->setDecimals(0);
        _pageSpinBox->setPrefix(tr("Page #"));
        _pageSpinBox->setRange(1, (std::numeric_limits<quint32>::max)());
        _pageSpinBox->setValue(1);
		_pageSpinBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        QHBoxLayout *hlayout = new QHBoxLayout;
        hlayout->addWidget(_left10Button);
        hlayout->addWidget(_leftButton);
		//hlayout->addStretch();
		hlayout->addWidget(_pageSpinBox);
		//hlayout->addStretch();
        hlayout->addWidget(_rightButton);
        hlayout->addWidget(_right10Button);

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
    setStretchFactor(0, 4);

    //connect(_itemsView, SIGNAL(clicked(const QModelIndex &)), SLOT(itemSelected(const QModelIndex &)));
    //connect(_itemsView, SIGNAL(currentItemChanged(const QModelIndex &)), SLOT(itemSelected(const QModelIndex &)));
	connect(_itemsView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), SLOT(itemSelected(const QModelIndex &)));
	connect(_itemsView, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(showContextMenu(const QPoint &)));

    if (shouldCreateNavigation)
    {
        connect(_pageSpinBox, SIGNAL(valueChanged(double)), SLOT(updateItemsForCurrentPage()));

        connect(_left10Button, SIGNAL(clicked()), SLOT(left10Clicked()));
        connect(_leftButton, SIGNAL(clicked()), SLOT(leftClicked()));
        connect(_rightButton, SIGNAL(clicked()), SLOT(rightClicked()));
        connect(_right10Button, SIGNAL(clicked()), SLOT(right10Clicked()));
    }
}

void ItemsPropertiesSplitter::itemSelected(const QModelIndex &index)
{
    _propertiesWidget->displayItemProperties(_itemsModel->itemAt(index));
}

void ItemsPropertiesSplitter::keyPressEvent(QKeyEvent *keyEvent)
{
    if (_left10Button && keyEvent->key() == Qt::Key_Shift)
    {
        _left10Button->setIcon(QIcon(iconPathFormat.arg("left100")));
        _leftButton->setIcon(QIcon(iconPathFormat.arg("first")));
        _rightButton->setIcon(QIcon(iconPathFormat.arg("last")));
        _rightButton->setToolTip(QString::number(_lastNotEmptyPage));
        _right10Button->setIcon(QIcon(iconPathFormat.arg("right100")));

        _isShiftPressed = true;
    }
    else
        QSplitter::keyPressEvent(keyEvent);
}

void ItemsPropertiesSplitter::keyReleaseEvent(QKeyEvent *keyEvent)
{
    if (_left10Button && keyEvent->key() == Qt::Key_Shift)
    {
        _left10Button->setIcon(QIcon(iconPathFormat.arg("left10")));
        _leftButton->setIcon(QIcon(iconPathFormat.arg("left")));
        _rightButton->setIcon(QIcon(iconPathFormat.arg("right")));
        _rightButton->setToolTip(QString());
        _right10Button->setIcon(QIcon(iconPathFormat.arg("right10")));

        _isShiftPressed = false;
    }
    else
        QSplitter::keyPressEvent(keyEvent);
}

void ItemsPropertiesSplitter::setItems(const ItemsList &newItems)
{
    _allItems = newItems;
    if (_left10Button)
    {
        _lastNotEmptyPage = _allItems.size() ? _allItems.last()->plugyPage : 0;
		_pageSpinBox->setSuffix(QString(" / %1").arg(_lastNotEmptyPage));
		_pageSpinBox->setMaximum(_lastNotEmptyPage);
        updateItemsForCurrentPage();
    }
    else
        updateItems(_allItems);
}

void ItemsPropertiesSplitter::showItem(ItemInfo *item)
{
	if (item->plugyPage)
		_pageSpinBox->setValue(item->plugyPage);
	_itemsView->setCurrentIndex(_itemsModel->index(item->row, item->column));
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
}

void ItemsPropertiesSplitter::updateItemsForCurrentPage()
{
    ItemsList pagedItems;
    foreach (ItemInfo *item, _allItems)
    {
        if (item->plugyPage == static_cast<quint32>(_pageSpinBox->value()))
            pagedItems += item;
    }
    updateItems(pagedItems);
}

void ItemsPropertiesSplitter::left10Clicked()
{
    quint32 step = _isShiftPressed ? 100 : 10;
    _pageSpinBox->setValue(qFloor((_pageSpinBox->value() - 1) / step) * step);
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

void ItemsPropertiesSplitter::right10Clicked()
{
    quint32 step = _isShiftPressed ? 100 : 10;
    _pageSpinBox->setValue(qCeil((_pageSpinBox->value() + 1) / step) * step);
}

void ItemsPropertiesSplitter::showContextMenu(const QPoint &pos)
{
	ItemInfo *item = itemFromCoordinate(pos);
	if (item)
	{
		QList<QAction *> actions;

		QAction *actionHtml = new QAction(tr("HTML..."), _itemsView), *actionBbCode = new QAction(tr("BBCode..."), _itemsView);
		QMenu *menuExport = new QMenu(tr("Export as"), _itemsView);
		menuExport->addActions(QList<QAction *>() << actionHtml << actionBbCode);
		actions << menuExport->menuAction();

		QAction *separator = new QAction(_itemsView);
		separator->setSeparator(true);
		actions << separator;

		QByteArray typeString = ItemDataBase::Items()->value(item->itemType).typeString;
        bool isCharm = typeString == "grtz" || typeString.startsWith("ara"), isSummonBook = typeString == "summ";
		if (item->quality == Enums::ItemQuality::Set || item->quality == Enums::ItemQuality::Unique && !isCharm && !isSummonBook)
		{
			QAction *actionShards = new QAction(QIcon(ResourcePathManager::pathForImageName("invfary4")), tr("Arcane Shards"), _itemsView);
			actionShards->setObjectName("shards");
			connect(actionShards, SIGNAL(triggered()), SLOT(disenchantItem()));

			QAction *actionSol = new QAction(QIcon(ResourcePathManager::pathForImageName("sigil1b")), tr("Signet of Learning"), _itemsView);
			actionSol->setObjectName("signet");
			connect(actionSol, SIGNAL(triggered()), SLOT(disenchantItem()));

			QMenu *menuDisenchant = new QMenu(tr("Disenchant into"), _itemsView);
			menuDisenchant->addActions(QList<QAction *>() << actionShards << actionSol);
			actions << menuDisenchant->menuAction();
		}
		// TODO 0.3
		//if (item->isSocketed && item->socketablesNumber)
		//{
		//	QAction *actionUnsocket = new QAction(tr("Unsocket"), _itemsView);
		//	connect(actionUnsocket, SIGNAL(triggered()), SLOT(unsocketItem()));
		//	actions << actionUnsocket;
		//}
        // no need
		//if (item->isEthereal)
		//{
		//	QAction *actionMakeNonEthereal = new QAction(tr("Make Non-Ethereal"), _itemsView);
		//	connect(actionMakeNonEthereal, SIGNAL(triggered()), SLOT(makeNonEthereal()));
		//	actions << actionMakeNonEthereal;
		//}

		// TODO: add "Remove Mystic Orbs"
		QAction *actionDelete = new QAction(tr("Delete"), _itemsView);
		connect(actionDelete, SIGNAL(triggered()), SLOT(deleteItem()));
		actions << actionDelete;

		foreach (QAction *action, actions)
		{
			action->setData(pos);

			QMenu *actionMenu = action->menu();
			if (actionMenu)
				foreach (QAction *childAction, actionMenu->actions())
					childAction->setData(pos);
		}

		QMenu::exec(actions, _itemsView->mapToGlobal(pos));
	}
}

void ItemsPropertiesSplitter::disenchantItem()
{
	QAction *action = actionFromSender(sender(), QLatin1String("disenchant"));
	if (!action)
		return;
	
	ItemInfo *item = itemFromAction(action);
    int insertIndex = _allItems.indexOf(item);

	QString path = ResourcePathManager::dataPathForFileName(QString("items/%1.d2i").arg(action->objectName() == "signet" ? "signet_of_learning" : "arcane_shard"));
	ItemInfo *newItem = ItemParser::loadItemFromFile(path);

    // TODO: fix if item is on the character
    ItemsList items = ItemParser::itemsLocatedAt(item->storage);
    items.removeOne(item);
	if (!ItemParser::canStoreItemAt(item->row, item->column, newItem->itemType, items, ItemsViewerDialog::rows.at(ItemsViewerDialog::indexFromItemStorage(item->storage)), 10))
	{
		ERROR_BOX("If you see this text (which you shouldn't), please tell me which item you've just tried to disenchant");
		delete newItem;
		return;
	}

	newItem->row = item->row;
	newItem->column = item->column;
	newItem->storage = item->storage;
    newItem->whereEquipped = item->whereEquipped;
    newItem->location = Enums::ItemLocation::Stored;
	newItem->plugyPage = item->plugyPage;

    // update bits
    bool isPlugyStorage = newItem->storage == Enums::ItemStorage::PersonalStash || newItem->storage == Enums::ItemStorage::SharedStash || newItem->storage == Enums::ItemStorage::HCStash;
    ReverseBitWriter::replaceValueInBitString(newItem->bitString, Enums::ItemOffsets::Storage, 3, isPlugyStorage ? Enums::ItemStorage::Stash : newItem->storage);
    ReverseBitWriter::replaceValueInBitString(newItem->bitString, Enums::ItemOffsets::Columns, 4, newItem->column);
    ReverseBitWriter::replaceValueInBitString(newItem->bitString, Enums::ItemOffsets::Rows, 3, newItem->row);
    ReverseBitWriter::replaceValueInBitString(newItem->bitString, Enums::ItemOffsets::EquipIndex, 4, newItem->whereEquipped);
    ReverseBitWriter::replaceValueInBitString(newItem->bitString, Enums::ItemOffsets::Location, 3, newItem->location);

    ItemsList personalStashItems = ItemParser::itemsLocatedAt(Enums::ItemStorage::PersonalStash);
    if (newItem->storage == Enums::ItemStorage::Stash && personalStashItems.size() > 0 || newItem->storage == Enums::ItemStorage::PersonalStash)
    {
        ItemInfo *copy = new ItemInfo(*newItem);
        copy->storage = newItem->storage == Enums::ItemStorage::Stash ? Enums::ItemStorage::PersonalStash : Enums::ItemStorage::Stash;
        copy->plugyPage = newItem->storage == Enums::ItemStorage::Stash;
        // TODO: finish
    }

    performDeleteItem(item);
	addItemToList(newItem, insertIndex);
}

//void ItemsPropertiesSplitter::unsocketItem()
//{
//
//}
//
//void ItemsPropertiesSplitter::makeNonEthereal()
//{
//	ItemInfo *item = itemFromAction(actionFromSender(sender(), QLatin1String("non-etherealize")));
//	if (item)
//    {
//        item->hasChanged = true;
//
//        item->isEthereal = false;
//        ReverseBitWriter::replaceValueInBitString(item->bitString, Enums::ItemOffsets::Ethereal, 1, 0);
//
//        if (ItemDataBase::Items()->value(item->itemType).genericType == Enums::ItemTypeGeneric::Armor)
//        {
//            // TODO if will bring back: find the correct position for replacement (the defense offset isn't static)
//            item->defense /= 1.5;
//            const ItemPropertyTxt &defenceProp = ItemDataBase::Properties()->value(Enums::ItemProperties::Defence);
//            ReverseBitWriter::replaceValueInBitString(item->bitString, Enums::ItemOffsets::Ethereal, defenceProp.bits, item->defense + defenceProp.add);
//        }
//
//		_propertiesWidget->displayItemProperties(item);
//	}
//}

void ItemsPropertiesSplitter::deleteItem()
{
	ItemInfo *item = itemFromAction(actionFromSender(sender(), QLatin1String("delete")));
	if (item)
		performDeleteItem(item);
}

ItemInfo *ItemsPropertiesSplitter::itemFromCoordinate(const QPoint &pos)
{
	return _itemsModel->itemAt(_itemsView->indexAt(pos));
}

void ItemsPropertiesSplitter::performDeleteItem(ItemInfo *item)
{
	// TODO 0.3: add option to unsocket at first
	removeItemFromList(item);
	qDeleteAll(item->socketablesInfo);
	delete item;

	setItems(_allItems); // maybe not needed?
}

QAction *ItemsPropertiesSplitter::actionFromSender(QObject *sender, const QLatin1String &errorActionText)
{
	QAction *action = qobject_cast<QAction *>(sender);
	if (!action)
		ERROR_BOX("TROLOLOL I can't " + errorActionText + " the item");
	return action;
}

ItemInfo *ItemsPropertiesSplitter::itemFromAction(QAction *action)
{
	ItemInfo *item = 0;
	if (action)
		item = itemFromCoordinate(action->data().toPoint());
	return item;
}

void ItemsPropertiesSplitter::addItemToList(ItemInfo *item, int pos /*= -1*/)
{
	ItemDataBase::currentCharacterItems->append(item);
	if (pos > -1)
		_allItems.insert(pos, item);
	else
		_allItems.append(item);
}

void ItemsPropertiesSplitter::removeItemFromList(ItemInfo *item)
{
	ItemDataBase::currentCharacterItems->removeOne(item);
	_allItems.removeOne(item);
}
