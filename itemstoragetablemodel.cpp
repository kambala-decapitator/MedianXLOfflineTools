#include "itemstoragetablemodel.h"
#include "structs.h"

#include <QApplication>
#include <QFont>


void ItemStorageTableModel::setItems(const ItemsList &newItems)
{
    beginResetModel();
    _itemsHash.clear();
    foreach (ItemInfo *item, newItems)
        _itemsHash[qMakePair(item->row, item->column)] = item;
    endResetModel();
}

QVariant ItemStorageTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::FontRole)
    {
        QFont font = qApp->font();
        font.setPointSize(font.pointSize() - 2);
        return font;
    }
    return ItemManager::modelData(index, role);
}
