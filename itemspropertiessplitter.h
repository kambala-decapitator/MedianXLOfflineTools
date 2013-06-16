#ifndef ITEMSPROPERTIESSPLITTER_H
#define ITEMSPROPERTIESSPLITTER_H

#include "structs.h"

#include <QSplitter>


class ItemStorageTableView;
class ItemStorageTableModel;
class PropertiesViewerWidget;

class QModelIndex;
class QMenu;

typedef QMultiMap<quint8, ItemInfo *> UpgradableItemsMultiMap;

class ItemsPropertiesSplitter : public QSplitter
{
    Q_OBJECT

public:
    explicit ItemsPropertiesSplitter(ItemStorageTableView *itemsView, QWidget *parent = 0);
    virtual ~ItemsPropertiesSplitter() {}

    void setModel(ItemStorageTableModel *model);

    PropertiesViewerWidget *propertiesWidget() const { return _propertiesWidget; }
    ItemStorageTableView   *itemsView()        const { return _itemsView; }
    ItemStorageTableModel  *itemsModel()       const { return _itemsModel; }

    virtual void setItems(const ItemsList &newItems);
    virtual void showItem(ItemInfo *item);
    void showFirstItem();
    virtual ItemsList *getItems() { return &_allItems; }

    virtual QPair<bool, bool> updateDisenchantButtonsState(bool includeUniques, bool includeSets, bool toCrystals, ItemsList *pItems = 0);
    virtual QPair<bool, bool> updateUpgradeButtonsState(ItemsList *pItems = 0);

    virtual ItemsList disenchantAllItems(bool toShards, bool upgradeToCrystals, bool eatSignets, ItemsList *pItems = 0);
    virtual void upgradeGems(ItemsList *pItems = 0);
    virtual void upgradeRunes(ItemsList *pItems = 0);

    int itemCount() const { return _allItems.size(); }

signals:
    void itemCountChanged(int itemCount);
    void itemDeleted();
    void cubeDeleted(bool = true);  // connect directly to QAction's setEnabled() slot
    void itemsChanged(bool = true); // connect directly to main window's setModified() slot
    void signetsOfLearningEaten(int signets);

protected slots:
    void itemSelected(const QModelIndex &index, bool display = true);
    void moveItem(const QModelIndex &newIndex, const QModelIndex &oldIndex);

    void showContextMenu(const QPoint &pos);
    void exportText();
    void disenchantSelectedItem();
//    void unsocketItem();
    //void makeNonEthereal();
    void downgradeSelectedRune();
    void eatSelectedSignet();
    void deleteItemTriggered();

protected:
    enum ItemAction
    {
        //ExportBbCode,
        //ExportHtml,
        DisenchantShards,
        DisenchantSignet,
        //Unsocket,
        RemoveMO,
        EatSignetOfLearning,
        Delete
    };

    ItemStorageTableView *_itemsView;
    PropertiesViewerWidget *_propertiesWidget;

    QHash<ItemAction, QAction *> _itemActions;
    ItemStorageTableModel *_itemsModel;
    ItemsList _allItems;

    void updateItems(const ItemsList &newItems);
    void deleteItem(ItemInfo *item);
    void performDeleteItem(ItemInfo *item, bool emitSignal = true);
    ItemInfo *selectedItem(bool showError = true);

    virtual void addItemToList(ItemInfo *item, bool emitSignal = true);
    virtual void removeItemFromList(ItemInfo *item, bool emitSignal = true);
    ItemInfo *disenchantItemIntoItem(ItemInfo *oldItem, ItemInfo *newItem, bool emitSignal = true);
    virtual bool isItemInCurrentStorage(ItemInfo *item) const { Q_UNUSED(item); return true; }
    virtual bool storeItemInStorage(ItemInfo *item, int storage);
    bool upgradeItemsInMap(UpgradableItemsMultiMap &itemsMap, quint8 maxKey, const QString &itemNameFormat);

    bool canSocketableMapBeUpgraded(const UpgradableItemsMultiMap &socketableMap);
    QHash<QByteArray, UpgradableItemsMultiMap> getGemsMapsFromItems(const ItemsList &items);
    UpgradableItemsMultiMap getRunesMapFromItems(const ItemsList &items);

    void createItemActions();
    QAction *separator();
    void createActionsForMysticOrbs(QMenu *parentMenu, bool isItemMO, ItemInfo *item);
};

#endif // ITEMSPROPERTIESSPLITTER_H
