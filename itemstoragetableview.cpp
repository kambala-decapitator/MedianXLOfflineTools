#include "itemstoragetableview.h"
#include "itemstoragetablemodel.h"
#include "structs.h"
#include "itemdatabase.h"

#include <QHeaderView>
#include <QKeyEvent>

#include <QTimer>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


ItemStorageTableView::ItemStorageTableView(QWidget *parent /*= 0*/) : QTableView(parent), _dragLeaveTimer(new QTimer(this)), _draggedItem(0)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionMode(QAbstractItemView::SingleSelection); // TODO 0.3: change to ExtendedSelection

    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    setGridStyle(Qt::SolidLine);
    setStyleSheet("QTableView { background-color: black; gridline-color: #808080; }"
                  "QTableView::item:selected { background-color: black; border: 1px solid #d9d9d9; }"
                  "QTableView::icon:selected { right: 1px; }"
                 );

    setDragDropMode(QAbstractItemView::DragDrop);
    setDragDropOverwriteMode(false);
    setDropIndicatorShown(true);

    setCornerButtonEnabled(false);
    horizontalHeader()->hide();
    verticalHeader()->hide();

    _dragLeaveTimer->setInterval(100);
    connect(_dragLeaveTimer, SIGNAL(timeout()), SLOT(checkIfStillDragging()));

//    connect(this, SIGNAL(clicked(const QModelIndex &)), SLOT(itemClicked(const QModelIndex &)));
}

ItemStorageTableModel *ItemStorageTableView::model() const
{
    return static_cast<ItemStorageTableModel *>(QTableView::model());
}

void ItemStorageTableView::setCellSpanForItem(ItemInfo *item)
{
    const ItemBase &itemBase = ItemDataBase::Items()->value(item->itemType);
    setSpan(item->row, item->column, itemBase.height, itemBase.width);
}

void ItemStorageTableView::keyPressEvent(QKeyEvent *event)
{
    QTableView::keyPressEvent(event);

    int key = event->key();
    if (key == Qt::Key_Up || key == Qt::Key_Down || key == Qt::Key_Left || key == Qt::Key_Right || key == Qt::Key_PageUp || key == Qt::Key_PageDown || key == Qt::Key_Home || key == Qt::Key_End)
        selectionModel()->setCurrentIndex(originIndexInRectOfIndex(currentIndex()), QItemSelectionModel::ClearAndSelect);
}

void ItemStorageTableView::dragEnterEvent(QDragEnterEvent *event)
{
    _dragLeaveTimer->stop();

    ItemStorageTableModel *model_ = model();
    QModelIndex index = indexAt(event->pos());
    if (!model_->dragOriginIndex().isValid())
    {
        model_->setDragOriginIndex(index);
        setSpan(index.row(), index.column(), 1, 1);
    }
    selectionModel()->clearSelection();

    if (!_draggedItem)
        _draggedItem = model_->itemFromMimeData(event->mimeData());
    const ItemBase &dragItemBase = ItemDataBase::Items()->operator[](_draggedItem->itemType);
    QModelIndexList highlightIndexes;
    highlightIndexes += index;
    for (int i = 0; i < dragItemBase.width; ++i)
        for (int j = 0; j < dragItemBase.height; ++j)
            if (i || j) // first index is already in the list
            {
                QModelIndex anIndex = model_->index(index.row() + i, index.column() + j);
                if (anIndex.isValid())
                    highlightIndexes += anIndex;
            }
    model_->setHighlightIndexes(highlightIndexes);

    QTableView::dragEnterEvent(event);
}

void ItemStorageTableView::dragMoveEvent(QDragMoveEvent *event)
{
    if (model()->canStoreItemWithMimeDataAtIndex(event->mimeData(), indexForDragDropEvent(event)))
        event->acceptProposedAction();
    else
        event->ignore();
}

void ItemStorageTableView::dragLeaveEvent(QDragLeaveEvent *event)
{
    QTableView::dragLeaveEvent(event);

    _dragLeaveTimer->setSingleShot(true);
    _dragLeaveTimer->start();
}

void ItemStorageTableView::dropEvent(QDropEvent *event)
{
    dragStopped();

    QModelIndex index = indexForDragDropEvent(event);
    if (index.isValid() && model()->dropMimeData(event->mimeData(), event->dropAction(), index.row(), index.column(), index.parent()))
        event->acceptProposedAction();
    else
        event->ignore();
}

void ItemStorageTableView::checkIfStillDragging()
{
    if (findChild<QDrag *>()) // item has left current view
    {
        _dragLeaveTimer->setSingleShot(false);
        if (!_dragLeaveTimer->isActive())
            _dragLeaveTimer->start();
    }
    else // dragging stopped
    {
        _dragLeaveTimer->stop();

        ItemStorageTableModel *model_ = model();
        QModelIndex originalIndex = model_->dragOriginIndex();
        dragStopped();
        setCellSpanForItem(model_->itemAtIndex(originalIndex));
        setCurrentIndex(originalIndex);
    }
}

QModelIndex ItemStorageTableView::actualIndexAt(const QPoint &p)
{
    return model()->index(rowAt(p.y()), columnAt(p.x()));
}

QModelIndex ItemStorageTableView::indexForDragDropEvent(QDropEvent *event)
{
    return actualIndexAt(event->pos() - findChild<QDrag *>()->hotSpot() + QPoint(rowHeight(0), columnWidth(0)) / 3);
}

void ItemStorageTableView::dragStopped()
{
    _draggedItem = 0;
    model()->setDragOriginIndex(QModelIndex());
}

//void ItemStorageTableView::itemClicked(const QModelIndex &index)
//{
//    startDrag(Qt::MoveAction);
//}
