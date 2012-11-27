#ifndef FINDRESULTSDIALOG_H
#define FINDRESULTSDIALOG_H

#include <QWidget>

#include "showselecteditemdelegate.h"
#include "structs.h"


typedef QPair<ItemInfo *, QString> SearchResultItem;

class ItemNamesTreeWidget;
class QEvent;

class FindResultsWidget : public QWidget, public ShowSelectedItemInterface
{
    Q_OBJECT

public:
    FindResultsWidget(QWidget *parent = 0);
    virtual ~FindResultsWidget() {}

    void updateItems(QList<SearchResultItem> *newItems);
    void selectItem(ItemInfo *item);

    virtual ItemInfo *itemForCurrentTreeItem() const;

private:
    ItemNamesTreeWidget *_resultsTreeWidget;
    QMap<int, ItemsList> _foundItemsMap;
};

#endif // FINDRESULTSDIALOG_H
