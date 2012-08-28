#include "disenchantpreviewdialog.h"
#include "itemnamestreewidget.h"

#include <QLabel>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QKeyEvent>


DisenchantPreviewDialog::DisenchantPreviewDialog(ItemsList *items, QWidget *parent) : QDialog(parent), _itemsTreeWidget(new ItemNamesTreeWidget(this)), _items(*items)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Disenchant preview"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    QObject::connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Select items to disenchant:"), this));
    layout->addWidget(_itemsTreeWidget);
    layout->addWidget(buttonBox);

    selectItemDelegate = new ShowSelectedItemDelegate(_itemsTreeWidget, this);
    _itemsTreeWidget->installEventFilter(static_cast<QDialog *>(this)); // to intercept pressing Space

    QList<QTreeWidgetItem *> treeItems = treeItemsForItems(_items);
    foreach (QTreeWidgetItem *treeItem, treeItems)
        treeItem->setText(0, "    " + treeItem->text(0)); // FIXME: dirty hack to place checkboxes near text
    _itemsTreeWidget->addTopLevelItems(treeItems);
    _itemsTreeWidget->setColumnWidth(0, width() / 2);
    _itemsTreeWidget->setRootIsDecorated(false);
    // TODO: load/save settings
    for (int i = 0, n = _itemsTreeWidget->topLevelItemCount(); i < n; ++i)
    {
        QCheckBox *checkBox = new QCheckBox(_itemsTreeWidget);
        checkBox->setChecked(true);
        checkBox->setFocusPolicy(Qt::NoFocus);
        _itemsTreeWidget->setItemWidget(_itemsTreeWidget->topLevelItem(i), 0, checkBox);
    }

}

ItemInfo *DisenchantPreviewDialog::itemForTreeItem(QTreeWidgetItem *treeItem)
{
    return _items.at(_itemsTreeWidget->indexOfTopLevelItem(treeItem));
}

ItemsList DisenchantPreviewDialog::selectedItems() const
{
    ItemsList items;
    for (int i = 0, n = _itemsTreeWidget->topLevelItemCount(); i < n; ++i)
    {
        QTreeWidgetItem *treeItem = _itemsTreeWidget->topLevelItem(i);
        if (qobject_cast<QCheckBox *>(_itemsTreeWidget->itemWidget(treeItem, 0))->isChecked())
            items += _items.at(i);
    }
    return items;
}

bool DisenchantPreviewDialog::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == _itemsTreeWidget && e->type() == QEvent::KeyPress && static_cast<QKeyEvent *>(e)->key() == Qt::Key_Space)
    {
        QCheckBox *checkbox = qobject_cast<QCheckBox *>(_itemsTreeWidget->itemWidget(_itemsTreeWidget->currentItem(), 0));
        checkbox->setChecked(!checkbox->isChecked());
        return true;
    }
    return QDialog::eventFilter(obj, e);
}
