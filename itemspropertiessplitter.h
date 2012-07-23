#ifndef ITEMSPROPERTIESSPLITTER_H
#define ITEMSPROPERTIESSPLITTER_H

#include "structs.h"

#include <QSplitter>


class ItemStorageTableView;
class ItemStorageTableModel;
class PropertiesViewerWidget;

class QModelIndex;
class QMenu;
class QGroupBox;
class QPushButton;
class QCheckBox;

class ItemsPropertiesSplitter : public QSplitter
{
    Q_OBJECT

public:
    explicit ItemsPropertiesSplitter(ItemStorageTableView *itemsView, QWidget *parent = 0, bool createChildren = true);
    virtual ~ItemsPropertiesSplitter() {}

    void setModel(ItemStorageTableModel *model);

    PropertiesViewerWidget *propertiesWidget() const { return _propertiesWidget; }
    ItemStorageTableView   *itemsView()        const { return _itemsView; }
    ItemStorageTableModel  *itemsModel()       const { return _itemsModel; }

    virtual void setItems(const ItemsList &newItems);
    virtual void showItem(ItemInfo *item);
    void showFirstItem();

signals:
    void itemCountChanged(int itemCount);
    void itemDeleted();
    void cubeDeleted(bool = true);  // connect directly to QAction's setEnabled() slot
    //void storageModified(int storage);
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

    void disenchantAllItems();
    void disenchantItemIntoItem(ItemInfo *oldItem, ItemInfo *newItem);

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

    QGroupBox *_disenchantBox;
    QPushButton *_disenchantToShardsButton, *_disenchantToSignetButton;
    QCheckBox *_upgradeToCrystalsCheckbox, *_eatSignetsCheckbox;

    QGroupBox *_upgradeBox;
    QPushButton *_upgradeGemsButton, *_upgradeRunesButton, *_upgradeBothButton;

    QHash<ItemAction, QAction *> _itemActions;
    ItemStorageTableModel *_itemsModel;
    ItemsList _allItems;

    void updateButtonsState(ItemsList *items = 0);
    void updateDisenchantButtonsState(ItemsList *items = 0);
    void updateUpgradeButtonsState(ItemsList *items = 0);

    void updateItems(const ItemsList &newItems);
    void performDeleteItem(ItemInfo *item, bool currentStorage = true);
    ItemInfo *selectedItem(bool showError = true);

    void addItemToList(ItemInfo *item, bool currentStorage = true);
    void removeItemFromList(ItemInfo *item, bool currentStorage = true);

    void createItemActions();
    QAction *separator();
    void createActionsForMysticOrbs(QMenu *parentMenu, bool isItemMO, ItemInfo *item);
};

#endif // ITEMSPROPERTIESSPLITTER_H
