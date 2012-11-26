#include "disenchantpreviewmodel.h"
#include "itemnamestreewidget.hpp"
#include "itemdatabase.h"

#include <QStringList>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


DisenchantPreviewModel::DisenchantPreviewModel(const ItemsList &items, QObject *parent /*= 0*/) : QAbstractItemModel(parent), _items(items), _rows(items.size())
{
}

Qt::ItemFlags DisenchantPreviewModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags_ = QAbstractItemModel::flags(index);
    if (!index.column())
        flags_ |= Qt::ItemIsUserCheckable;
    return flags_;
}

QVariant DisenchantPreviewModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
    ItemInfo *item = _items.at(index.row());
    QString completeItemName = ItemDataBase::completeItemName(item, false, false);
    QStringList names = completeItemName.split(kHtmlLineBreak);

    switch (role)
    {
    case Qt::DisplayRole:
        switch (index.column())
        {
        case 1:
            if (item->plugyPage)
                return item->plugyPage;
            break;
        case 2:
            if (names.size() > 1)
                return names.at(1);
            break;
        case 3:
            return names.at(0);
        default:
            break;
        }
        break;
    case Qt::CheckStateRole:
        if (!index.column())
            return _uncheckedIndexesSet.contains(index.row()) ? Qt::Unchecked : Qt::Checked;
        break;
    case Qt::ForegroundRole:
        return index.column() > 1 ? ColorsManager::colors().at(ItemDataBase::colorOfItem(item)) : Qt::white;
    default:
        break;
    }

    return QVariant();
}

QVariant DisenchantPreviewModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    Q_UNUSED(orientation);

    if (role == Qt::DisplayRole)
    {
        if (section > 1)
            return ItemNamesTreeWidget::headerLabels().at(section - 2);
        else
            return section ? tr("Page") : QString();
    }

    return QVariant();
}

bool DisenchantPreviewModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
    if (role == Qt::CheckStateRole)
    {
        if (value.toBool())
            _uncheckedIndexesSet.remove(index.row());
        else
            _uncheckedIndexesSet += index.row();

        emit dataChanged(index, index);
        return true;
    }

    return false;
}
