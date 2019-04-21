#ifndef PROPERTIESDISPLAYMANAGER_H
#define PROPERTIESDISPLAYMANAGER_H

#include "structs.h"

#include <QString>


class PropertiesDisplayManager
{
    Q_DECLARE_TR_FUNCTIONS(PropertiesDisplayManager)

public:
    enum SecondaryDamageUsage
    {
        Unused,
        UsedWithPrimary,
        UsedWithoutPrimary
    };

    static QString completeItemDescription(ItemInfo *item, bool useColor = false); // this is an ugly copy-paste from properties viewer, but I didn't find a better way; currently used for search
    static void addProperties(PropertiesMap *mutableProps, const PropertiesMap &propsToAdd, const QSet<int> *pIgnorePropIds = 0);
    static void addTemporaryPropertiesAndDelete(PropertiesMap *mutableProps, const PropertiesMap &tempPropsToAdd, const QSet<int> *pIgnorePropIds = 0);
    // currently shouldColor is used for reanimates' names only
    static void constructPropertyStrings(const PropertiesMap &properties, QMap<quint8, ItemPropertyDisplay> *outDisplayPropertiesMap, bool shouldColor = false, ItemInfo *item = 0);
    static SecondaryDamageUsage secondaryDamageUsage(int secondaryDamageId, int secondaryDamageValue, const PropertiesMap &allProperties, ItemInfo *item);
    static QString propertyDisplay(ItemProperty *propDisplay, int propId, bool shouldColor = false);
    static QString propertiesToHtml(const PropertiesMap &properties, ItemInfo *item = 0, int textColor = ColorsManager::Blue);

    static PropertiesMap socketableProperties(ItemInfo *socketableItem, qint8 socketableType);
    static PropertiesMap genericSocketableProperties(ItemInfo *socketableItem, qint8 socketableType);

    static void addChallengeNamesToClassCharm(PropertiesMap::iterator &iter);
    static PropertiesMultiMap collectSetFixedProps(const QList<SetFixedProperty> &setProps, quint8 propsNumber = 0);

    static const QList<QByteArray> kDamageToUndeadTypes;
};

#endif // PROPERTIESDISPLAYMANAGER_H
