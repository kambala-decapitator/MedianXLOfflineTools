#include "itemstoragetableview.h"

#include <QKeyEvent>


void ItemStorageTableView::keyPressEvent(QKeyEvent *event)
{
    QTableView::keyPressEvent(event);

    int key = event->key();
    if (key == Qt::Key_Up || key == Qt::Key_Down || key == Qt::Key_Left || key == Qt::Key_Right || key == Qt::Key_PageUp || key == Qt::Key_PageDown || key == Qt::Key_Home || key == Qt::Key_End)
    {
        QModelIndex newIndex = indexAt(visualRect(currentIndex()).topLeft());
        selectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::ClearAndSelect);
    }
#ifdef Q_WS_MACX
    else if (key == Qt::Key_Backspace)
#else
    else if (key == Qt::Key_Delete)
#endif
        emit deleteSelectedItem();
}
