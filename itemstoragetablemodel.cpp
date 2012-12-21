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
//        QPainter p(&pixmap);
//        p.setOpacity(0.6);
//        p.end();
    }

    if (_dragOriginIndex != index)
    {
        ItemInfo *item = itemAtIndex(index);
        if (item)
        {
            QString imageName = item->itemType == "jew" ? "invjw" : ItemDataBase::Items()->value(item->itemType)->imageName; // quick hack for jewel
            bool isUniqueImage = false;
            if (item->quality == Enums::ItemQuality::Unique)
            {
                UniqueItemInfo *uniqueInfo = ItemDataBase::Uniques()->value(item->setOrUniqueId);
                if (uniqueInfo && !uniqueInfo->imageName.isEmpty())
                {
                    imageName = uniqueInfo->imageName;
                    isUniqueImage = true;
                }
            }
            if (!isUniqueImage && item->variableGraphicIndex && QRegExp("\\d$").indexIn(imageName) == -1)
                imageName += QString::number(item->variableGraphicIndex);
            QString imagePath = ResourcePathManager::pathForImageName(imageName);

            switch(role)
            {
            case Qt::DisplayRole:
                if (!QFile::exists(imagePath))
                    return imageName + ".dc6";
                break;
            case Qt::DecorationRole:
                if (QFile::exists(imagePath))
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
                return ItemDataBase::completeItemName(item, false);
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
