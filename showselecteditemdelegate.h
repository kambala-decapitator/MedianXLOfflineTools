#ifndef SHOWSELECTEDITEMDELEGATE_H
#define SHOWSELECTEDITEMDELEGATE_H

#include <QObject>


class ShowSelectedItemDelegate;
class ItemInfo;

class QTreeWidgetItem;

class ShowSelectedItemInterface
{
public:
    ShowSelectedItemDelegate *selectItemDelegate;

    virtual ItemInfo *itemForCurrentTreeItem() const = 0;
};


class QTreeView;
class QEvent;

class ShowSelectedItemDelegate : public QObject
{
    Q_OBJECT

public:
    ShowSelectedItemDelegate(QTreeView *treeWidget, ShowSelectedItemInterface *selectItemInterface);
    virtual ~ShowSelectedItemDelegate() {}

    virtual bool eventFilter(QObject *obj, QEvent *event);

signals:
    void showItem(ItemInfo *item);

private:
    QTreeView *_treeView;
    ShowSelectedItemInterface *_selectItemInterface;
};

#endif // SHOWSELECTEDITEMDELEGATE_H
