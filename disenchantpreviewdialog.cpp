#include "disenchantpreviewdialog.h"
#include "itemnamestreewidget.hpp"
#include "disenchantpreviewmodel.h"

#include <QLabel>
#include <QTreeView>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QKeyEvent>
#include <QPushButton>
#include <QAction>
#include <QMenu>
#include <QHeaderView>

#include <QSettings>
#include <QSortFilterProxyModel>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


DisenchantPreviewDialog::DisenchantPreviewDialog(const ItemsList &items, QWidget *parent) : QDialog(parent), _label(new QLabel(this)), _itemsTreeView(new QTreeView(this)), _itemsTreeModel(new DisenchantPreviewModel(items, this)), _proxyModel(new QSortFilterProxyModel(this))
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Disenchant preview"));

    selectItemDelegate = new ShowSelectedItemDelegate(_itemsTreeView, this);
    _buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(_label);
    layout->addWidget(_itemsTreeView);
    layout->addWidget(_buttonBox);

    customizeItemsTreeView(_itemsTreeView);
    _itemsTreeView->setRootIsDecorated(false);
    _itemsTreeView->setColumnWidth(0, 20);
    _itemsTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    _itemsTreeView->installEventFilter(static_cast<QDialog *>(this)); // to intercept pressing Space
    _itemsTreeView->setContextMenuPolicy(Qt::CustomContextMenu);

    _proxyModel->setSourceModel(_itemsTreeModel);
    _itemsTreeView->setModel(_proxyModel);

    connect(_itemsTreeView, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(showTreeViewContextMenu(const QPoint &)));
    //connect(_itemsTreeView, SIGNAL(clicked(const QModelIndex &)), SLOT(treeViewClicked(const QModelIndex &)));
    connect(_buttonBox, SIGNAL(accepted()), SLOT(accept()));
    connect(_buttonBox, SIGNAL(rejected()), SLOT(reject()));

    loadSettings();
}

ItemInfo *DisenchantPreviewDialog::itemForCurrentTreeItem()
{
    return _itemsTreeModel->items().at(_itemsTreeView->currentIndex().row());
}

ItemsList DisenchantPreviewDialog::selectedItems() const
{
    ItemsList items;
    //for (int i = 0, n = _itemsTreeView->topLevelItemCount(); i < n; ++i)
    //{
    //    QTreeWidgetItem *treeItem = _itemsTreeView->topLevelItem(i);
    //    if (checkboxOfTreeItem(treeItem)->isChecked())
    //        items += _items.at(i);
    //}
    return items;
}

bool DisenchantPreviewDialog::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == _itemsTreeView && e->type() == QEvent::KeyPress && static_cast<QKeyEvent *>(e)->key() == Qt::Key_Space)
    {
        changeSelectedItemsCheckState();
        return true;
    }
    return QDialog::eventFilter(obj, e);
}

void DisenchantPreviewDialog::showTreeViewContextMenu(const QPoint &pos)
{
    bool allChecked = true, allUnchecked = true;
    foreach (const QModelIndex &index, _itemsTreeView->selectionModel()->selectedRows())
    {
        bool isChecked = _proxyModel->data(_proxyModel->index(index.row(), 0), Qt::CheckStateRole).toBool();
        allChecked = allChecked && isChecked;
        allUnchecked = allUnchecked && !isChecked;
        if (!allChecked && !allUnchecked)
            break;
    }

    QList<QAction *> actions;
    if (!allChecked)
    {
        QAction *actionCheckSelected = new QAction(tr("Check selected"), _itemsTreeView);
        actionCheckSelected->setData(Check);
        connect(actionCheckSelected, SIGNAL(triggered()), SLOT(changeSelectedItemsCheckState()));
        actions << actionCheckSelected;
    }
    if (!allChecked && !allUnchecked)
    {
        QAction *actionInvertSelected = new QAction(tr("Invert selected"), _itemsTreeView);
        actionInvertSelected->setData(Invert);
        connect(actionInvertSelected, SIGNAL(triggered()), SLOT(changeSelectedItemsCheckState()));
        actions << actionInvertSelected;
    }
    if (!allUnchecked)
    {
        QAction *actionUncheckSelected = new QAction(tr("Uncheck selected"), _itemsTreeView);
        actionUncheckSelected->setData(Uncheck);
        connect(actionUncheckSelected, SIGNAL(triggered()), SLOT(changeSelectedItemsCheckState()));
        actions << actionUncheckSelected;
    }

    QMenu::exec(actions, _itemsTreeView->viewport()->mapToGlobal(pos));
}

void DisenchantPreviewDialog::changeSelectedItemsCheckState()
{
    CheckAction checkAction;
    if (QAction *senderAction = qobject_cast<QAction *>(sender())) // context menu
        checkAction = static_cast<CheckAction>(senderAction->data().toUInt());
    else // space pressed
        checkAction = Invert;

    bool newCheckedState = static_cast<bool>(checkAction);
    foreach (const QModelIndex &index, _itemsTreeView->selectionModel()->selectedRows())
    {
        QModelIndex checkboxIndex = _proxyModel->index(index.row(), 0);
        if (checkAction == Invert)
            newCheckedState = !_proxyModel->data(checkboxIndex, Qt::CheckStateRole).toBool();
        _proxyModel->setData(checkboxIndex, newCheckedState, Qt::CheckStateRole);
    }

    updateLabelTextAndOkButtonState();
}

void DisenchantPreviewDialog::updateLabelTextAndOkButtonState()
{
    int total = _itemsTreeModel->items().size(), unchecked = _itemsTreeModel->uncheckedItemsCount();
    _label->setText(tr("Select items to disenchant (%1/%2 selected):").arg(total - unchecked).arg(total));
    _buttonBox->button(QDialogButtonBox::Ok)->setEnabled(total - unchecked > 0);
}

void DisenchantPreviewDialog::loadSettings()
{
    QSettings settings;
    settings.beginGroup("disenchantDialog");
    restoreGeometry(settings.value("geometry").toByteArray());
}

void DisenchantPreviewDialog::saveSettings()
{
    QSettings settings;
    settings.beginGroup("disenchantDialog");
    settings.setValue("geometry", saveGeometry());
}

void DisenchantPreviewDialog::showEvent(QShowEvent *e)
{
    Q_UNUSED(e);

    for (int i = 0; i < _itemsTreeModel->columnCount(); ++i)
        _itemsTreeView->resizeColumnToContents(i);

    //_itemsTreeView->header()->setSortIndicatorShown(true);
    _itemsTreeView->header()->setSortIndicator(1, Qt::AscendingOrder);
    _itemsTreeView->setSortingEnabled(true);
    //_itemsTreeView->sortByColumn(1);

    updateLabelTextAndOkButtonState();
}
