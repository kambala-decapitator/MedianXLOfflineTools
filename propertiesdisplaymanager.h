#ifndef PROPERTIESDISPLAYMANAGER_H
#define PROPERTIESDISPLAYMANAGER_H

#include "structs.h"

#include <QString>


class PropertiesDisplayManager
{
    Q_DECLARE_TR_FUNCTIONS(PropertiesDisplayManager)

public:
    static QString completeItemDescription(ItemInfo *item); // this is an ugly copy-paste from properties viewer, but I didn't find a better way; currently used for search
    static void addProperties(PropertiesMap *mutableProps, const PropertiesMap &propsToAdd);
    static void constructPropertyStrings(const PropertiesMap &properties, QMap<quint8, ItemPropertyDisplay> *outDisplayPropertiesMap);
    static QString propertyDisplay(const ItemProperty &propDisplay, int propId);

    static const QList<QByteArray> damageToUndeadTypes;
};

#endif // PROPERTIESDISPLAYMANAGER_H
