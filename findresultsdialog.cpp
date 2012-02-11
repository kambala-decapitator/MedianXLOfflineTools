#include "findresultsdialog.h"
#include "itemsviewerdialog.h"
#include "itemparser.h"
#include "itemdatabase.h"

#include <QTreeWidget>
#include <QCloseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

#include <QSettings>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


FindResultsDialog::FindResultsDialog(ItemsList *items, QWidget *parent) : QDialog(parent), _resultsTreeWidget(new QTreeWidget(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(windowFlags() | Qt::Tool);
    setStyleSheet("QTreeWidget {background-color: black;}");
    setWindowTitle(tr("Search results"));

    _resultsTreeWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _resultsTreeWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    _resultsTreeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    _resultsTreeWidget->setHeaderHidden(true);
    _resultsTreeWidget->setUniformRowHeights(true);
    //_resultsTreeWidget->setStyleSheet("background-color: black");
    _resultsTreeWidget->setColumnCount(1);

    updateItems(items);

    QPushButton *expandAllButton = new QPushButton(tr("Expand all"), this), *collapseAllButton = new QPushButton(tr("Collapse all"), this);
    connect(expandAllButton, SIGNAL(clicked()), _resultsTreeWidget, SLOT(expandAll()));
    connect(collapseAllButton, SIGNAL(clicked()), _resultsTreeWidget, SLOT(collapseAll()));

    QHBoxLayout *hboxLayout = new QHBoxLayout;
    hboxLayout->addWidget(expandAllButton);
    hboxLayout->addStretch();
    hboxLayout->addWidget(collapseAllButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(_resultsTreeWidget);
    mainLayout->addLayout(hboxLayout);

    restoreGeometry(QSettings().value("findResultsGeometry").toByteArray());

    _resultsTreeWidget->installEventFilter(this);
    _resultsTreeWidget->viewport()->installEventFilter(this); // mouse clicks are delivered to viewport rather than to the widget itself
}

void FindResultsDialog::closeEvent(QCloseEvent *e)
{
    saveSettings();
    e->accept();
}

void FindResultsDialog::saveSettings()
{
    QSettings().setValue("findResultsGeometry", saveGeometry());
}

void FindResultsDialog::updateItems(ItemsList *newItems)
{
    _resultsTreeWidget->clear();
    _foundItemsMap.clear();

    for (int i = ItemsViewerDialog::GearIndex; i <= ItemsViewerDialog::LastIndex; ++i)
    {
        ItemsList locationItems = ItemParser::itemsLocatedAt(Enums::ItemStorage::metaEnum().value(i), newItems, i == ItemsViewerDialog::GearIndex);
        _foundItemsMap[i] = locationItems;

        QString topLevelItemText = ItemsViewerDialog::tabNames.at(i);
        if (locationItems.size())
            topLevelItemText += QString(" (%1)").arg(locationItems.size());
        QTreeWidgetItem *topLevelItem = new QTreeWidgetItem(QStringList(topLevelItemText));
        topLevelItem->setForeground(0, Qt::white);

        foreach (ItemInfo *item, locationItems)
        {
            QTreeWidgetItem *childItem = new QTreeWidgetItem(QStringList(ItemDataBase::completeItemName(item, false, false).replace("<br>", " ", Qt::CaseInsensitive).replace(QRegExp("\\s+"), " ")));
            childItem->setToolTip(0, ItemDataBase::completeItemName(item, false));
            if (i >= ItemsViewerDialog::PersonalStashIndex)
                childItem->setText(0, QString("[%1] ").arg(tr("p. %1", "page abbreviation").arg(item->plugyPage)) + childItem->text(0));
            childItem->setForeground(0, colors.at(ItemDataBase::colorOfItem(item)));

            topLevelItem->addChild(childItem);
        }
        _resultsTreeWidget->addTopLevelItem(topLevelItem);
    }
    _resultsTreeWidget->expandAll();
}

void FindResultsDialog::selectItem(ItemInfo *item)
{
    int topItemIndex = ItemsViewerDialog::indexFromItemStorage(item->storage);
    _resultsTreeWidget->setCurrentItem(_resultsTreeWidget->topLevelItem(topItemIndex)->child(_foundItemsMap[topItemIndex].indexOf(item)));
}

bool FindResultsDialog::eventFilter(QObject *obj, QEvent *event)
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

    return QDialog::eventFilter(obj, event);
}
