#include "findresultswidget.h"
#include "itemsviewerdialog.h"
#include "itemparser.h"
#include "itemdatabase.h"

#include <QGroupBox>
#include <QTreeWidget>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

#include <QSettings>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


FindResultsWidget::FindResultsWidget(QWidget *parent) : QWidget(parent), _groupBox(new QGroupBox(tr("Results"), this)), _resultsTreeWidget(new QTreeWidget(_groupBox))
{
    setStyleSheet("QTreeWidget { background-color: black; }"
                  "QTreeWidget::item { selection-color: red; }"
                  "QTreeWidget::item:hover { border: 1px solid #bfcde4; }"
                  "QTreeWidget::item:selected { border: 1px solid #567dbc; }"
                 );

    _resultsTreeWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _resultsTreeWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    _resultsTreeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    _resultsTreeWidget->setHeaderHidden(true);
    _resultsTreeWidget->setUniformRowHeights(true);
    _resultsTreeWidget->setColumnCount(1);

    QPushButton *expandAllButton = new QPushButton(tr("Expand all"), this), *collapseAllButton = new QPushButton(tr("Collapse all"), this);
    connect(expandAllButton, SIGNAL(clicked()), _resultsTreeWidget, SLOT(expandAll()));
    connect(collapseAllButton, SIGNAL(clicked()), _resultsTreeWidget, SLOT(collapseAll()));

    QHBoxLayout *hboxLayout = new QHBoxLayout;
    hboxLayout->addWidget(expandAllButton);
    hboxLayout->addStretch();
    hboxLayout->addWidget(collapseAllButton);

    QVBoxLayout *groupBoxLayout = new QVBoxLayout(_groupBox);
    groupBoxLayout->addWidget(_resultsTreeWidget);
    groupBoxLayout->addLayout(hboxLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(_groupBox);

    setMinimumHeight(sizeHint().height());

    _resultsTreeWidget->installEventFilter(this);
    _resultsTreeWidget->viewport()->installEventFilter(this); // mouse clicks are delivered to viewport rather than to the widget itself
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
        ItemsList locationItems = ItemParser::itemsLocatedAt(Enums::ItemStorage::metaEnum().value(i), &items, i == ItemsViewerDialog::GearIndex);
        _foundItemsMap[i] = locationItems;

        QString topLevelItemText = ItemsViewerDialog::tabNames.at(i);
        if (locationItems.size())
            topLevelItemText += QString(" (%1)").arg(locationItems.size());
        QTreeWidgetItem *topLevelItem = new QTreeWidgetItem(QStringList(topLevelItemText));
        topLevelItem->setForeground(0, Qt::white);

        foreach (ItemInfo *item, locationItems)
        {
            QString matchedText;
            foreach (const SearchResultItem &searchItem, *newItems)
                if (searchItem.first == item)
                {
                    matchedText = searchItem.second;
                    break;
                }

            QTreeWidgetItem *childItem = new QTreeWidgetItem(QStringList(ItemDataBase::completeItemName(item, false, false).replace(htmlLineBreak, " ", Qt::CaseInsensitive).replace(QRegExp("\\s+"), " ")));
            childItem->setToolTip(0, matchedText);
            if (i >= ItemsViewerDialog::PersonalStashIndex)
                childItem->setText(0, QString("[%1] ").arg(tr("p. %1", "page abbreviation").arg(item->plugyPage)) + childItem->text(0));
            childItem->setForeground(0, colors.at(ItemDataBase::colorOfItem(item)));

            topLevelItem->addChild(childItem);
        }
        _resultsTreeWidget->addTopLevelItem(topLevelItem);
    }
    _resultsTreeWidget->expandAll();
}

void FindResultsWidget::selectItem(ItemInfo *item)
{
    int topItemIndex = ItemsViewerDialog::indexFromItemStorage(item->storage);
    _resultsTreeWidget->setCurrentItem(_resultsTreeWidget->topLevelItem(topItemIndex)->child(_foundItemsMap[topItemIndex].indexOf(item)));
}

bool FindResultsWidget::eventFilter(QObject *obj, QEvent *event)
{
    bool shouldShowItem = false;
    if (obj == _resultsTreeWidget)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            shouldShowItem = keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return;
        }
    }
    else if (obj == _resultsTreeWidget->viewport())
        shouldShowItem = event->type() == QEvent::MouseButtonDblClick;

    if (shouldShowItem)
    {
        QTreeWidgetItem *currentItem = _resultsTreeWidget->currentItem(), *parentItem = currentItem->parent();
        if (parentItem) // this is an item, not a storage name
        {
            ItemInfo *item = _foundItemsMap[_resultsTreeWidget->indexOfTopLevelItem(parentItem)].at(parentItem->indexOfChild(currentItem));
            emit showItem(item);
            //return true;
        }
    }

    return QWidget::eventFilter(obj, event);
}
