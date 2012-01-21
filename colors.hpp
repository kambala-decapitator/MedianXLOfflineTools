#ifndef COLORS_HPP
#define COLORS_HPP

#include <QColor>

#if defined(Q_WS_WIN32)
#include <Windows.h>
#elif defined(Q_WS_MACX)
#include <QTextCodec>
static QTextCodec *macTextCodec() { return QTextCodec::codecForName("MacRoman"); }
#endif


const char ansiChar = 0xFF;
const char ansiColorHeaderChars[] = {ansiChar, 0x63};
const QByteArray ansiColorHeader(ansiColorHeaderChars);
// Blizzard uses ANSI codepage to read save files' names
#if defined(Q_WS_WIN32)
static WCHAR unicodeWChar;
static int unusedValueJustToRemoveError = MultiByteToWideChar(CP_ACP, 0, &ansiChar, 1, &unicodeWChar, 1);
const QChar unicodeChar = unicodeWChar;
#elif defined(Q_WS_MACX)
const QChar unicodeChar = macTextCodec()->toUnicode(&ansiChar).at(0);
#else // just in case someone decides to compile under Linux
const QChar unicodeChar = ansiChar;
#endif
const QString unicodeColorHeader = QString("%1%2").arg(unicodeChar, QChar(0x63));

// last byte in the color code
const QList<QLatin1Char> colorCodes = QList<QLatin1Char>()
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

// real colors
const QList<QColor> colors = QList<QColor>()
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

// user-readable color codes: unicodeColorHeader + colorCodes.at(i) == colorStrings.at(i)
const QList<QByteArray> colorStrings = QList<QByteArray>()
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

const int correctColorsNum = colors.size(); // number of correctly working colors

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
    Purple
};

#endif // COLORS_HPP
