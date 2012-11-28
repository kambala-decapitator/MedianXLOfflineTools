#ifndef CHECKBOXSORTFILTERPROXYMODEL_HPP
#define CHECKBOXSORTFILTERPROXYMODEL_HPP

#include <QSortFilterProxyModel>

#include "disenchantpreviewmodel.h"


class CheckboxSortFilterProxyModel : public QSortFilterProxyModel
{
public:
    CheckboxSortFilterProxyModel(QObject *parent = 0) : QSortFilterProxyModel(parent) {}
    virtual ~CheckboxSortFilterProxyModel() {}

protected:
    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const
    {
        if (sortColumn() == DisenchantPreviewModel::CheckboxColumn) // own sorting only for checkboxes column
            return sourceModel()->data(left, Qt::CheckStateRole).toBool() < sourceModel()->data(right, Qt::CheckStateRole).toBool();
        return QSortFilterProxyModel::lessThan(left, right);
    }

    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
    {
        // use only 'name' columns for filtering
        QAbstractItemModel *model = sourceModel();
        QRegExp regexp = filterRegExp();
        return model->data(model->index(source_row, DisenchantPreviewModel::BaseNameColumn,    source_parent)).toString().contains(regexp) ||
               model->data(model->index(source_row, DisenchantPreviewModel::SpecialNameColumn, source_parent)).toString().contains(regexp);
    }
};

#endif // !CHECKBOXSORTFILTERPROXYMODEL_HPP
