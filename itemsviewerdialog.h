#ifndef ITEMSVIEWERWIDGET_H
#define ITEMSVIEWERWIDGET_H

#include "structs.h"
#include "enums.h"

#include <QWidget>


class ItemsPropertiesSplitter;

class QTabWidget;
class QCloseEvent;
class QModelIndex;
class QGroupBox;
class QPushButton;
class QCheckBox;
class QRadioButton;

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

    static const int kCellSize;

    static QList<int> &kRows(); // TODO: [0.4] move to some other place
    static int rowsInStorageAtIndex(int storage);
    static int tabIndexFromItemStorage(int storage);
    static const QString &tabNameAtIndex(int i);

    explicit ItemsViewerDialog(const QHash<int, bool> &plugyStashesExistenceHash, QWidget *parent = 0);
    virtual ~ItemsViewerDialog() {}

    void updateItems(const QHash<int, bool> &plugyStashesExistenceHash);
    void showItem(ItemInfo *item);

    void saveSettings();
    bool isPlugyStorageIndex(int index) { return index >= PersonalStashIndex && index <= HCStashIndex; }

    QTabWidget *tabWidget() { return _tabWidget; }
    ItemsPropertiesSplitter *splitterAtIndex(int tabIndex);
    ItemsPropertiesSplitter *currentSplitter();

public slots:
    void setCubeTabDisabled(bool disabled);
    virtual void reject();
    void updateButtonsState();

signals:
    void cubeDeleted(bool = true);  // connect directly to QAction's setEnabled() slot
    void closing(bool = true);      // connect directly to QAction's setDisabled() slot
    void itemsChanged(bool = true); // connect directly to QMainWindow's setWindowModified() slot
    void signetsOfLearningEaten(int signets);

private slots:
    void tabChanged(int tabIndex);
    void itemCountChangedInCurrentTab(int newCount);
    void decreaseItemCount();

    void applyActionToAllPagesChanged(bool b);
    void updateDisenchantButtonsState();
    void updateUpgradeButtonsState();

    void disenchantAllItems();
    void upgradeGems();
    void upgradeRunes();
    void upgradeGemsAndRunes() { upgradeGems(); upgradeRunes(); }

private:
    QTabWidget *_tabWidget;
    quint64 _itemsTotal;
    QWidget *_itemManagementWidget;

    QGroupBox *_disenchantBox;
    QPushButton *_disenchantToShardsButton, *_disenchantToSignetButton;
    QCheckBox *_upgradeToCrystalsCheckbox, *_eatSignetsCheckbox;
    QRadioButton *_uniquesRadioButton, *_setsRadioButton, *_bothQualitiesRadioButton;

    QGroupBox *_upgradeBox;
    QPushButton *_upgradeGemsButton, *_upgradeRunesButton, *_upgradeBothButton;

    QCheckBox *_applyActionToAllPagesCheckbox;

    void loadSettings();

    void itemCountChangedInTab(int tabIndex, int newCount);
    void updateWindowTitle();
    void updateBeltItemsCoordinates(bool restore, ItemsList *pBeltItems);
};

#endif // ITEMSVIEWERWIDGET_H
