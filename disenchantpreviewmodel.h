#ifndef DISENCHANTPREVIEWMODEL_H
#define DISENCHANTPREVIEWMODEL_H

#include <QAbstractItemModel>

#include <QSet>

#include "structs.h"


class DisenchantPreviewModel : public QAbstractItemModel
{
public:
    enum
    {
        CheckboxColumn = 0,
        PageColumn,
        QualityColumn,
        BaseNameColumn,
        SpecialNameColumn
    };

    DisenchantPreviewModel(const ItemsList &items, QObject *parent = 0) : QAbstractItemModel(parent), _items(items), _rows(items.size()) {}
    virtual ~DisenchantPreviewModel() {}

    virtual int     rowCount(const QModelIndex &parent = QModelIndex()) const { Q_UNUSED(parent); return _rows; }
    virtual int  columnCount(const QModelIndex &parent = QModelIndex()) const { Q_UNUSED(parent); return 5; }
    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const { Q_UNUSED(parent); return !parent.isValid(); }

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const { Q_UNUSED(parent); return createIndex(row, column); }
    virtual QModelIndex parent(const QModelIndex &index) const { Q_UNUSED(index); return QModelIndex(); }

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant       data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    const ItemsList  &items()  const { return _items; }
    int uncheckedItemsCount()  const { return _uncheckedIndexesSet.size(); }

private:
    ItemsList _items;
    int _rows;
    QSet<int> _uncheckedIndexesSet;
};

#endif // DISENCHANTPREVIEWMODEL_H
