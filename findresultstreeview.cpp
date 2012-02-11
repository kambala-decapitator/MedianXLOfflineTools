#include "findresultstreeview.h"


FindResultsTreeView::FindResultsTreeView(QWidget *parent) : QTreeView(parent)
{
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    //setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    //setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setHeaderHidden(true);
    setRootIsDecorated(false);
    setUniformRowHeights(true);
}
