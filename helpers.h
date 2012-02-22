#ifndef HELPERS_H
#define HELPERS_H

// message boxes
#include <QMessageBox>
#define CUSTOM_BOX(type, message, buttons, defaultButton) QMessageBox::type(this, qApp->applicationName(), message, buttons, defaultButton)
#define QUESTION_BOX_YESNO(message, defaultButton) CUSTOM_BOX(question, message, QMessageBox::Yes | QMessageBox::No, defaultButton)
#define CUSTOM_BOX_OK(type, message) CUSTOM_BOX(type, message, QMessageBox::Ok, QMessageBox::Ok)
#define ERROR_BOX(message) CUSTOM_BOX_OK(critical, message)
#define INFO_BOX(message) CUSTOM_BOX_OK(information, message)
#define WARNING_BOX(message) CUSTOM_BOX_OK(warning, message)
#define ERROR_BOX_NO_PARENT(message) QMessageBox::critical(0, qApp->applicationName(), message)

// string building
class QString;
static const QChar zeroChar('0');
static const QString htmlLineBreak("<br>");
QString binaryStringFromNumber(quint64 number, bool needsInversion = false, int fieldWidth = 8);
QString colorHexString(const QColor &c);
QString coloredText(const QString &text, int colorIndex);

#include "colors.hpp"
QString htmlStringFromDiabloColorString(const QString &name, ColorIndex defaultColor = White);

// QMetaEnum getter
#include <QMetaEnum>
template<class T>
QMetaEnum metaEnumFromName(const char *enumName)
{
    const QMetaObject &metaObject = T::staticMetaObject;
    return metaObject.enumerator(metaObject.indexOfEnumerator(enumName));
}

// item conditions for STL algorithms
struct ItemInfo;
bool isCubeItem(ItemInfo *item);
bool hasChanged(ItemInfo *item);

#endif // HELPERS_H
