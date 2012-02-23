#ifndef ITEMSPROPERTIESSPLITTER_H
#define ITEMSPROPERTIESSPLITTER_H

#include "structs.h"

#include <QSplitter>


class ItemStorageTableView;
class ItemStorageTableModel;
class PropertiesViewerWidget;
class QModelIndex;
class QPushButton;
class QDoubleSpinBox;
class QKeySequence;

class ItemsPropertiesSplitter : public QSplitter
{
    Q_OBJECT

public:
    explicit ItemsPropertiesSplitter(ItemStorageTableView *itemsView, bool shouldCreateNavigation, QWidget *parent = 0);

    void setModel(ItemStorageTableModel *model);

    PropertiesViewerWidget *propertiesWidget() const { return _propertiesWidget; }
    ItemStorageTableView *itemsView() const { return _itemsView; }
    ItemStorageTableModel *itemsModel() const { return _itemsModel; }

    void setItems(const ItemsList &newItems);
    void showItem(ItemInfo *item);
    void showFirstItem();

public slots:
    // these 8 are connected to main menu actions
    void previous10Pages() { left10Clicked();  }
    void previousPage()    { leftClicked();    }
    void nextPage()        { rightClicked();   }
    void next10Pages()     { right10Clicked(); }

    // emulating pressed shift if action was pressed by mouse
//    void previous100Pages() { _isShiftPressed = true; left10Clicked(); _isShiftPressed = false; }
//    void firstPage() { _isShiftPressed = true; leftClicked(); _isShiftPressed = false; }
//    void lastPage() { _isShiftPressed = true; rightClicked(); _isShiftPressed = false; }
//    void next100Pages() { _isShiftPressed = true; right10Clicked(); _isShiftPressed = false; }
    void previous100Pages() { emulateShiftAndInvokeMethod(&ItemsPropertiesSplitter::left10Clicked);  }
    void firstPage()        { emulateShiftAndInvokeMethod(&ItemsPropertiesSplitter::leftClicked);    }
    void lastPage()         { emulateShiftAndInvokeMethod(&ItemsPropertiesSplitter::rightClicked);   }
    void next100Pages()     { emulateShiftAndInvokeMethod(&ItemsPropertiesSplitter::right10Clicked); }

protected:
    void keyPressEvent(QKeyEvent *keyEvent);
    void keyReleaseEvent(QKeyEvent *keyEvent);

signals:
    void itemCountChanged(int itemCount);
    void itemDeleted();
    void cubeDeleted(bool deleted = true); // param is here because I want to connect this signal directly to QAction's setEnabled() slot
    //void storageModified(int storage);

private slots:
    void itemSelected(const QModelIndex &index);

    void updateItemsForCurrentPage(bool pageChanged = true);
    void leftClicked();
    void rightClicked();
    void left10Clicked();
    void right10Clicked();

    void showContextMenu(const QPoint &pos);
    void exportHtml();
    void exportBbCode();
    void disenchantItem();
    void unsocketItem();
    //void makeNonEthereal();
    void deleteItem();

private:
    PropertiesViewerWidget *_propertiesWidget;
    ItemStorageTableView *_itemsView;
    ItemStorageTableModel *_itemsModel;
    QPushButton *_left10Button, *_leftButton, *_rightButton, *_right10Button;
    QDoubleSpinBox *_pageSpinBox;

    ItemsList _allItems;
    quint32 _lastNotEmptyPage;

    bool _isShiftPressed;

    void emulateShiftAndInvokeMethod(void (ItemsPropertiesSplitter::*method)(void)) { _isShiftPressed = true; (this->*method)(); _isShiftPressed = false; }
    bool keyEventHasShift(QKeyEvent *keyEvent);
    void setShortcutTextInButtonTooltip(QPushButton *button, const QKeySequence &keySequence);

    void updateItems(const ItemsList &newItems);
    void performDeleteItem(ItemInfo *item, bool currentStorage = true);
    ItemInfo *selectedItem(bool showError = true);

    void addItemToList(ItemInfo *item, bool currentStorage = true);
    void removeItemFromList(ItemInfo *item, bool currentStorage = true);
};

#endif // ITEMSPROPERTIESSPLITTER_H
