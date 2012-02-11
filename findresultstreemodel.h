#ifndef FINDRESULTSTREEMODEL_H
#define FINDRESULTSTREEMODEL_H

#include <QAbstractItemModel>
#include <QMap>

#include "structs.h"


class FindResultsTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    FindResultsTreeModel(QObject *parent = 0);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const { Q_UNUSED(parent); return 1; }
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    //virtual QVariant QAbstractItemModel::headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const { return QVariant(); }
    virtual QModelIndex parent(const QModelIndex &index) const { return QModelIndex(); }

    void updateItems(ItemsList *newItems);

private:
    QMap<int, ItemsList> _foundItemsMap;
};

#endif // FINDRESULTSTREEMODEL_H
