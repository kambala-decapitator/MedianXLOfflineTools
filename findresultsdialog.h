#ifndef FINDRESULTSDIALOG_H
#define FINDRESULTSDIALOG_H

#include <QDialog>

#include "structs.h"


class FindResultsTreeModel;
class FindResultsTreeView;
class QCloseEvent;

class FindResultsDialog : public QDialog
{
    Q_OBJECT

public:
    FindResultsDialog(ItemsList *items, QWidget *parent);

    void saveSettings();

protected:
    void closeEvent(QCloseEvent *e);

private:
    FindResultsTreeModel *_resultsTreeModel;
    FindResultsTreeView *_resultsTreeView;
};

#endif // FINDRESULTSDIALOG_H
