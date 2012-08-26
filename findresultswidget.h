#ifndef FINDRESULTSDIALOG_H
#define FINDRESULTSDIALOG_H

#include <QWidget>

#include "structs.h"


typedef QPair<ItemInfo *, QString> SearchResultItem;

class ItemNamesTreeWidget;
class QEvent;

class FindResultsWidget : public QWidget
{
    Q_OBJECT

public:
    FindResultsWidget(QWidget *parent = 0);
    virtual ~FindResultsWidget() {}

    void updateItems(QList<SearchResultItem> *newItems);
    void selectItem(ItemInfo *item);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

signals:
    void showItem(ItemInfo *item);

private:
    ItemNamesTreeWidget *_resultsTreeWidget;
    QMap<int, ItemsList> _foundItemsMap;
};

#endif // FINDRESULTSDIALOG_H
