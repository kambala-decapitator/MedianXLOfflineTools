#include "itemsviewerdialog.h"
#include "itemspropertiessplitter.h"
#include "plugyitemssplitter.h"
#include "gearitemssplitter.h"
#include "itemstoragetableview.h"
#include "itemstoragetablemodel.h"
#include "itemdatabase.h"
#include "propertiesviewerwidget.h"
#include "itemparser.h"
#include "characterinfo.hpp"

#include <QTabWidget>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QCloseEvent>

#include <QSettings>


const int ItemsViewerDialog::kCellSize = 32;
const QList<int> ItemsViewerDialog::kRows = QList<int>() << 11 << 6 << 8 << 10 << 10 << 10 << 10;

ItemsViewerDialog::ItemsViewerDialog(const QHash<int, bool> &plugyStashesExistenceHash, QWidget *parent) : QDialog(parent), _tabWidget(new QTabWidget(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(_tabWidget);

    for (int i = GearIndex; i <= LastIndex; ++i)
    {
        ItemsPropertiesSplitter *splitter;
        if (i == GearIndex)
            splitter = new GearItemsSplitter(new ItemStorageTableView(this), this);
        else if (i < PersonalStashIndex)
            splitter = new ItemsPropertiesSplitter(new ItemStorageTableView(this), this);
        else
            splitter = new PlugyItemsSplitter(new ItemStorageTableView(this), this);
        splitter->setModel(new ItemStorageTableModel(kRows.at(i), i == GearIndex ? 8 : 10, splitter));
        _tabWidget->addTab(splitter, tabNameAtIndex(i));

        connect(splitter, SIGNAL(itemCountChanged(int)), SLOT(itemCountChangedInCurrentTab(int)));
        connect(splitter, SIGNAL(itemDeleted()), SLOT(decreaseItemCount()));
        connect(splitter, SIGNAL(itemsChanged(bool)), SIGNAL(itemsChanged(bool)));
        connect(splitter, SIGNAL(cubeDeleted(bool)), SIGNAL(cubeDeleted(bool)));
        connect(splitter, SIGNAL(cubeDeleted(bool)), SLOT(setCubeTabDisabled(bool)));
        //connect(splitter, SIGNAL(storageModified(int)), SLOT(storageItemsModified(int)));
    }
    updateItems(plugyStashesExistenceHash);

    for (int i = GearIndex; i <= LastIndex; ++i)
    {
        ItemsPropertiesSplitter *splitter = splitterAtIndex(i);
        QTableView *tableView = static_cast<QTableView *>(splitter->itemsView());
        for (int j = 0; j < splitter->itemsModel()->rowCount(); ++j)
            tableView->setRowHeight(j, kCellSize);
        for (int j = 0; j < splitter->itemsModel()->columnCount(); ++j)
            tableView->setColumnWidth(j, kCellSize);
    }

    connect(_tabWidget, SIGNAL(currentChanged(int)), SLOT(tabChanged(int)));

    loadSettings();
    _tabWidget->setCurrentIndex(0);
}

void ItemsViewerDialog::loadSettings()
{
    QSettings settings;
    restoreGeometry(settings.value("itemsViewerGeometry").toByteArray());

    for (int i = GearIndex; i <= LastIndex; ++i)
        splitterAtIndex(i)->restoreState(settings.value(QString("itemsTab%1_state").arg(i)).toByteArray());
}

ItemsPropertiesSplitter *ItemsViewerDialog::splitterAtIndex(int tabIndex)
{
    return static_cast<ItemsPropertiesSplitter *>(_tabWidget->widget(tabIndex));
}

void ItemsViewerDialog::saveSettings()
{
    QSettings settings;
    settings.setValue("itemsViewerGeometry", saveGeometry());

    for (int i = GearIndex; i <= LastIndex; ++i)
        settings.setValue(QString("itemsTab%1_state").arg(i), splitterAtIndex(i)->saveState());
}

void ItemsViewerDialog::closeEvent(QCloseEvent *event)
{
    saveSettings();
    updateBeltItemsCoordinates(true, 0);
    emit closing();
    event->accept();
}

void ItemsViewerDialog::tabChanged(int tabIndex)
{
    splitterAtIndex(tabIndex)->showFirstItem();
}

void ItemsViewerDialog::itemCountChangedInCurrentTab(int newCount)
{
    itemCountChangedInTab(_tabWidget->currentIndex(), newCount);
}

void ItemsViewerDialog::itemCountChangedInTab(int tabIndex, int newCount)
{
    QString newTabTitle = isPlugyStorageIndex(tabIndex) ? QString(" (%1/%2)").arg(splitterAtIndex(tabIndex)->itemsModel()->itemCount()).arg(newCount) : QString(" (%1)").arg(newCount);
    _tabWidget->setTabText(tabIndex, tabNameAtIndex(tabIndex) + newTabTitle);
}

void ItemsViewerDialog::updateItems(const QHash<int, bool> &plugyStashesExistenceHash)
{
    _itemsTotal = 0;
    for (int i = GearIndex; i <= LastIndex; ++i)
    {
        bool isGearTab = i == GearIndex;
        ItemsList items = ItemDataBase::itemsStoredIn(Enums::ItemStorage::metaEnum().value(i), isGearTab ? Enums::ItemLocation::Equipped : Enums::ItemLocation::Stored);
        if (isGearTab)
        {
            items += ItemDataBase::itemsStoredIn(Enums::ItemStorage::NotInStorage, Enums::ItemLocation::Merc);
            items += ItemDataBase::itemsStoredIn(Enums::ItemStorage::NotInStorage, Enums::ItemLocation::Corpse);

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
                default:
                    qDebug("item->whereEquipped == %d!!!", item->whereEquipped);
                    break;
                }
            }

            ItemsList beltItems = ItemDataBase::itemsStoredIn(Enums::ItemStorage::NotInStorage, Enums::ItemLocation::Belt);
            updateBeltItemsCoordinates(false, &beltItems);
            items += beltItems;
        }
        else // itemCountChanged() signal is sent from GearItemsSplitter when setItems() is called
            itemCountChangedInTab(i, items.size());
        splitterAtIndex(i)->setItems(items);
        
        _itemsTotal += items.size();
    }

    setCubeTabDisabled(!CharacterInfo::instance().items.hasCube());
    for (QHash<int, bool>::const_iterator iter = plugyStashesExistenceHash.constBegin(); iter != plugyStashesExistenceHash.constEnd(); ++iter)
        _tabWidget->setTabEnabled(tabIndexFromItemStorage(iter.key()), iter.value());
    
    updateWindowTitle();
}

