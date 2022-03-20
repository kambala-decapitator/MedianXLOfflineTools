#include "disenchantpreviewmodel.h"
#include "itemnamestreewidget.hpp"
#include "itemdatabase.h"

#include <QStringList>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


Qt::ItemFlags DisenchantPreviewModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags_ = QAbstractItemModel::flags(index);
    if (index.column() == CheckboxColumn)
        flags_ |= Qt::ItemIsUserCheckable;
    return flags_;
}

QVariant DisenchantPreviewModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
    ItemInfo *item = _items.at(index.row());

    switch (role)
    {
    case Qt::DisplayRole:
        switch (index.column())
        {
        case PageColumn:
            if (item->plugyPage)
                return item->plugyPage;
            break;
        case QualityColumn:
            return item->quality == Enums::ItemQuality::Set ? tr("set") : tr("unique");
        case BaseNameColumn: case SpecialNameColumn:
        {
            QString completeItemName = ItemDataBase::completeItemName(item, false, false);
            QStringList names = completeItemName.split(kHtmlLineBreak);
            if (index.column() == SpecialNameColumn)
                return names.size() < 3 ? names.at(0) : QString("%1 %2").arg(names.at(1), names.at(0));
            else if (names.size() > 1)
                return names.last();
            break;
        }
        default:
            break;
        }
        break;
    case Qt::CheckStateRole:
        if (index.column() == CheckboxColumn)
            return _uncheckedIndexesSet.contains(index.row()) ? Qt::Unchecked : Qt::Checked;
        break;
    case Qt::ForegroundRole:
        return index.column() > QualityColumn ? ColorsManager::colors().at(ItemDataBase::colorOfItem(item)) : Qt::white;
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
        switch (section)
        {
        case CheckboxColumn:
            return QString();
        case PageColumn:
            return tr("Page");
        case QualityColumn:
            return tr("Quality");
        case BaseNameColumn:
            return ItemNamesTreeWidget::headerLabels().at(0);
        case SpecialNameColumn:
            return ItemNamesTreeWidget::headerLabels().at(1);
        default:
            break;
        }
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
