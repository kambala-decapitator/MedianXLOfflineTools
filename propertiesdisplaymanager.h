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
    // currently shouldColor is used for reanimates' names only
    static void constructPropertyStrings(const PropertiesMap &properties, QMap<quint8, ItemPropertyDisplay> *outDisplayPropertiesMap, bool shouldColor = false);
    static QString propertyDisplay(const ItemProperty &propDisplay, int propId, bool shouldColor = false);
    static PropertiesMap genericSocketableProperties(ItemInfo *socketableItem, qint8 socketableType);

    static void addChallengeNamesToClassCharm(PropertiesMap::iterator &iter);

    static const QList<QByteArray> kDamageToUndeadTypes;
};

#endif // PROPERTIESDISPLAYMANAGER_H
