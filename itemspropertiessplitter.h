#ifndef ITEMSPROPERTIESSPLITTER_H
#define ITEMSPROPERTIESSPLITTER_H

#include "structs.h"

#include <QSplitter>


class ItemStorageTableView;
class ItemStorageTableModel;
class PropertiesViewerWidget;

class QModelIndex;
class QMenu;

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

    virtual QPair<bool, bool> updateDisenchantButtonsState(bool includeUniques, bool includeSets, ItemsList *items = 0);
    virtual QPair<bool, bool> updateUpgradeButtonsState(ItemsList *items = 0);

    virtual void disenchantAllItems(bool toShards, bool upgradeToCrystals, bool eatSignets, bool includeUniques, bool includeSets, ItemsList *items = 0);

signals:
    void itemCountChanged(int itemCount);
    void itemDeleted();
    void cubeDeleted(bool = true);  // connect directly to QAction's setEnabled() slot
    void itemsChanged(bool = true); // connect directly to main window's setModified() slot

protected slots:
    void itemSelected(const QModelIndex &index);
    void moveItem(const QModelIndex &newIndex, const QModelIndex &oldIndex);

    void showContextMenu(const QPoint &pos);
    void exportText();
    void disenchantSelectedItem();
//    void unsocketItem();
    //void makeNonEthereal();
    void deleteItem();

protected:
    enum ItemAction
    {
        //ExportBbCode,
        //ExportHtml,
        DisenchantShards,
        DisenchantSignet,
        //Unsocket,
        RemoveMO,
        Delete
    };

    ItemStorageTableView *_itemsView;
    PropertiesViewerWidget *_propertiesWidget;

    QHash<ItemAction, QAction *> _itemActions;
    ItemStorageTableModel *_itemsModel;
    ItemsList _allItems;

    void updateItems(const ItemsList &newItems);
    void performDeleteItem(ItemInfo *item, bool currentStorage = true, bool emitSignal = true);
    ItemInfo *selectedItem(bool showError = true);

    virtual void addItemToList(ItemInfo *item, bool currentStorage = true, bool emitSignal = true);
    virtual void removeItemFromList(ItemInfo *item, bool currentStorage = true, bool emitSignal = true);
    ItemInfo *disenchantItemIntoItem(ItemInfo *oldItem, ItemInfo *newItem, bool emitSignal = true);
    virtual bool isItemInCurrentStorage(ItemInfo *item) const { Q_UNUSED(item); return true; }

    void createItemActions();
    QAction *separator();
    void createActionsForMysticOrbs(QMenu *parentMenu, bool isItemMO, ItemInfo *item);
};

#endif // ITEMSPROPERTIESSPLITTER_H
