#include "disenchantpreviewdialog.h"
#include "itemnamestreewidget.hpp"
#include "disenchantpreviewmodel.h"
#include "checkboxsortfilterproxymodel.hpp"

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

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


DisenchantPreviewDialog::DisenchantPreviewDialog(const ItemsList &items, bool areItemsFromSamePage, QWidget *parent /*= 0*/) : QDialog(parent), _label(new QLabel(this)), _itemsTreeView(new QTreeView(this)),
    _itemsTreeModel(new DisenchantPreviewModel(items, this)), _proxyModel(new CheckboxSortFilterProxyModel(this))
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
    _itemsTreeView->header()->setSortIndicator(DisenchantPreviewModel::PageColumn, Qt::AscendingOrder);
    _itemsTreeView->setRootIsDecorated(false);
    _itemsTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    _itemsTreeView->installEventFilter(static_cast<QDialog *>(this)); // to intercept pressing Space
    _itemsTreeView->setContextMenuPolicy(Qt::CustomContextMenu);

    _proxyModel->setSourceModel(_itemsTreeModel);
    _itemsTreeView->setModel(_proxyModel);
    if (areItemsFromSamePage)
        _itemsTreeView->hideColumn(DisenchantPreviewModel::PageColumn);

    connect(_itemsTreeView, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(showTreeViewContextMenu(const QPoint &)));
    connect(_buttonBox, SIGNAL(accepted()), SLOT(accept()));
    connect(_buttonBox, SIGNAL(rejected()), SLOT(reject()));

    loadSettings();
}

ItemInfo *DisenchantPreviewDialog::itemForCurrentTreeItem() const
{
    return itemAtRow(_itemsTreeView->currentIndex().row());
}

ItemInfo *DisenchantPreviewDialog::itemAtRow(int row) const
{
    QModelIndex index = _proxyModel->index(row, 0);
    QItemSelection selection(index, index);
    return _itemsTreeModel->items().at(_proxyModel->mapSelectionToSource(selection).indexes().first().row());
}

ItemsList DisenchantPreviewDialog::selectedItems() const
{
    ItemsList items;
    for (int i = 0, n = _itemsTreeModel->items().size(); i < n; ++i)
        if (_itemsTreeModel->isRowChecked(i))
            items += itemAtRow(i);
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
        bool isChecked = _proxyModel->data(_proxyModel->index(index.row(), DisenchantPreviewModel::CheckboxColumn), Qt::CheckStateRole).toBool();
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
        QModelIndex checkboxIndex = _proxyModel->index(index.row(), DisenchantPreviewModel::CheckboxColumn);
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

    for (int i = 0, n = _itemsTreeModel->columnCount(); i < n; ++i)
        _itemsTreeView->resizeColumnToContents(i);
    _itemsTreeView->setSortingEnabled(true);

    updateLabelTextAndOkButtonState();
}
