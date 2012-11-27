#include "showselecteditemdelegate.h"

#include <QTreeWidget>
#include <QKeyEvent>


ShowSelectedItemDelegate::ShowSelectedItemDelegate(QTreeView *treeWidget, ShowSelectedItemInterface *selectItemInterface) : QObject(dynamic_cast<QObject *>(selectItemInterface)), _treeView(treeWidget),
    _selectItemInterface(selectItemInterface)
{
    _treeView->installEventFilter(this);
    _treeView->viewport()->installEventFilter(this); // mouse clicks are delivered to viewport rather than the widget itself
}

bool ShowSelectedItemDelegate::eventFilter(QObject *obj, QEvent *event)
{
    bool shouldShowItem = false;
    if (obj == _treeView)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            shouldShowItem = keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return;
        }
    }
    else if (obj == _treeView->viewport())
        shouldShowItem = event->type() == QEvent::MouseButtonDblClick;

    if (shouldShowItem)
    {
        if (ItemInfo *item = _selectItemInterface->itemForCurrentTreeItem())
        {
            emit showItem(item);
            return true;
        }
    }

    return QObject::eventFilter(obj, event);
}
