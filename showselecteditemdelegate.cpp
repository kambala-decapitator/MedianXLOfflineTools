#include "showselecteditemdelegate.h"

#include <QTreeWidget>
#include <QKeyEvent>


ShowSelectedItemDelegate::ShowSelectedItemDelegate(QTreeWidget *treeWidget, ShowSelectedItemInterface *selectItemInterface) : QObject(selectItemInterface), _treeWidget(treeWidget), _selectItemInterface(selectItemInterface)
{
    _treeWidget->installEventFilter(this);
    _treeWidget->viewport()->installEventFilter(this); // mouse clicks are delivered to viewport rather than to the widget itself
}

bool ShowSelectedItemDelegate::eventFilter(QObject *obj, QEvent *event)
{
    bool shouldShowItem = false;
    if (obj == _treeWidget)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            shouldShowItem = keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return;
        }
    }
    else if (obj == _treeWidget->viewport())
        shouldShowItem = event->type() == QEvent::MouseButtonDblClick;

    if (shouldShowItem)
    {
        if (ItemInfo *item = _selectItemInterface->itemForTreeItem(_treeWidget->currentItem()))
        {
            emit showItem(item);
            return true;
        }
    }

    return QObject::eventFilter(obj, event);
}
