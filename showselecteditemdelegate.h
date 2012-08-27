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

    virtual ItemInfo *itemForTreeItem(QTreeWidgetItem *treeItem) = 0;
};


class QTreeWidget;
class QEvent;

class ShowSelectedItemDelegate : public QObject
{
    Q_OBJECT

public:
    ShowSelectedItemDelegate(QTreeWidget *treeWidget, ShowSelectedItemInterface *selectItemInterface);
    virtual ~ShowSelectedItemDelegate() {}

protected:
    bool eventFilter(QObject *obj, QEvent *event);

signals:
    void showItem(ItemInfo *item);

private:
    QTreeWidget *_treeWidget;
    ShowSelectedItemInterface *_selectItemInterface;
};

#endif // SHOWSELECTEDITEMDELEGATE_H
