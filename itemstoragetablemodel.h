#ifndef ITEMSTORAGETABLEMODEL_H
#define ITEMSTORAGETABLEMODEL_H

#include "itemmanager.h"

#include <QAbstractTableModel>
#include <QHash>


struct ItemInfo;

class ItemStorageTableModel : public QAbstractTableModel, public ItemManager
{
    Q_OBJECT

public:
    explicit ItemStorageTableModel(int rows, QObject *parent = 0) : QAbstractTableModel(parent), _rows(rows) {}

    virtual int	rowCount(const QModelIndex &parent = QModelIndex()) const { Q_UNUSED(parent); return _rows; }
    virtual int	columnCount(const QModelIndex &parent = QModelIndex()) const { Q_UNUSED(parent); return 10; }
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    virtual ItemInfo *itemAt(const QModelIndex &modelIndex) const { return _itemsHash[qMakePair(modelIndex.row(), modelIndex.column())]; }
    virtual void setItems(const ItemsList &newItems);

private:
    int _rows;

    typedef QPair<int, int> TableKey;
    QHash<TableKey, ItemInfo *> _itemsHash;
};

#endif // ITEMSTORAGETABLEMODEL_H
