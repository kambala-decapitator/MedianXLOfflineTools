#ifndef PROPERTIESVIEWERWIDGET_H
#define PROPERTIESVIEWERWIDGET_H

#include "structs.h"

#include <QSet>


class ItemInfo;

namespace Ui { class PropertiesViewerWidget; }
class QTextEdit;

class PropertiesViewerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PropertiesViewerWidget(QWidget *parent = 0);
    virtual ~PropertiesViewerWidget();

    void showItem(ItemInfo *item);
    void clear() { showItem(0); }

    int mysticOrbsTotal() const { return _itemMysticOrbs.size() + _rwMysticOrbs.size(); }
    bool hasMysticOrbs() const { return mysticOrbsTotal() > 0; }
    const QSet<int> &mysticOrbs(bool isItemMo) const { return isItemMo ? _itemMysticOrbs : _rwMysticOrbs; }

    const QString htmlLine; // it's intended that it's a class member and not static

public slots:
    void removeAllMysticOrbs();
    void removeMysticOrb();

private:
    Ui::PropertiesViewerWidget *ui;
    ItemInfo *_item;
    QSet<int> _itemMysticOrbs, _rwMysticOrbs;

    QString propertiesToHtml(const PropertiesMap &properties, int textColor = ColorsManager::Blue);
    void renderHtml(QTextEdit *textEdit, const QString &description);

    void removeMysticOrbsFromProperties(const QSet<int> &mysticOrbs, PropertiesMultiMap *props);
    void removeMysticOrbData(int moCode, PropertiesMultiMap *props);
    int indexOfPropertyValue(int id, const PropertiesMultiMap *const props, quint32 param = 0) const;
    void modifyMysticOrbProperty(int id, int decrement, PropertiesMultiMap *props, quint32 param = 0);
    ItemProperty *getProperty(int id, quint32 param, const PropertiesMultiMap *const props) const;
    int totalMysticOrbValue(int moCode, PropertiesMap *props) const;
    void decreaseRequiredLevel(int decrement, PropertiesMultiMap *props);

    void updateItem() { _item->hasChanged = true; showItem(_item); }

    QString collectMysticOrbsDataFromProps(QSet<int> *moSet, PropertiesMap &props);
    bool isMysticOrbEffectDoubled() const;
};

#endif // PROPERTIESVIEWERWIDGET_H
