#include "itemstoragetablemodel.h"
#include "itemdatabase.h"
#include "resourcepathmanager.hpp"

#include <QPixmap>
#include <QPainter>

#include <QFile>
#include <QMimeData>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


void ItemStorageTableModel::setItems(const ItemsList &newItems)
{
    beginResetModel();

    _itemsHash.clear();
    foreach (ItemInfo *item, newItems)
        addItem(item);

    endResetModel();
}

QVariant ItemStorageTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::BackgroundRole) // not in switch because it doesn't depend on item
        return QColor(Qt::black);

    if (role == Qt::DecorationRole && _highlightIndexes.contains(index))
    {
        bool isGreen = true;
        ItemInfo *draggedItem = itemAtIndex(_dragOriginIndex);
        ItemsList items_ = items();
        items_.removeOne(draggedItem);
        foreach (const QModelIndex &anIndex, _highlightIndexes)
        {
            // 1x1 item should be checked, so let's use Arcane Crystal for this
            if (!ItemDataBase::canStoreItemAt(anIndex.row(), anIndex.column(), "qum", items_, _rows, _columns))
            {
                isGreen = false;
                break;
            }
        }

        QColor color(isGreen ? Qt::green : Qt::red);
        color.setAlpha(64);
        QPixmap pixmap(32, 32);
        pixmap.fill(color);
        return pixmap;
    }

    if (_dragOriginIndex != index)
    {
        ItemInfo *item = itemAtIndex(index);
        if (item)
        {
            ItemBase *baseInfo = ItemDataBase::Items()->value(item->itemType);
            QString imageName;
            if (item->quality == Enums::ItemQuality::Unique || item->quality == Enums::ItemQuality::Set)
            {
                SetOrUniqueItemInfo *setOrUniqueInfo = item->quality == Enums::ItemQuality::Set ? static_cast<SetOrUniqueItemInfo *>(ItemDataBase::Sets()   ->value(item->setOrUniqueId))
                                                                                                : static_cast<SetOrUniqueItemInfo *>(ItemDataBase::Uniques()->value(item->setOrUniqueId));
                if (setOrUniqueInfo && !setOrUniqueInfo->imageName.isEmpty())
                    imageName = setOrUniqueInfo->imageName;
            }
            if (imageName.isEmpty())
            {
                if (item->variableGraphicIndex)
                {
                    int i = item->variableGraphicIndex - 1;
                    const QList<QByteArray> &variableImageNames = ItemDataBase::ItemTypes()->value(baseInfo->types.at(0)).variableImageNames;
                    if (!variableImageNames.isEmpty())
                        imageName = i < variableImageNames.size() ? variableImageNames.at(i) : variableImageNames.last();
                }
                if (imageName.isEmpty())
                    imageName = baseInfo->imageName;
            }

            QString imagePath = ResourcePathManager::pathForItemImageName(imageName);
            bool doesImageExist = QFile::exists(imagePath);

            switch(role)
            {
            case Qt::DisplayRole:
                if (!doesImageExist)
                    return imageName + ".dc6";
                break;
            case Qt::DecorationRole:
                if (doesImageExist)
                {
                    QPixmap pixmap(imagePath);
                    if (item->isEthereal)
                    {
                        // apply transparency: http://www.developer.nokia.com/Community/Wiki/CS001515_-_Transparent_QPixmap_picture (modified because it doesn't work on Mac OS X)
                        QPixmap transparent(pixmap.size());
                        transparent.fill(Qt::transparent);
                        QPainter p(&transparent);
                        p.setOpacity(0.5);
                        p.drawPixmap(0, 0, pixmap);
                        p.end();

                        pixmap = transparent;
                    }
                    return pixmap;
                }
                break;
            case Qt::ToolTipRole:
            {
                QString tooltip = ItemDataBase::completeItemName(item, false);
                if (!doesImageExist)
                    tooltip += QString("%1%2.dc6").arg(kHtmlLineBreak, imageName);
                return tooltip;
            }
            case Qt::ForegroundRole:
                return QColor(Qt::white);
            }
        }
    }

    return QVariant();
}

Qt::ItemFlags ItemStorageTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags_ = QAbstractTableModel::flags(index);
    flags_ |= itemAtIndex(index) ? Qt::ItemIsDragEnabled : Qt::ItemIsDropEnabled;
    return flags_;
}

bool ItemStorageTableModel::dropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(action); Q_UNUSED(parent);

    ItemInfo *droppedItem = itemFromMimeData(mimeData);
    QModelIndex newIndex = index(row, column), oldIndex = index(droppedItem->row, droppedItem->column);
    if (newIndex != oldIndex)
    {
        removeItem(droppedItem);
        droppedItem->row = newIndex.row();
        droppedItem->column = newIndex.column();
        addItem(droppedItem);
    }
    emit itemMoved(newIndex, oldIndex);
    return true;
}

ItemsList ItemStorageTableModel::items() const
{
    ItemsList items_;
    foreach (ItemInfo *item, _itemsHash)
        items_ += item;
    return items_;
}

bool ItemStorageTableModel::canStoreItemWithMimeDataAtIndex(const QMimeData *mimeData, const QModelIndex &index) const
{
    ItemInfo *item = itemFromMimeData(mimeData);
    ItemsList items_ = items();
    items_.removeOne(item);
    return ItemDataBase::canStoreItemAt(index.row(), index.column(), item->itemType, items_, _rows, _columns);
}

ItemInfo *ItemStorageTableModel::itemFromMimeData(const QMimeData *mimeData) const
{
    QByteArray encoded = mimeData->data("application/x-qabstractitemmodeldatalist");
    QDataStream stream(&encoded, QIODevice::ReadOnly);
//    while (!stream.atEnd())
    int rowFrom, colFrom;
//    QMap<int,  QVariant> roleDataMap;
    stream >> rowFrom >> colFrom;// >> roleDataMap;
    return itemAtIndex(index(rowFrom, colFrom));
}
