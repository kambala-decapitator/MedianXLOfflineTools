#ifndef PLUGYITEMSSPLITTER_H
#define PLUGYITEMSSPLITTER_H

#include "itemspropertiessplitter.h"


class QDoubleSpinBox;
class QKeySequence;

class QFile;

class PlugyItemsSplitter : public ItemsPropertiesSplitter
{
    Q_OBJECT

public:
    explicit PlugyItemsSplitter(ItemStorageTableView *itemsView, QWidget *parent = 0);
    virtual ~PlugyItemsSplitter() {}

    virtual void setItems(const ItemsList &newItems);
    void updateSpinbox() { setItems(_allItems); }
    virtual void showItem(ItemInfo *item);
    virtual ItemsList *getItems() { return allOrCurrentPageItems(); }

    virtual QPair<bool, bool> updateDisenchantButtonsState(bool includeUniques, bool includeSets, bool toCrystals, ItemsList *items = 0);
    virtual QPair<bool, bool> updateUpgradeButtonsState(int reserveRunes, ItemsList *items = 0);

    virtual ItemsList disenchantAllItems(bool toShards, bool upgradeToCrystals, bool eatSignets, ItemsList *pItems = 0);
    virtual void upgradeGems(ItemsList *items = 0);
    virtual void upgradeRunes(int reserveRunes, ItemsList *items = 0);

    void sortStash(const StashSortOptions &sortOptions);
    void insertBlankPages(int pages, bool isAfter);
    void removeCurrentPage();

    quint32 lastNotEmptyPage() const { return _lastNotEmptyPage; }
    quint32 currentPage() const;
    int pageItemCount() const { return _pagedItems.size(); }

public slots:
    // these 8 are connected to main menu actions
    void previous10Pages() { left10Clicked();  }
    void previousPage()    { leftClicked();    }
    void nextPage()        { rightClicked();   }
    void next10Pages()     { right10Clicked(); }

    // emulating pressed shift if action was pressed by mouse
    void previous100Pages() { emulateShiftAndInvokeMethod(&PlugyItemsSplitter::left10Clicked);  }
    void firstPage()        { emulateShiftAndInvokeMethod(&PlugyItemsSplitter::leftClicked);    }
    void lastPage()         { emulateShiftAndInvokeMethod(&PlugyItemsSplitter::rightClicked);   }
    void next100Pages()     { emulateShiftAndInvokeMethod(&PlugyItemsSplitter::right10Clicked); }

    void setApplyActionToAllPages(bool b) { _shouldApplyActionToAllPages = b; }

protected:
    virtual void keyPressEvent(QKeyEvent *keyEvent);
    virtual void keyReleaseEvent(QKeyEvent *keyEvent);

    virtual bool isItemInCurrentStorage(ItemInfo *item) const;
    virtual void addItemToList(ItemInfo *item, bool emitSignal = true);
    virtual void removeItemFromList(ItemInfo *item, bool emitSignal = true);
    virtual bool storeItemInStorage(ItemInfo *item, int storage);

signals:
    void pageChanged();
    void stashSorted();

private slots:
    void updateItemsForCurrentPage(bool pageChanged_ = true);
    void leftClicked();
    void rightClicked();
    void left10Clicked();
    void right10Clicked();

private:
    QPushButton *_left10Button, *_leftButton, *_rightButton, *_right10Button;
    QDoubleSpinBox *_pageSpinBox;

    quint32 _lastNotEmptyPage;
    bool _isShiftPressed;
    ItemsList _pagedItems;
    bool _shouldApplyActionToAllPages;
    quint8 _maxItemHeightInRow;

    void emulateShiftAndInvokeMethod(void (PlugyItemsSplitter::*method)(void)) { _isShiftPressed = true; (this->*method)(); _isShiftPressed = false; }
    bool keyEventHasShift(QKeyEvent *keyEvent);
    void setShortcutTextInButtonTooltip(QPushButton *button, const QKeySequence &keySequence);
    ItemsList *allOrCurrentPageItems() { return _shouldApplyActionToAllPages ? &_allItems : &_pagedItems; }
//    void moveItemsToFirstPages(ItemsList *items, bool toShards);

    void showErrorLoadingSortingOrderFile(const QFile &f);
    void sortWearableItems(       ItemsList &selectedItems, quint32 &page, const StashSortOptions &sortOptions, const QList<QByteArray> &gearBaseTypesOrder, const QList<QList<int> > &setsOrder);
    void sortWearableQualityItems(ItemsList &selectedItems, quint32 &page, const StashSortOptions &sortOptions, const QList<QByteArray> &gearBaseTypesOrder, QHash<QByteArray, ItemsList> &itemsByBaseType, bool isSacredOnly = true);
    void sortMiscItems(           ItemsList &selectedItems, quint32 &page, const StashSortOptions &sortOptions, const QList<QByteArray> &miscBaseTypesOrder, const QList<QByteArray> &thngTypesOrder);
    void storeItemsOnPage(const ItemsList &items, bool shouldStartAnotherTypeFromNewRow, quint32 &page, int *pRow = 0, int *pCol = 0, bool shouldStartAnotherCotwFromNewRow = false);

    QHash<QByteArray, ItemsList> itemsSortedByBaseType(const ItemsList &items);
    template<typename K>
    QMap<K, ItemsList> itemsSortedByType(const ItemsList &items);
};

#endif // PLUGYITEMSSPLITTER_H
