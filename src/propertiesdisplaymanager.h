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
    static void addProperties(PropertiesMultiMap *mutableProps, const PropertiesMap &propsToAdd, const QSet<int> *pIgnorePropIds = 0);
    static void addTemporaryPropertiesAndDelete(PropertiesMultiMap *mutableProps, const PropertiesMap &tempPropsToAdd, const QSet<int> *pIgnorePropIds = 0);
    // currently shouldColor is used for reanimates' names only
    static void constructPropertyStrings(const PropertiesMultiMap &properties, QMap<quint8, ItemPropertyDisplay> *outDisplayPropertiesMultiMap, bool shouldColor = false, ItemInfo *item = 0);
    static SecondaryDamageUsage secondaryDamageUsage(int secondaryDamageId, int secondaryDamageValue, const PropertiesMultiMap &allProperties, ItemInfo *item);
    static QString propertyDisplay(ItemProperty *propDisplay, int propId, bool shouldColor = false);
    static QString propertiesToHtml(const PropertiesMultiMap &properties, ItemInfo *item = 0, int textColor = ColorsManager::Blue);

    static PropertiesMultiMap socketableProperties(ItemInfo *socketableItem, qint8 socketableType);
    static PropertiesMultiMap genericSocketableProperties(ItemInfo *socketableItem, qint8 socketableType);

    static void addChallengeNamesToClassCharm(PropertiesMultiMap::iterator &iter);
    static PropertiesMultiMap collectSetFixedProps(const QList<SetFixedProperty> &setProps, quint8 propsNumber = 0);
    static QString propsToString(const PropertiesMultiMap &setProps);

    static QStringList weaponDamageBonuses(ItemBase *itemBase);

    static const QList<QByteArray> kDamageToUndeadTypes;
};

#endif // PROPERTIESDISPLAYMANAGER_H
