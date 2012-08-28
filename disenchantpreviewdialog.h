#ifndef DISENCHANTPREVIEWDIALOG_H
#define DISENCHANTPREVIEWDIALOG_H

#include <QDialog>

#include "showselecteditemdelegate.h"
#include "structs.h"


class ItemNamesTreeWidget;

class QLabel;
class QDialogButtonBox;

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

private slots:
    void checkboxStateChanged(bool checked);

private:
    QLabel *_label;
    ItemNamesTreeWidget *_itemsTreeWidget;
    QDialogButtonBox *_buttonBox;

    ItemsList _items;
    quint32 _selectedItemsCount;

    void updateLabelText();
};

#endif // DISENCHANTPREVIEWDIALOG_H
