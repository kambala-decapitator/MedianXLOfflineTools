#ifndef COLORSMANAGER_HPP
#define COLORSMANAGER_HPP

#include <QColor>

#if defined(Q_OS_WIN32)
#include <Windows.h>
#elif defined(Q_OS_MAC)
#include <QTextCodec>
#endif


class ColorsManager
{
public:
    // header bytes
    static const QByteArray &ansiColorHeader()
    {
        static QByteArray ansiColorHeader;
        if (ansiColorHeader.isEmpty())
            ansiColorHeader.append(0xFF).append(0x63);
        return ansiColorHeader;
    }

    static const QString &unicodeColorHeader()
    {
        static QString unicodeColorHeader;
        if (unicodeColorHeader.isEmpty())
        {
            const QByteArray &ansiHeader = ansiColorHeader();
#if defined(Q_OS_WIN32)
            // Blizzard uses ANSI codepage to read save files' names
            int ansiHeaderSizeWithTerminator = ansiHeader.length() + 1;
            LPWSTR unicodeWChar = new WCHAR[ansiHeaderSizeWithTerminator];
            MultiByteToWideChar(CP_ACP, 0, ansiHeader.data(), ansiHeaderSizeWithTerminator, unicodeWChar, ansiHeaderSizeWithTerminator);
            unicodeColorHeader = QString::fromWCharArray(unicodeWChar);
            delete [] unicodeWChar;
#elif defined(Q_OS_MAC)
            unicodeColorHeader = macTextCodec()->toUnicode(ansiHeader);
#else
#warning Make sure to fix next line
            unicodeColorHeader = ansiHeader;
#endif
        }
        return unicodeColorHeader;
    }

#ifdef Q_OS_MAC
    static QTextCodec *macTextCodec() { return QTextCodec::codecForName("MacRoman"); }
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
        LastColor = Purple
    };

    // last byte in the color code
    static const QList<QLatin1Char> &colorCodes()
    {
        static const QList<QLatin1Char> colorCodes = QList<QLatin1Char>()
            << QLatin1Char('0')
            << QLatin1Char('1')
            << QLatin1Char('2')
            << QLatin1Char('3')
            << QLatin1Char('4')
            << QLatin1Char('5')
            // black was here
            << QLatin1Char('7')
            << QLatin1Char('8')
            << QLatin1Char('9')
            << QLatin1Char(':')
            << QLatin1Char(';')
            // strange colors below
            << QLatin1Char(')')
            << QLatin1Char('!')
            << QLatin1Char('+')
            << QLatin1Char('!')
            << QLatin1Char('%')
            << QLatin1Char(',')
            << QLatin1Char('-')
            << QLatin1Char('&')
            << QLatin1Char('\'')
            ;
        return colorCodes;
    }

    // user-readable color codes: unicodeColorHeader + colorCodes.at(i) == colorStrings.at(i)
    static const QList<QByteArray> &colorStrings()
    {
        static const QList<QByteArray> colorStrings = QList<QByteArray>()
            << "\\white;"
            << "\\red;"
            << "\\green;"
            << "\\blue;"
            << "\\gold;"
            << "\\dgrey;"
            // black was here
            << "\\tan;"
            << "\\orange;"
            << "\\yellow;"
            << "\\dgreen;"
            << "\\purple;"
            ;
        return colorStrings;
    }

    // real colors
    static const QList<QColor> &colors()
    {
        static const QList<QColor> colors = QList<QColor>()
            << QColor(Qt::white)
            << QColor(Qt::red)
            << QColor(Qt::green)
            << QColor(80, 80, 200)
            << QColor(160, 145, 105)
            << QColor(Qt::darkGray)
            // black was here
            << QColor(170, 160, 120)
            << QColor(250, 170, 35)
            << QColor(Qt::yellow)
            << QColor(Qt::darkGreen)
            << QColor(150, 90, 250)
            ;
        return colors;
    }

    static int correctColorsNum() { return colors().size(); }
};

#endif // COLORSMANAGER_HPP
