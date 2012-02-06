#ifndef FINDRESULTSTREEVIEW_H
#define FINDRESULTSTREEVIEW_H

#include <QTreeView>


class FindResultsTreeModel;

class FindResultsTreeView : public QTreeView
{
    Q_OBJECT

public:
    FindResultsTreeView(QWidget *parent);

private:
    FindResultsTreeModel *_findResultsModel;
};

#endif // FINDRESULTSTREEVIEW_H
