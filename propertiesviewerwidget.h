#ifndef PROPERTIESVIEWERWIDGET_H
#define PROPERTIESVIEWERWIDGET_H

#include "ui_propertiesviewerwidget.h"

#include "structs.h"


struct ItemPropertyDisplay
{
    QString displayString;
    int priority, propertyId;

    ItemPropertyDisplay() {}
    ItemPropertyDisplay(const QString &displayString_, int priority_, int propertyId_) : displayString(displayString_), priority(priority_), propertyId(propertyId_) {}
};
Q_DECLARE_TYPEINFO(ItemPropertyDisplay, Q_MOVABLE_TYPE);


struct ItemInfo;

class PropertiesViewerWidget : public QWidget
{
    Q_OBJECT

public:
    typedef QMap<int, ItemProperty> PropertiesMap;

    explicit PropertiesViewerWidget(QWidget *parent = 0);

    void displayItemProperties(ItemInfo *item);
    void clear() { displayItemProperties(0); }
    bool hasMysticOrbs() const { return _itemMysticOrbs.size() + _rwMysticOrbs.size() > 0; }

    const QString htmlLine;

public slots:
    void removeAllMysticOrbs();

private slots:
#ifndef Q_WS_MACX
    void currentItemTabChanged(int index);
#endif

private:
    Ui::PropertiesViewerWidget ui;
    ItemInfo *_item;
    QSet<int> _itemMysticOrbs, _rwMysticOrbs;

    void displayProperties(QTextEdit *textEdit, const PropertiesMap &properties, bool shouldClearText = true);
    QString propertyDisplay(const ItemProperty &propDisplay, int propId);
    void renderItemDescription(QTextEdit *textEdit, QString *description = 0);

    void removeMysticOrbsFromProperties(const QSet<int> &mysticOrbs, PropertiesMultiMap *props);
    int indexOfPropertyValue(int id, PropertiesMultiMap *props);
    void modifyMysticOrbProperty(int id, int decrement, PropertiesMultiMap *props);
    int totalMysticOrbValue(int moCode, PropertiesMap *props);
    PropertiesMultiMap *propertiesWithCode(int code);

    void addProperties(PropertiesMap *mutableProps, const PropertiesMap &propsToAdd);
    void collectMysticOrbsDataFromProps(QSet<int> *moSet, const PropertiesMap &props, QTextEdit *textEdit, bool isClassCharm_);
    bool isClassCharm();
};

#endif // PROPERTIESVIEWERWIDGET_H
