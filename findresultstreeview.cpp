#include "findresultstreeview.h"
#include "findresultstreemodel.h"


FindResultsTreeView::FindResultsTreeView(QWidget *parent) : QTreeView(parent)
{
    _findResultsModel = new FindResultsTreeModel(this);
    setModel(_findResultsModel);
}
