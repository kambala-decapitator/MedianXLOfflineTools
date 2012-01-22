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

class ItemsPropertiesSplitter : public QSplitter
{
    Q_OBJECT

public:
    explicit ItemsPropertiesSplitter(ItemStorageTableView *itemsView, ItemStorageTableModel *itemsModel, bool shouldCreateNavigation, QWidget *parent);

    PropertiesViewerWidget *propertiesWidget() const { return _propertiesWidget; }
    ItemStorageTableView *itemsView() const { return _itemsView; }
    ItemStorageTableModel *itemsModel() const { return _itemsModel; }

    void setItems(const ItemsList &newItems);
	void showItem(ItemInfo *item);

protected:
    void keyPressEvent(QKeyEvent *keyEvent);
    void keyReleaseEvent(QKeyEvent *keyEvent);

private slots:
	void itemSelected(const QModelIndex &index);

    void updateItemsForCurrentPage();
    void left10Clicked();
    void leftClicked();
    void rightClicked();
    void right10Clicked();

	void showContextMenu(const QPoint &pos);
	void disenchantItem();
	void unsocketItem();
	void makeNonEthereal();
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

    void updateItems(const ItemsList &newItems);
	ItemInfo *itemFromCoordinate(const QPoint &pos);
	void performDeleteItem(ItemInfo *item);
	QAction *actionFromSender(QObject *sender, const QLatin1String &errorActionText);
	ItemInfo *itemFromAction(QAction *action);

	void addItemToList(ItemInfo *item, int pos = -1);
	void removeItemFromList(ItemInfo *item);
};

#endif // ITEMSPROPERTIESSPLITTER_H