int ItemsViewerDialog::tabIndexFromItemStorage(int storage)
{
    return storage > Enums::ItemStorage::Inventory ? storage - 2 : storage;
}

const QString &ItemsViewerDialog::tabNameAtIndex(int i)
{
    // add elements here when adding new tab
    static const QStringList tabNames = QStringList() << tr("Gear") << tr("Inventory") << tr("Cube") << tr("Stash") << tr("Personal Stash") << tr("Shared Stash") << tr("Hardcore Stash");
    return tabNames.at(i);
}

void ItemsViewerDialog::showItem(ItemInfo *item)
{
    _tabWidget->setCurrentIndex(tabIndexFromItemStorage(item->storage));
    splitterAtIndex(_tabWidget->currentIndex())->showItem(item);
}

void ItemsViewerDialog::setCubeTabDisabled(bool disabled)
{
    _tabWidget->setTabEnabled(CubeIndex, !disabled);
}

void ItemsViewerDialog::updateWindowTitle()
{
    setWindowTitle(tr("Items viewer (items total: %1)").arg(_itemsTotal));
}

void ItemsViewerDialog::decreaseItemCount()
{
    --_itemsTotal;
    updateWindowTitle();
}

void ItemsViewerDialog::updateBeltItemsCoordinates(bool restore, ItemsList *pBeltItems)
{
    ItemsList beltItems = pBeltItems ? *pBeltItems : ItemDataBase::itemsStoredIn(Enums::ItemStorage::NotInStorage, Enums::ItemLocation::Belt);
    int lastRowIndex = kRows.at(GearIndex) - 1;
    foreach (ItemInfo *item, beltItems)
    {
        item->row = lastRowIndex - item->row;
        item->column += restore ? -2 : 2;
    }
}

//int ItemsViewerDialog::storageItemsModified(int storage)
//{
//    int tabIndex = tabIndexFromItemStorage(storage);
//}
