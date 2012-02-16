#include "helpers.h"
#include "structs.h"

#include <QString>
#include <QStack>

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

bool colorStringsIndecesLessThan(const QPair<int, int> &a, const QPair<int, int> &b)
{
    return a.second < b.second;
}

QString htmlStringFromDiabloColorString(const QString &name, ColorIndex defaultColor)
{
    //QString htmlName = colorStrings.at(defaultColor) + name;
    //for (int i = 0; i < correctColorsNum; ++i) // replace color codes with their hex values for HTML
    //{
    //    QString replacement = colorReplacementString(i);
    //    htmlName.replace(QString("%1%2").arg(unicodeColorHeader).arg(colorCodes.at(i)), replacement).replace(colorStrings.at(i), replacement);
    //}
    //htmlName.replace("\\grey;", colorReplacementString(White)); // ugly hack
    //return htmlName.remove(QRegExp("<font color = \"#[\\da-fA-F]{6}\"></font>")); // remove empty tags

    // text is white by default + stupid HTML with its newlines
    QString text = colorStrings.at(defaultColor) + name;
    text.replace("\\n", htmlLineBreak).replace("\\grey;", colorStrings.at(White));

    QList<QPair<int, int> > colorStringsIndeces; // <index_of_color_string_in_array, position_in_string>
    for (int i = 0; i < colors.size(); ++i)
    {
        QString colorString = colorStrings.at(i);
        int occurencesCount = text.count(colorString), position = 0, length = colorString.length();
        for (int j = 0; j < occurencesCount; ++j, position += length)
        {
            position = text.indexOf(colorString, position);
            colorStringsIndeces.append(qMakePair(i, position));
        }
    }

    // sort colorStringsIndeces by position in ascending order
    int colorsNumberInString = colorStringsIndeces.size();
    //for (int i = 0; i < colorsNumberInString - 1; ++i)
    //{
    //    QPair<int, int> a = colorStringsIndeces.at(i);
    //    for (int j = i + 1; j < colorsNumberInString; ++j)
    //    {
    //        QPair<int, int> b = colorStringsIndeces.at(j);
    //        if (a.second > b.second)
    //        {
    //            colorStringsIndeces[i] = b;
    //            colorStringsIndeces[j] = a;
    //            a = b;
    //        }
    //    }
    //}
    qSort(colorStringsIndeces.begin(), colorStringsIndeces.end(), colorStringsIndecesLessThan);

    QStack<QString> colorStringsStack;
    for (int i = 0; i < colorsNumberInString; ++i)
    {
        int index = colorStringsIndeces.at(i).first;
        int position = colorStringsIndeces.at(i).second + colorStrings.at(index).length(); // skip colorString
        QString coloredText = text.mid(position, i != colorsNumberInString - 1 ? colorStringsIndeces.at(i + 1).second - position : -1);

        QStringList lines = coloredText.split(htmlLineBreak);
        QString reversedLines;
        for (QStringList::const_iterator i = lines.end() - 1; i != lines.begin() - 1; --i)
        {
            reversedLines.append(*i);
            if (i != lines.begin())
                reversedLines.append(htmlLineBreak);
        }
        if (!reversedLines.isEmpty())
            colorStringsStack.push(QString("<font color = \"%1\">%2</font>").arg(colorHexString(colors.at(index)), reversedLines));
    }

    // empty stack
    QString result;
    while (!colorStringsStack.isEmpty())
        result += colorStringsStack.pop();
    return result;
}

bool isCube(ItemInfo *item)
{
    return (item->storage == Enums::ItemStorage::Inventory || item->storage == Enums::ItemStorage::Stash) && item->itemType == "box";
}

bool hasChanged(ItemInfo *item)
{
    return item->hasChanged;
}
