#ifndef HELPERS_H
#define HELPERS_H

// message boxes
#include <QMessageBox>
#define CUSTOM_BOX(type, message) QMessageBox::type(this, qApp->applicationName(), message)
#define ERROR_BOX(message) CUSTOM_BOX(critical, message)
#define INFO_BOX(message) CUSTOM_BOX(information, message)
#define WARNING_BOX(message) CUSTOM_BOX(warning, message)
#define ERROR_BOX_NO_PARENT(message) QMessageBox::critical(0, qApp->applicationName(), message)

// string building
class QString;
static const QChar zeroChar('0');
QString binaryStringFromNumber(quint64 number, bool needsInversion = false, int fieldWidth = 8);
QString colorHexString(const QColor &c);
QString colorReplacementString(int colorIndex);

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
class ItemInfo;
bool isCube(ItemInfo *item);
bool hasChanged(ItemInfo *item);

#endif // HELPERS_H
