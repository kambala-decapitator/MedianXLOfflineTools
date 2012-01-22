#include "itemstoragetablemodel.h"
#include "itemdatabase.h"

#include <QFile>

#include <QPixmap>
#include <QImage>
#include <QPainter>


const int ItemStorageTableModel::columns = 10;

void ItemStorageTableModel::setItems(const ItemsList &newItems)
{
    beginResetModel();

    _itemsHash.clear();
    foreach (ItemInfo *item, newItems)
        _itemsHash[qMakePair(item->row, item->column)] = item;
    
	endResetModel();
}

QVariant ItemStorageTableModel::data(const QModelIndex &index, int role) const
{
	ItemInfo *item = itemAt(index);
	if (item)
	{
		const ItemBase &itemBase = ItemDataBase::Items()->value(item->itemType);
		// quick hack for jewel
		QString imageName = item->itemType != "jew" ? itemBase.imageName : "invjw", imagePath = DATA_PATH("images/" + imageName);
		if (item->variableGraphicIndex)
			imagePath += QString::number(item->variableGraphicIndex);

		switch(role)
		{
		case Qt::DisplayRole:
			if (!QFile::exists(imagePath + ".png"))
				return imageName + ".dc6";
			break;
		case Qt::DecorationRole:
			if (QFile::exists(imagePath + ".png"))
				imagePath += ".png";
			//            else if (QFile::exists(imagePath + ".dc6"))
			//            {
			//                int w, h;
			//                unsigned char *imageData;
			//                dc6ImageData(qPrintable(imagePath + ".dc6"), imageData, &w, &h);
			//                QImage dc6Image(imageData, w, h, QImage::Format_Invalid);
			//                QPixmap pixmap = QPixmap::fromImage(dc6Image);
			//                free(imageData);
			//                return pixmap;
			//            }
			if (imagePath.contains('.'))
			{
				QPixmap pixmap(imagePath);
				if (item->isEthereal)
				{
					// apply transparency: http://www.developer.nokia.com/Community/Wiki/CS001515_-_Transparent_QPixmap_picture

					// Create new picture for transparent
					QPixmap transparent(pixmap.size());

					// Do transparency
					transparent.fill(Qt::transparent);
					QPainter p(&transparent);
					p.setCompositionMode(QPainter::CompositionMode_Source);
					p.drawPixmap(0, 0, pixmap);
					p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
					// Set transparency level to 150 (possible values are 0-255)
					// The alpha channel of a color specifies the transparency effect, 
					// 0 represents a fully transparent color, while 255 represents 
					// a fully opaque color.
					p.fillRect(transparent.rect(), QColor(0, 0, 0, 127));
					p.end();

					// Set original picture's reference to new transparent one
					pixmap = transparent;
				}
				return pixmap;
				break;
			}
		case Qt::ToolTipRole:
			return ItemDataBase::completeItemName(item, false);
		case Qt::StatusTipRole:
			return ItemDataBase::completeItemName(item, false).replace("\n", " ");
		case Qt::ForegroundRole:
			return Qt::white;
		}
	}
	if (role == Qt::BackgroundRole) // not in switch because it doesn't depend on item
		return Qt::black;

	return QVariant();
}
