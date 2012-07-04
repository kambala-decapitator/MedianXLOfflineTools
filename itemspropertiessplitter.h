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

signals:
    void itemCountChanged(int itemCount);
    void itemDeleted();
    void cubeDeleted(bool = true);  // connect directly to QAction's setEnabled() slot
    //void storageModified(int storage);
    void itemsChanged(bool = true); // connect directly to main window's setModified() slot

private slots:
    void itemSelected(const QModelIndex &index);
    void moveItem(const QModelIndex &newIndex, const QModelIndex &oldIndex);

    void showContextMenu(const QPoint &pos);
    void exportText();
    void disenchantItem();
    void unsocketItem();
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

    PropertiesViewerWidget *_propertiesWidget;
    ItemStorageTableView *_itemsView;
    ItemStorageTableModel *_itemsModel;
    QHash<ItemAction, QAction *> _itemActions;

    ItemsList _allItems;

    void updateItems(const ItemsList &newItems);
    void performDeleteItem(ItemInfo *item, bool currentStorage = true);
    ItemInfo *selectedItem(bool showError = true);
    void setCellSpanForItem(ItemInfo *item);

    void addItemToList(ItemInfo *item, bool currentStorage = true);
    void removeItemFromList(ItemInfo *item, bool currentStorage = true);

    void createItemActions();
    QAction *separator();
    void createActionsForMysticOrbs(QMenu *parentMenu, bool isItemMO, ItemInfo *item);
};

#endif // ITEMSPROPERTIESSPLITTER_H
