#include "disenchantpreviewdialog.h"
#include "itemnamestreewidget.hpp"
#include "progressbarmodal.hpp"

#include <QLabel>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QKeyEvent>
#include <QPushButton>
#include <QAction>
#include <QMenu>

#include <QSettings>


DisenchantPreviewDialog::DisenchantPreviewDialog(const ItemsList &items, QWidget *parent) : QDialog(parent), _label(new QLabel(this)), _itemsTreeWidget(new ItemNamesTreeWidget(this)), _items(items), _selectedItemsCount(0)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Disenchant preview"));

    _buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    QObject::connect(_buttonBox, SIGNAL(accepted()), SLOT(accept()));
    QObject::connect(_buttonBox, SIGNAL(rejected()), SLOT(reject()));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(_label);
    layout->addWidget(_itemsTreeWidget);
    layout->addWidget(_buttonBox);

    selectItemDelegate = new ShowSelectedItemDelegate(_itemsTreeWidget, this);
    _itemsTreeWidget->installEventFilter(static_cast<QDialog *>(this)); // to intercept pressing Space
    _itemsTreeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    _itemsTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_itemsTreeWidget, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(showTreeWidgetContextMenu(const QPoint &)));

    loadSettings();
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
        if (checkboxOfTreeItem(treeItem)->isChecked())
            items += _items.at(i);
    }
    return items;
}

bool DisenchantPreviewDialog::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == _itemsTreeWidget && e->type() == QEvent::KeyPress && static_cast<QKeyEvent *>(e)->key() == Qt::Key_Space)
    {
        changeSelectedItemsCheckState();
        return true;
    }
    return QDialog::eventFilter(obj, e);
}

void DisenchantPreviewDialog::checkboxStateChanged(bool checked)
{
    checked ? ++_selectedItemsCount : --_selectedItemsCount;
    _buttonBox->button(QDialogButtonBox::Ok)->setEnabled(_selectedItemsCount > 0);
    updateLabelText();
}

void DisenchantPreviewDialog::showTreeWidgetContextMenu(const QPoint &pos)
{
    QList<QTreeWidgetItem *> selectedItems = _itemsTreeWidget->selectedItems();
    bool allChecked = true, allUnchecked = true;
    foreach (QTreeWidgetItem *treeItem, selectedItems)
    {
        bool isChecked = checkboxOfTreeItem(treeItem)->isChecked();
        allChecked = allChecked && isChecked;
        allUnchecked = allUnchecked && !isChecked;
    }

    QList<QAction *> actions;
    if (!allChecked)
    {
        QAction *actionCheckSelected = new QAction(tr("Check selected"), _itemsTreeWidget);
        actionCheckSelected->setData(Check);
        connect(actionCheckSelected, SIGNAL(triggered()), SLOT(changeSelectedItemsCheckState()));
        actions << actionCheckSelected;
    }
    if (!allChecked && !allUnchecked)
    {
        QAction *actionInvertSelected = new QAction(tr("Invert selected"), _itemsTreeWidget);
        actionInvertSelected->setData(Invert);
        connect(actionInvertSelected, SIGNAL(triggered()), SLOT(changeSelectedItemsCheckState()));
        actions << actionInvertSelected;
    }
    if (!allUnchecked)
    {
        QAction *actionUncheckSelected = new QAction(tr("Uncheck selected"), _itemsTreeWidget);
        actionUncheckSelected->setData(Uncheck);
        connect(actionUncheckSelected, SIGNAL(triggered()), SLOT(changeSelectedItemsCheckState()));
        actions << actionUncheckSelected;
    }

    QMenu::exec(actions, mapToGlobal(pos));
}

void DisenchantPreviewDialog::changeSelectedItemsCheckState()
{
    CheckAction checkAction;
    if (QAction *senderAction = qobject_cast<QAction *>(sender()))
        checkAction = static_cast<CheckAction>(senderAction->data().toUInt());
    else // space pressed
        checkAction = Invert;
    bool newCheckedState = static_cast<bool>(checkAction);

    QList<QTreeWidgetItem *> selectedItems = _itemsTreeWidget->selectedItems();
    foreach (QTreeWidgetItem *treeItem, selectedItems)
    {
        QCheckBox *checkbox = checkboxOfTreeItem(treeItem);
        if (checkAction == Invert)
            newCheckedState = !checkbox->isChecked();
        checkbox->setChecked(newCheckedState);
    }
}

void DisenchantPreviewDialog::updateLabelText()
{
    _label->setText(tr("Select items to disenchant (%1/%2 selected):").arg(_selectedItemsCount).arg(_items.size()));
}

QCheckBox *DisenchantPreviewDialog::checkboxOfTreeItem(QTreeWidgetItem *treeItem) const
{
    return qobject_cast<QCheckBox *>(_itemsTreeWidget->itemWidget(treeItem, 0));
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

    ProgressBarModal progressBar;
    progressBar.centerInWidget(this);
    progressBar.show();

    QList<QTreeWidgetItem *> treeItems = treeItemsForItems(_items);
    _itemsTreeWidget->addTopLevelItems(treeItems);
    foreach (QTreeWidgetItem *treeItem, treeItems)
    {
        qApp->processEvents();

        treeItem->setText(0, "    " + treeItem->text(0)); // FIXME: dirty hack to place checkboxes near text

        QCheckBox *checkBox = new QCheckBox(_itemsTreeWidget);
        checkBox->setChecked(true);
        checkBox->setFocusPolicy(Qt::NoFocus);
        connect(checkBox, SIGNAL(toggled(bool)), SLOT(checkboxStateChanged(bool)));
        _itemsTreeWidget->setItemWidget(treeItem, 0, checkBox);
    }
    _itemsTreeWidget->setRootIsDecorated(false);
    _itemsTreeWidget->resizeColumnToContents(0);
    _itemsTreeWidget->resizeColumnToContents(1);

    _selectedItemsCount = _items.size();
    updateLabelText();
}
