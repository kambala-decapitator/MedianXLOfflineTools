#include "itemmanager.h"
#include "itemdatabase.h"

//extern "C" {
//#include "dc6con.h"
//}

#include <QFile>

#include <QPixmap>
#include <QImage>
#include <QPainter>


QVariant ItemManager::modelData(const QModelIndex &index, int role) const
{
    ItemInfo *item = itemAt(index);
    if (item)
    {
        const ItemBase &itemBase = ItemDataBase::Items()->value(item->itemType);
        QString imageName = itemBase.imageName, imagePath = resourcesPath + "/data/images/" + imageName;
        if (item->variableGraphicIndex)
            imagePath += QString::number(item->variableGraphicIndex);

        switch(role)
        {
        case Qt::DisplayRole:
        {
//            QString itemName = ItemDataBase::Items()->value(item->itemType).name, specialName;
//            if (item->quality == Enums::ItemQuality::Set)
//                specialName = ItemDataBase::Sets()->value(item->setOrUniqueId).itemName;
//            else if (item->quality == Enums::ItemQuality::Unique)
//                specialName = ItemDataBase::Uniques()->value(item->setOrUniqueId).name;
//            else if (item->isRW)
//                specialName = item->rwName;
//            else if (item->isStarter)
//                specialName = tr("starter");

//            if (!specialName.isEmpty())
//                itemName.prepend(QString("[%1] ").arg(specialName));
//            return itemName.replace("\\n", "\n");
            if (!QFile::exists(imagePath + ".jpg") && !QFile::exists(imagePath + ".gif"))
            {
                if (QFile::exists(imagePath + ".dc6"))
                    imageName += ".dc6";
                return imageName;
            }
			break;
        }
        case Qt::DecorationRole:
        {
            if (QFile::exists(imagePath + ".jpg"))
                imagePath += ".jpg";
            else if (QFile::exists(imagePath + ".gif"))
                imagePath += ".gif";
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
//            return imagePath.contains('.') ? QPixmap(imagePath) : QPixmap();
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
			}
//            else
//                return QPixmap();
			break;
        }
        case Qt::ToolTipRole:
        {
            return ItemDataBase::completeItemName(item, false);
		}
		case Qt::StatusTipRole:
		{
			return ItemDataBase::completeItemName(item, false).replace("\n", " ");
		}
        case Qt::ForegroundRole:
            return Qt::white;
//            switch(item->quality)
//            {
//            case Enums::ItemQuality::LowQuality: case Enums::ItemQuality::Normal: case Enums::ItemQuality::HighQuality:
//                return item->isSocketed ? colors.at(5) : colors.at(0);
//            case Enums::ItemQuality::Magic:
//                return colors.at(3);
//            case Enums::ItemQuality::Set:
//                return colors.at(2);
//            case Enums::ItemQuality::Rare:
//                return colors.at(8);
//            case Enums::ItemQuality::Unique:
//                return colors.at(4);
//            case Enums::ItemQuality::Crafted:
//                return colors.at(7);
//            default: // honorific I guess
//                return Qt::darkGreen;
//            }
        }
    }
    if (role == Qt::BackgroundRole)
        return Qt::black;

    return QVariant();
}
