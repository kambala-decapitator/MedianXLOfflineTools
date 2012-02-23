#include "itemstoragetablemodel.h"
#include "itemdatabase.h"
#include "resourcepathmanager.hpp"

#include <QFile>

#include <QPixmap>
#include <QPainter>


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
    ItemInfo *item = itemAt(index);
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
