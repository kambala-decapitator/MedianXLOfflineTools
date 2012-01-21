#include "itemsviewerdialog.h"
#include "itemspropertiessplitter.h"
#include "itemstoragetableview.h"
#include "itemstoragetablemodel.h"
#include "itemdatabase.h"
#include "propertiesviewerwidget.h"

#include <QTabWidget>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QCloseEvent>

#include <QSettings>


const int ItemsViewerDialog::cellSize = 36;
// !!!: add elements here when adding new tab
const QStringList ItemsViewerDialog::tabNames = QStringList() << tr("Gear") << tr("Inventory") << tr("Cube") << tr("Stash") << tr("Personal Stash") << tr("Shared Stash") << tr("Hardcore Stash");
const QList<int> ItemsViewerDialog::rows = QList<int>() << 11 << 6 << 8 << 10 << 10 << 10 << 10;

ItemsViewerDialog::ItemsViewerDialog(QWidget *parent) : QDialog(parent), _tabWidget(new QTabWidget(this))
{
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Items viewer"));

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(_tabWidget);

    for (int i = GearIndex; i <= LastIndex; ++i)
        _tabWidget->addTab(new ItemsPropertiesSplitter(new ItemStorageTableView(this), new ItemStorageTableModel(rows.at(i), this), i >= PersonalStashIndex, this), tabNames.at(i));
    updateItems();

    for (int i = GearIndex; i <= LastIndex; ++i)
    {
        ItemsPropertiesSplitter *splitter = static_cast<ItemsPropertiesSplitter *>(_tabWidget->widget(i));
        QTableView *tableView = static_cast<QTableView *>(splitter->itemsView());
        for (int j = 0; j < splitter->itemsModel()->rowCount(); ++j)
        {
            tableView->setRowHeight(j, 50); // TODO: use cellSize
        }
        for (int j = 0; j < splitter->itemsModel()->columnCount(); ++j)
        {
            tableView->setColumnWidth(j, 100); // TODO: use cellSize
        }
    }

    restoreGeometry(QSettings().value("itemsViewerGeometry").toByteArray());
}

void ItemsViewerDialog::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    settings.setValue("itemsViewerGeometry", saveGeometry());
    event->accept();
}

void ItemsViewerDialog::updateItems()
{
    for (int i = GearIndex; i <= LastIndex; ++i)
    {
        bool isGear = i == GearIndex;
        ItemsList items = itemsLocatedAt(Enums::ItemStorage::metaEnum().value(i), isGear);
        if (isGear)
        {
            foreach (ItemInfo *item, items)
            {
                switch (item->whereEquipped)
                {
                case 1: // helm
                    item->row = 0;
                    item->column = 3;
                    break;
                case 2: // amulet
                    item->row = 1;
                    item->column = 5;
                    break;
                case 3: // armor
                    item->row = 2;
                    item->column = 3;
                    break;
                case 4: // right hand (usually weapon)
                    item->row = 0;
                    item->column = 0;
                    break;
                case 5: // left hand (usually shield)
                    item->row = 0;
                    item->column = 6;
                    break;
                case 6: // right ring
                    item->row = 5;
                    item->column = 2;
                    break;
                case 7: // left ring
                    item->row = 5;
                    item->column = 5;
                    break;
                case 8: // belt
                    item->row = 5;
                    item->column = 3;
                    break;
                case 9: // boots
                    item->row = 4;
                    item->column = 6;
                    break;
                case 10: // gloves
                    item->row = 4;
                    item->column = 0;
                    break;
                case 11: // alt. right hand (usually weapon)
                    item->row = 7;
                    item->column = 0;
                    break;
                case 12: // alt. left hand (usually shield)
                    item->row = 7;
                    item->column = 6;
                    break;
                }
            }
        }
        static_cast<ItemsPropertiesSplitter *>(_tabWidget->widget(i))->setItems(items);
        _tabWidget->setTabEnabled(i, items.size() > 0);
    }
}

ItemsList ItemsViewerDialog::itemsLocatedAt(int storage, bool location /*= Enums::ItemLocation::Stored*/)
{
    ItemsList items, *characterItems = ItemDataBase::currentCharacterItems;
    for (int i = 0; i < characterItems->size(); ++i)
    {
        ItemInfo *item = characterItems->at(i);
        if (item->location == location && item->storage == storage)
            items += item;
    }
    return items;
}

int ItemsViewerDialog::indexFromItemStorage(int storage)
{
	return storage > Enums::ItemStorage::Inventory ? storage - 2 : storage;
}

void ItemsViewerDialog::showItem(ItemInfo *item)
{
	_tabWidget->setCurrentIndex(indexFromItemStorage(item->storage));
	static_cast<ItemsPropertiesSplitter *>(_tabWidget->currentWidget())->showItem(item);
}
