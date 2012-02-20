#ifndef ITEMSVIEWERWIDGET_H
#define ITEMSVIEWERWIDGET_H

#include "structs.h"
#include "enums.h"

#include <QWidget>


class ItemsPropertiesSplitter;
class QTabWidget;
class QCloseEvent;
class QModelIndex;

class ItemsViewerDialog : public QDialog
{
    Q_OBJECT

public:
    enum TabIndex
    {
        GearIndex = 0,
        InventoryIndex,
        CubeIndex,
        StashIndex,
        PersonalStashIndex,
        SharedStashIndex,
        HCStashIndex,
        LastIndex = HCStashIndex
    };

    static const int cellSize;
    static const QStringList tabNames;
    static const QList<int> rows;

    static int tabIndexFromItemStorage(int storage);

    explicit ItemsViewerDialog(const QHash<int, bool> &plugyStashesExistenceHash, QWidget *parent = 0);

    void updateItems(const QHash<int, bool> &plugyStashesExistenceHash);
    void showItem(ItemInfo *item);

    //void enableCubeTab();

    void saveSettings();

    bool isPlugyStorageIndex(int index) { return index >= PersonalStashIndex && index <= HCStashIndex; }
    QTabWidget *tabWidget() { return _tabWidget; }
    ItemsPropertiesSplitter *splitterAtIndex(int tabIndex);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void tabChanged(int tabIndex);
    void itemCountChangedInCurrentTab(int newCount);
    void decreaseItemCount();
    //int storageItemsModified(int storage);

private:
    QTabWidget *_tabWidget;
    quint64 _itemsTotal;

    void loadSettings();

    void itemCountChangedInTab(int tabIndex, int newCount);
    void updateWindowTitle();
};

#endif // ITEMSVIEWERWIDGET_H
