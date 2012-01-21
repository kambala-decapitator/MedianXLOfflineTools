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
    explicit PropertiesViewerWidget(/*ItemsList *allItems, */QWidget *parent = 0);

    void displayItemProperties(ItemInfo *item);
    void clear() { displayItemProperties(0); }

private slots:
    void removeAllMysticOrbs();
	void currentItemTabChanged(int index);

private:
    Ui::PropertiesViewerWidget ui;
    ItemInfo *_item;
    QSet<int> _mysticOrbs;

    void setProperties(QTextEdit *textEdit, const QMap<int, ItemProperty> &properties, bool shouldClearText = true);
    void renderItemDescription(QTextEdit *textEdit, QString *description = 0);
    int indexOfPropertyValue(int id);
    void modifyMysticOrbProperty(int id, int decrement);
    QString propertyDisplay(const ItemProperty &propDisplay, int propId);
    void addProperties(QMap<int, ItemProperty> *mutableProps, const QMap<int, ItemProperty> &propsToAdd);
};

#endif // PROPERTIESVIEWERWIDGET_H
