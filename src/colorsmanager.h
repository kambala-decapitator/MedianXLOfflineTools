#ifndef COLORSMANAGER_H
#define COLORSMANAGER_H

#include <QByteArray>
#include <QColor>
#include <QList>
#include <QString>

class QTextCodec;

class ColorsManager
{
public:
    // header bytes
    static QByteArray ansiColorHeader();

    static const QString &unicodeColorHeader();

#ifdef Q_OS_MAC
    static QTextCodec *macTextCodec();
#endif


    // color data
    enum ColorIndex
    {
        NoColor = -1,
        White,
        Red,
        Green,
        Blue,
        Gold,
        DarkGrey,
        //Black,
        Tan,
        Orange,
        Yellow,
        DarkGreen,
        Purple,
        //DarkGreen2,
        LastColor = Purple
    };

    // last byte in the color code
    static const QList<QLatin1Char> &colorCodes();

    // user-readable color codes: unicodeColorHeader + colorCodes.at(i) == colorStrings.at(i)
    static const QList<QByteArray> &colorStrings();

    // real colors
    static const QList<QColor> &colors();

    static int correctColorsNum() { return colors().size(); }
};

#endif // COLORSMANAGER_H
