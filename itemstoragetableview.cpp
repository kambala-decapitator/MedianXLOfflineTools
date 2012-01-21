#include "itemstoragetableview.h"
#include "itemmanager.h"

#include <QKeyEvent>


//struct ItemInfo;

void ItemStorageTableView::keyPressEvent(QKeyEvent *event)
{
//    QModelIndex oldIndex = currentIndex();
//    QTableView::keyPressEvent(event);
//    int key = event->key(), oldRow = oldIndex.row(), oldCol = oldIndex.column(), newRow = -1, newCol = -1, rows = model()->rowCount(), cols = model()->columnCount();
//    ItemInfo *item = 0;
//    switch (key)
//    {
//    case Qt::Key_Up:
////        newRow = oldRow ? oldRow - 1 : 0;
//    {
//        if ((newRow = oldRow) == 0)
//            return;

//        newCol = oldCol;
//        QModelIndex newIndex;
//        ItemManager *itemManager = dynamic_cast<ItemManager *>(model());
//        ItemInfo *oldItem = itemManager->itemAt(indexAt(visualRect(oldIndex).center()));
//        do
//        {
//            newIndex = indexAt(visualRect(model()->index(--newRow, newCol)).center());
//            item = itemManager->itemAt(newIndex);
//        } while ((!item || item == oldItem) && newRow);

//        if (item)
//        {
//            currentChanged(newIndex, oldIndex);
//            emit currentItemChanged(newIndex);
//        }
//        break;
//    }
//    case Qt::Key_Down:
////        newRow = oldRow != rows - 1 ? oldRow + 1 : rows - 1;
////        newCol = oldCol;
//        break;
////    case Qt::Key_Left:
////        newRow = oldRow;
////        newCol = oldCol ? oldCol - 1 : 0;
////        break;
////    case Qt::Key_Right:
////        newRow = oldRow;
////        newCol = oldCol != cols - 1 ? oldCol + 1 : cols - 1;
////        break;
////    case Qt::Key_PageUp:
////        newRow = 0;
////        newCol = oldCol;
////        break;
////    case Qt::Key_PageDown:
////        newRow = rows - 1;
////        newCol = oldCol;
////        break;
////    case Qt::Key_Home:
////        newRow = event->modifiers() == Qt::CTRL ? 0 : oldRow;
////        newCol = 0;
////        break;
////    case Qt::Key_End:
////        newRow = event->modifiers() == Qt::CTRL ? rows - 1 : oldRow;
////        newCol = cols - 1;
////        break;
////    default:
////        QTableView::keyPressEvent(event);
////        return;
//    }

//    QModelIndex newIndex = indexAt(visualRect(model()->index(newRow, newCol)).center());
//    if (newIndex != oldIndex && dynamic_cast<ItemManager *>(model())->itemAt(newIndex))
//    {
//        currentChanged(newIndex, oldIndex);
//        emit currentItemChanged(newIndex);
//    }

    QModelIndex oldIndex = currentIndex();
    QTableView::keyPressEvent(event);

    int key = event->key();
    if (key == Qt::Key_Up || key == Qt::Key_Down || key == Qt::Key_Left || key == Qt::Key_Right || key == Qt::Key_PageUp || key == Qt::Key_PageDown || key == Qt::Key_Home || key == Qt::Key_End)
    {
        QModelIndex newIndex = indexAt(visualRect(currentIndex()).topLeft());
        //if (oldIndex != newIndex)
        {
			selectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::ClearAndSelect);
            //currentChanged(newIndex, oldIndex);
            //emit currentItemChanged(newIndex);
        }
    }
}
