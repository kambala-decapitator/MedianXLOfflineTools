#include "itemspropertiessplitter.h"
#include "propertiesviewerwidget.h"
#include "itemdatabase.h"
#include "itemstoragetableview.h"
#include "itemstoragetablemodel.h"
#include "itemparser.h"

#include <QPushButton>
#include <QDoubleSpinBox>
#include <QKeyEvent>
#include <QMenu>

#include <qmath.h>

#include <limits>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


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
        keyReleaseEvent(&QKeyEvent(QEvent::KeyRelease, Qt::Key_Shift, 0)); // hacky way to set text on buttons

        _pageSpinBox = new QDoubleSpinBox(this);
        _pageSpinBox->setDecimals(0);
        _pageSpinBox->setPrefix(tr("Page #"));
        _pageSpinBox->setRange(1, (std::numeric_limits<quint32>::max)());
        _pageSpinBox->setValue(1);

        QHBoxLayout *hlayout = new QHBoxLayout;
        hlayout->addWidget(_left10Button);
        hlayout->addWidget(_leftButton);
        hlayout->addWidget(_pageSpinBox);
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
    _propertiesWidget->displayItemProperties(dynamic_cast<ItemManager *>(_itemsModel)->itemAt(index));
}

void ItemsPropertiesSplitter::keyPressEvent(QKeyEvent *keyEvent)
{
    if (_left10Button && keyEvent->key() == Qt::Key_Shift)
    {
        _left10Button->setText("left 100");
        _leftButton->setText("first");
        _rightButton->setText("last not empty");
        _rightButton->setToolTip(QString::number(_lastNotEmptyPage));
        _right10Button->setText("right 100");

        _isShiftPressed = true;
    }
    else
        QSplitter::keyPressEvent(keyEvent);
}

void ItemsPropertiesSplitter::keyReleaseEvent(QKeyEvent *keyEvent)
{
    if (_left10Button && keyEvent->key() == Qt::Key_Shift)
    {
        _left10Button->setText("left 10");
        _leftButton->setText("left");
        _rightButton->setText("right");
        _rightButton->setToolTip(QString());
        _right10Button->setText("right 10");

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
    dynamic_cast<ItemManager *>(_itemsModel)->setItems(newItems);

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
		qDebug() << "pos" << pos << "item type" << item->itemType;
		// TODO: add slots
		QList<QAction *> actions;
		if (item->quality == Enums::ItemQuality::Set || item->quality == Enums::ItemQuality::Unique && ItemDataBase::Items()->value(item->itemType).typeString != "grtz") // not a charm
		{
			QAction *actionShards = new QAction(tr("Arcane Shards"), _itemsView);
			actionShards->setObjectName("shards");
			connect(actionShards, SIGNAL(triggered()), SLOT(disenchantItem()));

			QAction *actionSol = new QAction(tr("Signet of Learning"), _itemsView);
			actionSol->setObjectName("signet");
			connect(actionSol, SIGNAL(triggered()), SLOT(disenchantItem()));

			QMenu *menuDisenchant = new QMenu(tr("Disenchant into"), _itemsView);
			menuDisenchant->addActions(QList<QAction *>() << actionShards << actionSol);
			actions << menuDisenchant->menuAction();
		}
		if (item->isSocketed && item->socketablesNumber)
		{
			QAction *actionUnsocket = new QAction(tr("Unsocket"), _itemsView);
			connect(actionUnsocket, SIGNAL(triggered()), SLOT(unsocketItem()));
			actions << actionUnsocket;
		}
		// TODO: add "Remove Mystic Orbs"
		QAction *actionDelete = new QAction(tr("Delete"), _itemsView);
		connect(actionDelete, SIGNAL(triggered()), SLOT(deleteItem()));
		actions << actionDelete;

		foreach (QAction *action, actions)
		{
			action->setData(pos);

			QMenu *actionMenu = action->menu();
			if (actionMenu)
			{
				foreach (QAction *childAction, actionMenu->actions())
					childAction->setData(pos);
			}
		}

		QMenu::exec(actions, _itemsView->mapToGlobal(pos));
	}
}

void ItemsPropertiesSplitter::disenchantItem()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
	{
		ERROR_BOX("TROLOLOL I can't disenchant the item");
		return;
	}
	
	ItemInfo *item = itemFromCoordinate(action->data().toPoint());
	qDebug() << "pos" << action->data().toPoint() << "item type" << item->itemType;
	if (action->objectName() == "signet")
	{
		QString signetPath = DATA_PATH("items/signet_of_learning.d2i");
		ItemInfo *signet = ItemParser::loadItemFromFile(signetPath);
		if (!signet)
		{
			ERROR_BOX(tr("Error loading '%1'").arg(signetPath));
			return;
		}

		//if (!ItemParser::storeItemIn(Enums::ItemStorage::Inventory, 6, 10, signet) && !ItemParser::storeItemIn(Enums::ItemStorage::Stash, 10, 10, signet))
		//{
		//	ERROR_BOX(tr("You have no free space in inventory and stash to store the Signet of Learning"));
		//	delete signet;
		//	return;
		//}

		// TODO: fix if item is on the character
		signet->row = item->row;
		signet->column = item->column;
		signet->storage = item->storage;
		signet->whereEquipped = item->whereEquipped;
		signet->plugyPage = item->plugyPage;
		signet->location = Enums::ItemLocation::Stored;
		
		ItemDataBase::currentCharacterItems->append(signet);
		_allItems.append(signet);
		performDeleteItem(item);
	}
	else // shards
	{

	}
}

void ItemsPropertiesSplitter::unsocketItem()
{

}

void ItemsPropertiesSplitter::deleteItem()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
	{
		ERROR_BOX("TROLOLOL I can't delete the item");
		return;
	}

	performDeleteItem(itemFromCoordinate(action->data().toPoint()));
}

ItemInfo *ItemsPropertiesSplitter::itemFromCoordinate(const QPoint &pos)
{
	return _itemsModel->itemAt(_itemsView->indexAt(pos));
}

void ItemsPropertiesSplitter::performDeleteItem(ItemInfo *item)
{
	// TODO: add option to unsocket at first
	ItemDataBase::currentCharacterItems->removeOne(item);
	_allItems.removeOne(item);
	qDeleteAll(item->socketablesInfo);
	delete item;

	setItems(_allItems); // maybe not needed?
}
