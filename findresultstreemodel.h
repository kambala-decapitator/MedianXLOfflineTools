#ifndef FINDRESULTSTREEMODEL_H
#define FINDRESULTSTREEMODEL_H

#include <QAbstractItemModel>


class FindResultsTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    FindResultsTreeModel(QObject *parent = 0);

    virtual int    rowCount(const QModelIndex &parent = QModelIndex()) const { Q_UNUSED(parent); return 1; }
    virtual int    columnCount(const QModelIndex &parent = QModelIndex()) const { Q_UNUSED(parent); return 1; }
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const { return QModelIndex(); }
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const { return QVariant(); }
    virtual QModelIndex    parent(const QModelIndex &index) const { return QModelIndex(); }

private:
    
};

#endif // FINDRESULTSTREEMODEL_H
