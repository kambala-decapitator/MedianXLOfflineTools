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
#include <QList>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


FindResultsWidget::FindResultsWidget(QWidget *parent) : QWidget(parent), _resultsTreeWidget(new QTreeWidget(this))
{
    setStyleSheet("QTreeWidget { background-color: black; }"
                  "QTreeWidget::item { selection-color: red; }"
                  "QTreeWidget::item:hover { border: 1px solid #bfcde4; }"
                  "QTreeWidget::item:selected { border: 1px solid #567dbc; }"
                 );

    _resultsTreeWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _resultsTreeWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    _resultsTreeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    _resultsTreeWidget->setUniformRowHeights(true);
    _resultsTreeWidget->setColumnCount(2);
    _resultsTreeWidget->setHeaderLabels(QStringList() << tr("Base name") << tr("Special name"));

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
        ItemsList locationItems = ItemDataBase::itemsStoredIn(Enums::ItemStorage::metaEnum().value(i), i == ItemsViewerDialog::GearIndex, 0, &items);
        _foundItemsMap[i] = locationItems;

        QString topLevelItemText = ItemsViewerDialog::tabNames.at(i);
        if (!locationItems.isEmpty())
            topLevelItemText += QString(" (%1)").arg(locationItems.size());
        QTreeWidgetItem *topLevelItem = new QTreeWidgetItem(QStringList(topLevelItemText));
        topLevelItem->setForeground(0, Qt::white);

        foreach (ItemInfo *item, locationItems)
        {
            QString matchedText;
            foreach (const SearchResultItem &searchItem, *newItems)
            {
                if (searchItem.first == item)
                {
                    matchedText = searchItem.second;
                    break;
                }
            }

            QString htmlName = ItemDataBase::completeItemName(item, true);
            QStringList list;
            QList<QColor> colors;

            QRegExp rx("<font color = \"(.+)\">(.+)</font>");
            rx.setMinimal(true);
            int matchIndex = 0;
            while ((matchIndex = rx.indexIn(htmlName, matchIndex)) != -1)
            {
                matchIndex += rx.cap().length();
                colors.prepend(rx.cap(1));
                list.prepend(rx.cap(2).replace(htmlLineBreak, " ", Qt::CaseInsensitive).trimmed());
            }

            QTreeWidgetItem *childItem = new QTreeWidgetItem(list);
            if (i >= ItemsViewerDialog::PersonalStashIndex)
                childItem->setText(0, QString("[%1] ").arg(tr("p. %1", "page abbreviation").arg(item->plugyPage)) + childItem->text(0));
            for (int i = 0; i < list.size(); ++i)
            {
                childItem->setForeground(i, colors.at(i));
                childItem->setToolTip(i, matchedText);
            }

            int newWidth = width() / 2;
            if (newWidth > _resultsTreeWidget->columnWidth(0))
                _resultsTreeWidget->setColumnWidth(0, newWidth);

            topLevelItem->addChild(childItem);
        }
        _resultsTreeWidget->addTopLevelItem(topLevelItem);
    }
    _resultsTreeWidget->expandAll();
}

void FindResultsWidget::selectItem(ItemInfo *item)
{
    int topItemIndex = ItemsViewerDialog::tabIndexFromItemStorage(item->storage);
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
        }
    }

    return QWidget::eventFilter(obj, event);
}
