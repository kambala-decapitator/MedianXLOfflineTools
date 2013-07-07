#include "findresultswidget.h"
#include "itemsviewerdialog.h"
#include "itemparser.h"
#include "itemdatabase.h"
#include "itemnamestreewidget.hpp"

#include <QGroupBox>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

#include <QSettings>
#include <QList>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


FindResultsWidget::FindResultsWidget(QWidget *parent) : QWidget(parent), _resultsTreeWidget(new ItemNamesTreeWidget(this))
{
    QPushButton *expandAllButton = new QPushButton(tr("Expand all"), this), *collapseAllButton = new QPushButton(tr("Collapse all"), this);
    connect(expandAllButton, SIGNAL(clicked()), _resultsTreeWidget, SLOT(expandAll()));
    connect(collapseAllButton, SIGNAL(clicked()), _resultsTreeWidget, SLOT(collapseAll()));

    QHBoxLayout *hboxLayout = new QHBoxLayout;
    hboxLayout->addWidget(expandAllButton);
    hboxLayout->addStretch();
    hboxLayout->addWidget(collapseAllButton);

    QVBoxLayout *groupBoxLayout = new QVBoxLayout(this);
    groupBoxLayout->addWidget(_resultsTreeWidget);
    groupBoxLayout->addLayout(hboxLayout);

    setMinimumHeight(sizeHint().height());

    selectItemDelegate = new ShowSelectedItemDelegate(_resultsTreeWidget, this);
}

void FindResultsWidget::updateItems(QList<SearchResultItem> *newItems)
{
    _resultsTreeWidget->clear();
    _foundItemsMap.clear();

    for (int i = ItemsViewerDialog::GearIndex; i <= ItemsViewerDialog::LastIndex; ++i)
    {
        ItemsList items;
        foreach (const SearchResultItem &searchItem, *newItems)
            items += searchItem.first;
        ItemsList locationItems = ItemDataBase::itemsStoredIn(Enums::ItemStorage::metaEnum().value(i), i == ItemsViewerDialog::GearIndex, 0, &items);
        _foundItemsMap[i] = locationItems;

        QString topLevelItemText = ItemsViewerDialog::tabNameAtIndex(i);
        if (!locationItems.isEmpty())
            topLevelItemText += QString(" (%1)").arg(locationItems.size());

        QTreeWidgetItem *topLevelItem = new QTreeWidgetItem(QStringList(topLevelItemText));
        topLevelItem->setForeground(0, Qt::white);
        topLevelItem->addChildren(treeItemsForItems(locationItems));
        _resultsTreeWidget->addTopLevelItem(topLevelItem);
        _resultsTreeWidget->setColumnWidth(0, width() / 2);

        for (int k = 0, n = locationItems.size(); k < n; ++k)
        {
            ItemInfo *item = locationItems.at(k);
            foreach (const SearchResultItem &searchItem, *newItems)
            {
                if (searchItem.first == item)
                {
                    QTreeWidgetItem *childItem = topLevelItem->child(k);
                    for (int j = 0; j < childItem->columnCount(); ++j)
                        childItem->setToolTip(j, searchItem.second);
                    break;
                }
            }
        }
    }
    _resultsTreeWidget->expandAll();
}

void FindResultsWidget::selectItem(ItemInfo *item)
{
    int topItemIndex = ItemsViewerDialog::tabIndexFromItemStorage(item->storage);
    _resultsTreeWidget->setCurrentItem(_resultsTreeWidget->topLevelItem(topItemIndex)->child(_foundItemsMap[topItemIndex].indexOf(item)));
}

ItemInfo *FindResultsWidget::itemForCurrentTreeItem() const
{
    QTreeWidgetItem *treeItem = _resultsTreeWidget->currentItem();
    QTreeWidgetItem *parentItem = treeItem->parent(); // parentItem != 0 when this is an item, not a storage name
    return parentItem ? _foundItemsMap[_resultsTreeWidget->indexOfTopLevelItem(parentItem)].at(parentItem->indexOfChild(treeItem)) : 0;
}

void FindResultsWidget::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);
    _resultsTreeWidget->setColumnWidth(0, _resultsTreeWidget->width() / 2);
}
