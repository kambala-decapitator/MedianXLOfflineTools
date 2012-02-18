#ifndef PROPERTIESVIEWERWIDGET_H
#define PROPERTIESVIEWERWIDGET_H

#include "ui_propertiesviewerwidget.h"

#include "structs.h"


struct ItemInfo;

class PropertiesViewerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PropertiesViewerWidget(QWidget *parent = 0);

    void displayItemProperties(ItemInfo *item);
    void clear() { displayItemProperties(0); }
    bool hasMysticOrbs() const { return _itemMysticOrbs.size() + _rwMysticOrbs.size() > 0; }

    const QString htmlLine; // it's intended that it's a class member and not static

public slots:
    void removeAllMysticOrbs();

private:
    Ui::PropertiesViewerWidget ui;
    ItemInfo *_item;
    QSet<int> _itemMysticOrbs, _rwMysticOrbs;

    QString displayProperties(const PropertiesMap &properties);
    void renderItemDescription(QTextEdit *textEdit, const QString &description);

    void removeMysticOrbsFromProperties(const QSet<int> &mysticOrbs, PropertiesMultiMap *props);
    int indexOfPropertyValue(int id, PropertiesMultiMap *props);
    void modifyMysticOrbProperty(int id, int decrement, PropertiesMultiMap *props);
    int totalMysticOrbValue(int moCode, PropertiesMap *props);

    QString collectMysticOrbsDataFromProps(QSet<int> *moSet, const PropertiesMap &props, bool isClassCharm);
};

#endif // PROPERTIESVIEWERWIDGET_H
