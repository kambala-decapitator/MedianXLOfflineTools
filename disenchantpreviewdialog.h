#ifndef DISENCHANTPREVIEWDIALOG_H
#define DISENCHANTPREVIEWDIALOG_H

#include <QDialog>

#include "showselecteditemdelegate.h"
#include "structs.h"


class ItemNamesTreeWidget;

class DisenchantPreviewDialog : public QDialog, public ShowSelectedItemInterface
{
    Q_OBJECT

public:
    DisenchantPreviewDialog(ItemsList *items, QWidget *parent = 0);
    virtual ~DisenchantPreviewDialog() {}

    virtual ItemInfo *itemForTreeItem(QTreeWidgetItem *treeItem);

    ItemsList selectedItems() const;

protected:
    bool eventFilter(QObject *obj, QEvent *e);

private:
    ItemNamesTreeWidget *_itemsTreeWidget;
    ItemsList _items;
};

#endif // DISENCHANTPREVIEWDIALOG_H
