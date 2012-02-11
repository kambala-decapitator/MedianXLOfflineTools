#include "findresultstreemodel.h"
#include "itemsviewerdialog.h"
#include "itemdatabase.h"
#include "itemparser.h"
#include "enums.h"


FindResultsTreeModel::FindResultsTreeModel(QObject *parent) : QAbstractItemModel(parent)
{

}

int FindResultsTreeModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    if (parent.isValid())
        return _foundItemsMap[parent.row()].size();
    else
        return ItemsViewerDialog::tabNames.size();
}

QModelIndex FindResultsTreeModel::index(int row, int column, const QModelIndex &parent /*= QModelIndex()*/) const
{
    if (!parent.isValid())
        return createIndex(row, column);

    const ItemsList &items = _foundItemsMap[row];
    if (items.size())
        return createIndex(row, column);
    else
        return QModelIndex();
}

QVariant FindResultsTreeModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
    if (index.parent().isValid()) // item
    {
        ItemInfo *item = _foundItemsMap[index.parent().row()].at(index.row());
        return ItemDataBase::Items()->value(item->itemType).name;
    }
    else
        return ItemsViewerDialog::tabNames.at(index.row());
}

void FindResultsTreeModel::updateItems(ItemsList *newItems)
{
    for (int i = ItemsViewerDialog::GearIndex; i <= ItemsViewerDialog::LastIndex; ++i)
    {
        ItemsList locationItems = ItemParser::itemsLocatedAt(Enums::ItemStorage::metaEnum().value(i), newItems, i == ItemsViewerDialog::GearIndex);
        _foundItemsMap[i] = locationItems;
    }
}
