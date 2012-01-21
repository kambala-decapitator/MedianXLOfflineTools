#include "helpers.h"
#include "structs.h"

#include <QString>

#include <algorithm>


QString binaryStringFromNumber(quint64 number, bool needsInversion, int fieldWidth)
{
    QString binaryString = QString("%1").arg(number, fieldWidth, 2, zeroChar);
    if (needsInversion)
        std::reverse(binaryString.begin(), binaryString.end());
    return binaryString;
}

QString colorHexString(const QColor &c)
{
    return QString("#%1%2%3").arg(c.red(), 2, 16, zeroChar).arg(c.green(), 2, 16, zeroChar).arg(c.blue(), 2, 16, zeroChar);
}

QString colorReplacementString(int colorIndex)
{
    return QString("</font><font color = \"%1\">").arg(colorHexString(colors.at(colorIndex)));
}

QString htmlStringFromDiabloColorString(const QString &name, ColorIndex defaultColor)
{
    QString htmlName = colorStrings.at(defaultColor) + name;
    for (int i = 0; i < correctColorsNum; ++i) // replace color codes with their hex values for HTML
    {
        QString replacement = colorReplacementString(i);
        htmlName.replace(QString("%1%2").arg(unicodeColorHeader).arg(colorCodes.at(i)), replacement).replace(colorStrings.at(i), replacement);
    }
    htmlName.replace("\\grey;", colorReplacementString(White)); // ugly hack
    return htmlName.remove(QRegExp("<font color = \"#[\\da-fA-F]{6}\"></font>")); // remove empty tags
}

bool isCube(ItemInfo *item)
{
	return (item->storage == Enums::ItemStorage::Inventory || item->storage == Enums::ItemStorage::Stash) && item->itemType == "box";
}

bool hasChanged( ItemInfo *item )
{
	return item->hasChanged;
}
