#include "itemstoragetableview.h"
#include "itemstoragetablemodel.h"
#include "structs.h"
#include "itemdatabase.h"

#include <QHeaderView>
#include <QKeyEvent>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


ItemStorageTableView::ItemStorageTableView(QWidget *parent /*= 0*/) : QTableView(parent)
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

//    connect(this, SIGNAL(clicked(const QModelIndex &)), SLOT(itemClicked(const QModelIndex &)));
}

void ItemStorageTableView::keyPressEvent(QKeyEvent *event)
{
    QTableView::keyPressEvent(event);

    int key = event->key();
    if (key == Qt::Key_Up || key == Qt::Key_Down || key == Qt::Key_Left || key == Qt::Key_Right || key == Qt::Key_PageUp || key == Qt::Key_PageDown || key == Qt::Key_Home || key == Qt::Key_End)
        selectionModel()->setCurrentIndex(originIndexInRectOfIndex(currentIndex()), QItemSelectionModel::ClearAndSelect);
}

void ItemStorageTableView::dragMoveEvent(QDragMoveEvent *event)
{
    if (static_cast<ItemStorageTableModel *>(model())->canStoreItemWithMimeDataAtIndex(event->mimeData(), actualIndexAt(event->pos())))
        event->acceptProposedAction();
    else
        event->ignore();
}

void ItemStorageTableView::dropEvent(QDropEvent *event)
{
    QModelIndex index = actualIndexAt(event->pos());
    if (index.isValid() && model()->dropMimeData(event->mimeData(), event->dropAction(), index.row(), index.column(), index.parent()))
        event->acceptProposedAction();
    else
        event->ignore();
}

//void ItemStorageTableView::itemClicked(const QModelIndex &index)
//{
//    startDrag(Qt::MoveAction);
//}
