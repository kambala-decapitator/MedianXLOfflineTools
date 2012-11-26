#ifndef DISENCHANTPREVIEWMODEL_H
#define DISENCHANTPREVIEWMODEL_H

#include <QAbstractItemModel>

#include <QSet>

#include "structs.h"


class DisenchantPreviewModel : public QAbstractItemModel
{
public:
    DisenchantPreviewModel(const ItemsList &items, QObject *parent = 0);
    virtual ~DisenchantPreviewModel() {}

    virtual int     rowCount(const QModelIndex &parent = QModelIndex()) const { Q_UNUSED(parent); return _rows; }
    virtual int  columnCount(const QModelIndex &parent = QModelIndex()) const { Q_UNUSED(parent); return 4; }
    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const { Q_UNUSED(parent); return !parent.isValid(); }

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const { return createIndex(row, column); }
    virtual QModelIndex parent(const QModelIndex &index) const { return QModelIndex(); }

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant       data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    //virtual void sort(int column, Qt::SortOrder order /* = Qt::AscendingOrder */);

    const ItemsList &items() const { return _items; }
    int uncheckedItemsCount() const { return _uncheckedIndexesSet.size(); }

private:
    int _rows;
    ItemsList _items;
    QSet<int> _uncheckedIndexesSet;
};

#endif // DISENCHANTPREVIEWMODEL_H
