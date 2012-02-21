#include "helpers.h"
#include "structs.h"
#include "itemdatabase.h"

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

QString coloredText(const QString &text, int colorIndex)
{
    return QString("<font color = \"%1\">%2</font>").arg(colorHexString(colors.at(colorIndex)), text);
}

bool colorStringsIndecesLessThan(const QPair<int, int> &a, const QPair<int, int> &b)
{
    return a.second < b.second;
}

QString htmlStringFromDiabloColorString(const QString &name, ColorIndex defaultColor /*= White*/)
{
    QString text = name;
    if (defaultColor != NoColor)
        text.prepend(colorStrings.at(defaultColor));
    text.replace("\\n", htmlLineBreak).replace("\\grey;", colorStrings.at(White));

    QList<QPair<int, int> > colorStringsIndeces; // <index_of_color_string_in_array, position_in_string>
    for (int i = 0; i < colors.size(); ++i)
    {
        QString colorString = colorStrings.at(i);
        int occurencesCount = text.count(colorString), position = 0, length = colorString.length();
        for (int j = 0; j < occurencesCount; ++j, position += length)
        {
            position = text.indexOf(colorString, position);
            colorStringsIndeces += qMakePair(i, position);
        }
    }

    // sort colorStringsIndeces by position in ascending order
    qSort(colorStringsIndeces.begin(), colorStringsIndeces.end(), colorStringsIndecesLessThan);

    int colorsNumberInString = colorStringsIndeces.size();
    if (!colorsNumberInString)
        return text;

    QStack<QString> colorStringsStack;
    for (int i = 0; i < colorsNumberInString; ++i)
    {
        int index = colorStringsIndeces.at(i).first;
        int position = colorStringsIndeces.at(i).second + colorStrings.at(index).length(); // skip colorString
        QString coloredText_ = text.mid(position, i != colorsNumberInString - 1 ? colorStringsIndeces.at(i + 1).second - position : -1);

        QStringList lines = coloredText_.split(htmlLineBreak);
        QString reversedLines;
        for (QStringList::const_iterator i = lines.end() - 1; i != lines.begin() - 1; --i)
        {
            reversedLines.append(*i);
            if (i != lines.begin())
                reversedLines.append(htmlLineBreak);
        }
        if (!reversedLines.isEmpty())
            colorStringsStack.push(coloredText(reversedLines, index));
    }

    // empty stack
    QString result;
    while (!colorStringsStack.isEmpty())
        result += colorStringsStack.pop();
    return result;
}

bool isCubeItem(ItemInfo *item)
{
    return (item->storage == Enums::ItemStorage::Inventory || item->storage == Enums::ItemStorage::Stash) && ItemDataBase::isCube(item);
}

bool hasChanged(ItemInfo *item)
{
    return item->hasChanged;
}
