#ifndef ITEMSVIEWERDIALOG_H
#define ITEMSVIEWERDIALOG_H

#include "structs.h"
#include "enums.h"

#include <QWidget>
#include <QAction>


class ItemsPropertiesSplitter;
class PlugyItemsSplitter;

class QTabWidget;
class QCloseEvent;
class QModelIndex;
class QGroupBox;
class QPushButton;
class QCheckBox;
class QRadioButton;
class QSpinBox;

class ItemsViewerDialog : public QDialog
{
    Q_OBJECT

public:
    enum TabIndex
    {
        GearIndex = 0,
        InventoryIndex,
        CubeIndex,
        PersonalStashIndex,
        SigmaSharedStashIndex,
        SigmaHCStashIndex,
        SharedStashIndex,
        HCStashIndex,
        LastIndex = HCStashIndex
    };

    enum ShowDisenchantPreviewOption
    {
        Always = 0,
        OnlyCurrentPage,
        Never
    };

    static const int kCellSize;

    static const QList<int> &kRows();
    static const QList<int>& kColumns();
    static int rowsInStorageAtIndex(int storage);
    static int colsInStorageAtIndex(int storage);
    static int tabIndexFromItemStorage(int storage);
    static const QString &tabNameAtIndex(int i);

    explicit ItemsViewerDialog(const QHash<int, bool> &plugyStashesExistenceHash, quint8 showDisenchantPreviewOption, QWidget *parent = 0);
    virtual ~ItemsViewerDialog() {}

    void updateItems(const QHash<int, bool> &plugyStashesExistenceHash, bool isCreatingTabs);
    void updateBeltItemsCoordinates(bool restore, ItemsList *pBeltItems);
    void updateGearItems(ItemsList *pBeltItems = 0, ItemsList *pEquippedItems = 0, bool isCreatingTabs = false);
    void totalItemsIncreasedBy(int n) { _itemsTotal += n; updateWindowTitle(); }

    void saveSettings();
    bool isPlugyStorageIndex(int index) { return index >= PersonalStashIndex; }

    QTabWidget *tabWidget() { return _tabWidget; }
    ItemsPropertiesSplitter *splitterAtIndex(int tabIndex);
    ItemsPropertiesSplitter *currentSplitter();
    PlugyItemsSplitter *currentPlugySplitter();

public slots:
    virtual void reject();

    void setCubeTabDisabled(bool disabled);
    void showItem(ItemInfo *item);
    void updateItemManagementButtonsState();
    void showDisenchantPreviewActionTriggered(QAction *action) { _showDisenchantPreviewOption = static_cast<ShowDisenchantPreviewOption>(action->data().toUInt()); }

protected:
#ifdef Q_OS_MAC
    virtual void keyPressEvent(QKeyEvent *e);
#endif

signals:
    void cubeDeleted(bool = true);  // connect directly to QAction's setEnabled() slot
    void closing(bool = true);      // connect directly to QAction's setDisabled() slot
    void itemsChanged(bool = true); // connect directly to QMainWindow's setWindowModified() slot
    void signetsOfLearningEaten(int signets);
    void stashSorted();

private slots:
    void tabChanged(int tabIndex);
    void itemCountChangedInCurrentTab(int newCount);
    void decreaseItemCount();

    void applyActionToAllPagesChanged(bool b);
    void updateDisenchantButtonsState();

    void disenchantAllItems();
    void upgradeGems();
    void upgradeRunes();
    void upgradeGemsAndRunes() { upgradeGems(); upgradeRunes(); }
    
    void sortStash();
    void insertBlankPages();
    void removeCurrentPage();
    void moveItemBetweenStashes(ItemInfo *item);
    void moveCurrentItemsToSharedStash();

    void adjustHeight(bool isBoxExpanded);

private:
    QTabWidget *_tabWidget;
    quint64 _itemsTotal;
    QWidget *_itemManagementWidget;
    ShowDisenchantPreviewOption _showDisenchantPreviewOption;

    QGroupBox *_disenchantBox;
    QPushButton *_disenchantToShardsButton, *_disenchantToSignetButton;
    QCheckBox *_upgradeToCrystalsCheckbox, *_eatSignetsCheckbox;
    QRadioButton *_uniquesRadioButton, *_setsRadioButton, *_bothQualitiesRadioButton;

    QGroupBox *_upgradeBox;
    QPushButton *_upgradeGemsButton, *_upgradeRunesButton, *_upgradeBothButton;
    QSpinBox *_reserveRunesSpinBox;

    QCheckBox *_applyActionToAllPagesCheckbox;
    QPushButton *_moveCurrentItemsToSharedStashButton;

    QGroupBox *_stashBox;
    QPushButton *_sortStashButton, *_insertBlankPagesBeforeButton, *_insertBlankPagesAfterButton, *_removeCurrentBlankPage;

    void createLayout();
    void loadSettings();

    void itemCountChangedInTab(int tabIndex, int newCount);
    void updateWindowTitle();

    void updateUpgradeButtonsState();
    void updateStashButtonsState();
    void updateRemoveCurrentBlankPageButtonState();
};

#endif // ITEMSVIEWERDIALOG_H
