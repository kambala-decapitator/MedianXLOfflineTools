#ifndef FINDRESULTSDIALOG_H
#define FINDRESULTSDIALOG_H

#include <QDialog>

#include "structs.h"


class QCloseEvent;
class QTreeWidget;
class QEvent;

class FindResultsDialog : public QDialog
{
    Q_OBJECT

public:
    FindResultsDialog(ItemsList *items, QWidget *parent);

    void saveSettings();

    void updateItems(ItemsList *newItems);
    void selectItem(ItemInfo *item);

protected:
    void closeEvent(QCloseEvent *e);
    bool eventFilter(QObject *obj, QEvent *event);

signals:
    void showItem(ItemInfo *item);

private:
    QTreeWidget *_resultsTreeWidget;
    QMap<int, ItemsList> _foundItemsMap;
};

#endif // FINDRESULTSDIALOG_H
