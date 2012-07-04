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
    virtual ~ItemStorageTableModel() {}

    virtual int    rowCount(const QModelIndex &parent = QModelIndex()) const { Q_UNUSED(parent); return _rows; }
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const { Q_UNUSED(parent); return _columns; }
    virtual QVariant   data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    virtual Qt::DropActions supportedDropActions() const { return Qt::MoveAction; }
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

    int itemCount() const { return _itemsHash.size(); }
    ItemsList items() const;
    bool canStoreItemWithMimeDataAtIndex(const QMimeData *mimeData, const QModelIndex &index) const;

    ItemInfo *itemAtIndex(const QModelIndex &modelIndex) const { return _itemsHash[qMakePair(modelIndex.row(), modelIndex.column())]; }
    ItemInfo *firstItem() { return _itemsHash.begin() == _itemsHash.end() ? 0 : _itemsHash.begin().value(); }
    ItemInfo *itemFromMimeData(const QMimeData *mimeData) const;

    void setItems(const ItemsList &newItems);
    void addItem(ItemInfo *item) { _itemsHash[qMakePair(item->row, item->column)] = item; }
    void removeItem(ItemInfo *item) { _itemsHash.remove(qMakePair(item->row, item->column)); }

signals:
    void itemMoved(const QModelIndex &newIndex, const QModelIndex &oldIndex);

private:
    int _rows, _columns;

    typedef QPair<int, int> TableKey;
    QHash<TableKey, ItemInfo *> _itemsHash;
};

#endif // ITEMSTORAGETABLEMODEL_H
