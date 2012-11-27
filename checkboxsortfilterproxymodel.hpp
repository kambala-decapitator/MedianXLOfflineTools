#ifndef CHECKBOXSORTFILTERPROXYMODEL_HPP
#define CHECKBOXSORTFILTERPROXYMODEL_HPP

#include <QSortFilterProxyModel>


class CheckboxSortFilterProxyModel : public QSortFilterProxyModel
{
public:
    CheckboxSortFilterProxyModel(QObject *parent = 0) : QSortFilterProxyModel(parent) {}
    virtual ~CheckboxSortFilterProxyModel() {}

protected:
    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const
    {
        if (!sortColumn()) // own sorting only for checkboxes column
            return sourceModel()->data(left, Qt::CheckStateRole).toBool() < sourceModel()->data(right, Qt::CheckStateRole).toBool();
        return QSortFilterProxyModel::lessThan(left, right);
    }
};

#endif // !CHECKBOXSORTFILTERPROXYMODEL_HPP
