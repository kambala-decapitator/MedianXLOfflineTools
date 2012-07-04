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
    ItemInfo *item = itemAtIndex(index);
    if (item)
    {
        const ItemBase &itemBase = ItemDataBase::Items()->value(item->itemType);
        QString imageName = item->itemType == "jew" ? "invjw" : itemBase.imageName; // quick hack for jewel
        if (item->variableGraphicIndex && QRegExp("\\d$").indexIn(imageName) == -1) // some items already have correct name despite the variableGraphicIndex (e.g. Assur's Bane)
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
                    p.setOpacity(0.6);
                    p.drawPixmap(0, 0, pixmap);
                    p.end();

                    pixmap = transparent;
                }
                return pixmap;
            }
        case Qt::ToolTipRole:
            return ItemDataBase::completeItemName(item, false);
        case Qt::ForegroundRole:
            return Qt::white;
        }
    }

    if (role == Qt::BackgroundRole) // not in switch because it doesn't depend on item
        return Qt::black;

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
    Q_UNUSED(action);

    ItemInfo *droppedItem = itemFromMimeData(mimeData);
    QModelIndex newIndex = index(row, column), oldIndex = index(droppedItem->row, droppedItem->column);
    removeItem(droppedItem);
    droppedItem->row = newIndex.row();
    droppedItem->column = newIndex.column();
    addItem(droppedItem);
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
//    qDebug() << roleDataMap;
    return itemAtIndex(index(rowFrom, colFrom));
}
