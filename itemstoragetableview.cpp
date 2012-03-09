#include "itemstoragetableview.h"

#include <QHeaderView>
#include <QKeyEvent>


ItemStorageTableView::ItemStorageTableView(QWidget *parent /*= 0*/) : QTableView(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setStyleSheet("QTableView { background-color: black; gridline-color: #808080; }"
                  "QTableView::item:selected { background-color: black; border: 1px solid #d9d9d9; }"
                  "QTableView::icon:selected { right: 1px; }"
                 );
    setGridStyle(Qt::SolidLine);
    setCornerButtonEnabled(false);
    horizontalHeader()->hide();
    verticalHeader()->hide();
}

void ItemStorageTableView::keyPressEvent(QKeyEvent *event)
{
    QTableView::keyPressEvent(event);

    int key = event->key();
    if (key == Qt::Key_Up || key == Qt::Key_Down || key == Qt::Key_Left || key == Qt::Key_Right || key == Qt::Key_PageUp || key == Qt::Key_PageDown || key == Qt::Key_Home || key == Qt::Key_End)
    {
        QModelIndex newIndex = indexAt(visualRect(currentIndex()).topLeft());
        selectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::ClearAndSelect);
    }
}
