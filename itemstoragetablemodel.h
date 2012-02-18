#ifndef ITEMSTORAGETABLEMODEL_H
#define ITEMSTORAGETABLEMODEL_H

#include "structs.h"

#include <QAbstractTableModel>
#include <QHash>


class ItemStorageTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    static const int columns;

    explicit ItemStorageTableModel(int rows, QObject *parent = 0) : QAbstractTableModel(parent), _rows(rows) {}

    virtual int    rowCount(const QModelIndex &parent = QModelIndex()) const { Q_UNUSED(parent); return _rows; }
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const { Q_UNUSED(parent); return columns; }
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    virtual ItemInfo *itemAt(const QModelIndex &modelIndex) const { return _itemsHash[qMakePair(modelIndex.row(), modelIndex.column())]; }
    void setItems(const ItemsList &newItems);
    ItemInfo *firstItem() { return _itemsHash.begin().value(); }

private:
    int _rows;

    typedef QPair<int, int> TableKey;
    QHash<TableKey, ItemInfo *> _itemsHash;
};

#endif // ITEMSTORAGETABLEMODEL_H
