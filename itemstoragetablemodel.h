#ifndef ITEMSTORAGETABLEMODEL_H
#define ITEMSTORAGETABLEMODEL_H

#include "structs.h"

#include <QAbstractTableModel>
#include <QHash>


class ItemStorageTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ItemStorageTableModel(int rows, int columns, QObject *parent = 0) : QAbstractTableModel(parent), _rows(rows), _columns(columns) {}

    virtual int    rowCount(const QModelIndex &parent = QModelIndex()) const { Q_UNUSED(parent); return _rows; }
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const { Q_UNUSED(parent); return _columns; }
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    int itemCount() const { return _itemsHash.size(); }

    ItemInfo *itemAt(const QModelIndex &modelIndex) const { return _itemsHash[qMakePair(modelIndex.row(), modelIndex.column())]; }
    ItemInfo *firstItem() { return _itemsHash.begin() == _itemsHash.end() ? 0 : _itemsHash.begin().value(); }

    void setItems(const ItemsList &newItems);
    void addItem(ItemInfo *item) { _itemsHash[qMakePair(item->row, item->column)] = item; }
    void removeItem(ItemInfo *item) { _itemsHash.remove(qMakePair(item->row, item->column)); }

private:
    int _rows, _columns;

    typedef QPair<int, int> TableKey;
    QHash<TableKey, ItemInfo *> _itemsHash;
};

#endif // ITEMSTORAGETABLEMODEL_H
