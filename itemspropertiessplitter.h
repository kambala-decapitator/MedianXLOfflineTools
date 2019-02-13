#ifndef ITEMSPROPERTIESSPLITTER_H
#define ITEMSPROPERTIESSPLITTER_H

#include "structs.h"

#include <QSplitter>

#ifndef QT_NO_DEBUG
#define DUMP_INFO_ACTION
#endif


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
    virtual bool storeItemInStorage(ItemInfo *item, int storage, bool emitSignal = false);
    void setCellSpanForItem(ItemInfo *item);

    virtual QPair<bool, bool> updateDisenchantButtonsState(bool includeUniques, bool includeSets, bool toCrystals, ItemsList *pItems = 0);
    virtual QPair<bool, bool> updateUpgradeButtonsState(int reserveRunes, ItemsList *pItems = 0);

    virtual ItemsList disenchantAllItems(bool toShards, bool upgradeToCrystals, bool eatSignets, ItemsList *pItems = 0);
    virtual void upgradeGems(ItemsList *pItems = 0);
    virtual void upgradeRunes(int reserveRunes, ItemsList *pItems = 0);

    int itemCount() const { return _allItems.size(); }

signals:
    void itemCountChanged(int itemCount);
    void itemDeleted();
    void cubeDeleted(bool = true);  // connect directly to QAction's setEnabled() slot
    void itemsChanged(bool = true); // connect directly to main window's setModified() slot
    void signetsOfLearningEaten(int signets);
    void itemMovingBetweenStashes(ItemInfo *);

protected slots:
    void itemSelected(const QModelIndex &index, bool display = true);
    void moveItem(const QModelIndex &newIndex, const QModelIndex &oldIndex);

    void showContextMenu(const QPoint &pos);
    void moveBetweenStashes();
    void exportText();
    void disenchantSelectedItem();
//    void unsocketItem();
    void downgradeSelectedRune();
    void eatSelectedSignet();
    void collectShrinesToVessel();
    void extractShrinesFromVessel();
    void depersonalize();
    void   personalize();
    void deleteItemTriggered();
#ifdef DUMP_INFO_ACTION
    void dumpInfo(ItemInfo *item = 0, bool shouldShowMsgBox = true);
#endif // DUMP_INFO_ACTION


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
    void removeItemFromModel(ItemInfo *item);
    ItemInfo *disenchantItemIntoItem(ItemInfo *oldItem, ItemInfo *newItem, bool emitSignal = true);
    virtual bool isItemInCurrentStorage(ItemInfo *item) const { Q_UNUSED(item); return true; }
    bool upgradeItemsInMap(UpgradableItemsMultiMap &itemsMap, quint8 maxKey, const QString &itemNameFormat);

    bool canSocketableMapBeUpgraded(const UpgradableItemsMultiMap &socketableMap);
    QHash<QByteArray, UpgradableItemsMultiMap> gemsMapsFromItems(const ItemsList &items);
    UpgradableItemsMultiMap runesMapFromItems(const ItemsList &items, int reserveRunes);

    void createItemActions();
    QAction *separatorAction();
    void createActionsForMysticOrbs(QMenu *parentMenu, bool isItemMO, ItemInfo *item);

    virtual bool shouldAddMoveItemAction() const;
    virtual QString moveItemActionText() const;
};

#endif // ITEMSPROPERTIESSPLITTER_H
