#include "plugyitemssplitter.h"
#include "itemstoragetableview.h"
#include "itemdatabase.h"
#include "itemsviewerdialog.h"
#include "progressbarmodal.hpp"
#include "itemstoragetablemodel.h"
#include "resourcepathmanager.hpp"
#include "itemparser.h"
#include "reversebitwriter.h"
#include "characterinfo.hpp"

#include <QPushButton>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QApplication>

#include <QFile>

//#include <QtConcurrentRun>
//#include <QFuture>
//#include <QFutureWatcher>
//#include <QtConcurrentMap>

#include <qmath.h>

#include <limits>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


static const QString kIconPathFormat(":/PlugyArrows/icons/plugy/%1.png");
static const QByteArray kThngBaseType("thng");

enum SortItemQuality
{
    Quest,
    LowQuality,
    Normal,
    Superior,
    RW,
    Magic,
    Honorific,
    Rare,
    Crafted,
    Unique,
    Set
};

const QHash<int, int> &itemQualityMapping()
{
    static QHash<int, int> m;
    if (m.isEmpty())
    {
        m[Enums::ItemQuality::LowQuality]  = LowQuality;
        m[Enums::ItemQuality::Normal]      = Normal;
        m[Enums::ItemQuality::HighQuality] = Superior;
        m[Enums::ItemQuality::Magic]       = Magic;
        m[Enums::ItemQuality::Honorific]   = Honorific;
        m[Enums::ItemQuality::Rare]        = Rare;
        m[Enums::ItemQuality::Crafted]     = Crafted;
        m[Enums::ItemQuality::Unique]      = Unique;
        m[Enums::ItemQuality::Set]         = Set;
    }
    return m;
}

const QHash<QByteArray, QByteArray> &gemTypesOrderMapping()
{
    static QHash<QByteArray, QByteArray> m;
    if (m.isEmpty())
    {
        // cLOD
        m["gemr"] = "00"; // ruby
        m["gems"] = "01"; // sapphire
        m["geme"] = "02"; // emerald
        m["gemt"] = "03"; // topaz
        m["gemd"] = "04"; // diamond
        m["gema"] = "05"; // amethyst
        m["gemz"] = "06"; // skull
        // MXL
        m["gemp"] = "10"; // bloodstone
        m["gemq"] = "11"; // onyx
        m["gemo"] = "12"; // turquoise
        m["geml"] = "13"; // amber
        m["gemx"] = "14"; // rainbow
    }
    return m;
}


PlugyItemsSplitter::PlugyItemsSplitter(ItemStorageTableView *itemsView, QWidget *parent) : ItemsPropertiesSplitter(itemsView, parent), _shouldApplyActionToAllPages(true), _maxItemHeightInRow(0)
{
    _left10Button = new QPushButton(this);
    _leftButton = new QPushButton(this);
    _rightButton = new QPushButton(this);
    _right10Button = new QPushButton(this);

    QList<QPushButton *> buttons = QList<QPushButton *>() << _left10Button << _leftButton << _rightButton << _right10Button;
    foreach (QPushButton *button, buttons)
        button->setIconSize(QSize(32, 20));
    // hacky way to set button icons
    QKeyEvent keyEvent(QEvent::KeyRelease, Qt::Key_Shift, 0);
    keyReleaseEvent(&keyEvent);

    _pageSpinBox = new QDoubleSpinBox(this);
    _pageSpinBox->setDecimals(0);
    _pageSpinBox->setPrefix(tr("Page #"));
    _pageSpinBox->setRange(1, std::numeric_limits<quint32>::max());
    _pageSpinBox->setValue(1);
    _pageSpinBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QWidget *w = new QWidget(this);
    insertWidget(0, w);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(_left10Button);
    hlayout->addWidget(_leftButton);
    hlayout->addWidget(_pageSpinBox);
    hlayout->addWidget(_rightButton);
    hlayout->addWidget(_right10Button);
    // glue everything together (used mainly for Mac OS X)
    hlayout->setSpacing(0);
    hlayout->setContentsMargins(QMargins());

    QVBoxLayout *vlayout = new QVBoxLayout(w);
    vlayout->addWidget(_itemsView);
    vlayout->addLayout(hlayout);
    vlayout->setSpacing(0);
    vlayout->setContentsMargins(QMargins());

    connect(_pageSpinBox, SIGNAL(valueChanged(double)), SLOT(updateItemsForCurrentPage()));

    connect(_left10Button,  SIGNAL(clicked()), SLOT(left10Clicked()));
    connect(_leftButton,    SIGNAL(clicked()), SLOT(leftClicked()));
    connect(_rightButton,   SIGNAL(clicked()), SLOT(rightClicked()));
    connect(_right10Button, SIGNAL(clicked()), SLOT(right10Clicked()));

    connect(this, SIGNAL(stashSorted()), SIGNAL(itemsChanged()));
}

quint32 PlugyItemsSplitter::currentPage() const
{
    return static_cast<quint32>(_pageSpinBox->value());
}

void PlugyItemsSplitter::keyPressEvent(QKeyEvent *keyEvent)
{
    if (keyEventHasShift(keyEvent))
    {
        _left10Button->setIcon(QIcon(kIconPathFormat.arg("left100")));
        setShortcutTextInButtonTooltip(_left10Button, Qt::ALT + Qt::SHIFT + Qt::Key_Left);

        _leftButton->setIcon(QIcon(kIconPathFormat.arg("first")));
        setShortcutTextInButtonTooltip(_leftButton, Qt::CTRL + Qt::SHIFT + Qt::Key_Left);

        _rightButton->setIcon(QIcon(kIconPathFormat.arg("last")));
        setShortcutTextInButtonTooltip(_rightButton, Qt::CTRL + Qt::SHIFT + Qt::Key_Right);

        _right10Button->setIcon(QIcon(kIconPathFormat.arg("right100")));
        setShortcutTextInButtonTooltip(_right10Button, Qt::ALT + Qt::SHIFT + Qt::Key_Right);

        _isShiftPressed = true;
    }
    ItemsPropertiesSplitter::keyPressEvent(keyEvent);
}

void PlugyItemsSplitter::keyReleaseEvent(QKeyEvent *keyEvent)
{
    if (keyEventHasShift(keyEvent))
    {
        _left10Button->setIcon(QIcon(kIconPathFormat.arg("left10")));
        setShortcutTextInButtonTooltip(_left10Button, Qt::ALT + Qt::Key_Left);

        _leftButton->setIcon(QIcon(kIconPathFormat.arg("left")));
        setShortcutTextInButtonTooltip(_leftButton, Qt::CTRL + Qt::Key_Left);

        _rightButton->setIcon(QIcon(kIconPathFormat.arg("right")));
        setShortcutTextInButtonTooltip(_rightButton, Qt::CTRL + Qt::Key_Right);

        _right10Button->setIcon(QIcon(kIconPathFormat.arg("right10")));
        setShortcutTextInButtonTooltip(_right10Button, Qt::ALT + Qt::Key_Right);

        _isShiftPressed = false;
    }
    ItemsPropertiesSplitter::keyPressEvent(keyEvent);
}

bool PlugyItemsSplitter::isItemInCurrentStorage(ItemInfo *item) const
{
    return item->plugyPage == currentPage();
}

void PlugyItemsSplitter::addItemToList(ItemInfo *item, bool emitSignal /*= true*/)
{
    ItemsPropertiesSplitter::addItemToList(item, emitSignal);
    if (isItemInCurrentStorage(item) && !_pagedItems.contains(item))
        _pagedItems.append(item);
}

void PlugyItemsSplitter::removeItemFromModel(ItemInfo *item)
{
    ItemsPropertiesSplitter::removeItemFromModel(item);
    _pagedItems.removeOne(item);
    _allItems.removeOne(item);
}

bool PlugyItemsSplitter::shouldAddMoveItemAction() const
{
    bool isHcChar = CharacterInfo::instance().basicInfo.isHardcore;
    return (isHcChar && isHcStash) || (!isHcChar && !isHcStash);
}

QString PlugyItemsSplitter::moveItemActionText() const
{
    return tr("Move to character");
}

bool PlugyItemsSplitter::storeItemInStorage(ItemInfo *item, int storage, bool emitSignal /*= false*/)
{
    Enums::ItemStorage::ItemStorageEnum storage_ = static_cast<Enums::ItemStorage::ItemStorageEnum>(storage);
    int rows = ItemsViewerDialog::rowsInStorageAtIndex(storage), cols = ItemsViewerDialog::colsInStorageAtIndex(storage);
    for (quint32 i = 1; i <= _lastNotEmptyPage + 1; ++i)
    {
        if (ItemDataBase::storeItemIn(item, storage_, rows, cols, i))
        {
            if (i > _lastNotEmptyPage)
                setNewLastPage(_lastNotEmptyPage + 1);
            addItemToList(item, emitSignal);
            return true;
        }
    }

    return false;
}

void PlugyItemsSplitter::addItemsToLastPage(const ItemsList &items, Enums::ItemStorage::ItemStorageEnum storage)
{
    if (items.isEmpty())
        return;

    bool wasStashEmpty = _lastNotEmptyPage == 0;
    setNewLastPage(_lastNotEmptyPage + 1);
    foreach (ItemInfo *item, items)
    {
        item->storage = storage;
        item->plugyPage = _lastNotEmptyPage;
        item->hasChanged = true;
        addItemToList(item, false);

        if (wasStashEmpty)
            setCellSpanForItem(item);
    }
    emit itemsChanged();
}

//void PlugyItemsSplitter::moveItemsToFirstPages(ItemsList *items, bool toShards)
//{
//    foreach (ItemInfo *item, *items)
//        if ((toShards && isArcaneShard(item)) || (!toShards && isSignetOfLearning(item)))
//            storeItemInStorage(item, item->storage);
//}

//class A : public QObject
//{
//    Q_OBJECT

//public:
//    A(QObject *parent = 0) : QObject() {}
//    virtual ~A() {}

//public slots:
//    void started() { qDebug() << Q_FUNC_INFO; }
//    void progressRangeChanged(int min, int max) { qDebug() << "min" << min << "max" << max; }
//    void	progressTextChanged ( const QString & progressText ) { qDebug() << "progressText" << progressText; }
//    void	progressValueChanged ( int progressValue ) { qDebug() << "progressValue" << progressValue; }
//    void	finished () { qDebug() << Q_FUNC_INFO; }
//};

//#include "plugyitemssplitter.moc"

ItemsList PlugyItemsSplitter::disenchantAllItems(bool toShards, bool upgradeToCrystals, bool eatSignets, ItemsList *pItems /*= 0*/)
{
    ItemsList disenchantedItems = ItemsPropertiesSplitter::disenchantAllItems(toShards, upgradeToCrystals, eatSignets, pItems);
    if (_shouldApplyActionToAllPages)
    {
        // move signets/shards to the beginning
        if ((toShards && !upgradeToCrystals) || !toShards)
        {
            ProgressBarModal progressBar;
            progressBar.centerInWidget(this);
            progressBar.show();

            quint32 pageWithNewItems = 0;
            foreach (ItemInfo *item, disenchantedItems)
            {
                if ((toShards && isArcaneShard(item)) || (!toShards && isSignetOfLearning(item)))
                {
                    qApp->processEvents();

                    storeItemInStorage(item, item->storage);
                    if (!pageWithNewItems)
                        pageWithNewItems = item->plugyPage;
                }
            }
            INFO_BOX(tr("New items have been stored starting from page %1").arg(pageWithNewItems));
            // TODO: [0.5+] optimize
//            QFutureWatcher<void> *watcher = new QFutureWatcher<void>;
//            A *a = new A;
//            connect(watcher, SIGNAL(started()), a, SLOT(started()));
//            connect(watcher, SIGNAL(progressRangeChanged(int, int)), a, SLOT(progressRangeChanged(int, int)));
//            connect(watcher, SIGNAL(progressTextChanged(const QString &)), a, SLOT(progressTextChanged(const QString &)));
//            connect(watcher, SIGNAL(progressValueChanged(int)), a, SLOT(progressValueChanged(int)));
//            connect(watcher, SIGNAL(finished()), a, SLOT(finished()));
//            QFuture<void> f = QtConcurrent::run(this, &PlugyItemsSplitter::moveItemsToFirstPages, items, toShards);
////            QFuture<void> f = QtConcurrent::map(*items, &PlugyItemsSplitter::moveItemsToFirstPages);
//            watcher->setFuture(f);
        }
        updateSpinbox();
    }
    return disenchantedItems;
}

void PlugyItemsSplitter::upgradeGems(ItemsList *items /*= 0*/)
{
    Q_UNUSED(items);

    ProgressBarModal progressBar;
    progressBar.centerInWidget(this);
    progressBar.show();

    ItemsPropertiesSplitter::upgradeGems(allOrCurrentPageItems());
    updateSpinbox();
}

void PlugyItemsSplitter::upgradeRunes(int reserveRunes, ItemsList *items /*= 0*/)
{
    Q_UNUSED(items);

    ProgressBarModal progressBar;
    progressBar.centerInWidget(this);
    progressBar.show();

    ItemsPropertiesSplitter::upgradeRunes(reserveRunes, allOrCurrentPageItems());
    updateSpinbox();
}


void PlugyItemsSplitter::sortStash(const StashSortOptions &sortOptions)
{
    // load gear sorting order
    QFile f(ResourcePathManager::pathForSortOrderFileName("gear"));
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        showErrorLoadingSortingOrderFile(f);
        return;
    }

    QList<QByteArray> gearBaseTypesOrder;
    while (!f.atEnd())
    {
        QByteArray line = f.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#'))
            continue;
        gearBaseTypesOrder << line.left(line.indexOf('#')).trimmed();
    }
    f.close();

    // load sets' sorting order
    f.setFileName(ResourcePathManager::pathForSortOrderFileName("sets"));
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        showErrorLoadingSortingOrderFile(f);
        return;
    }

    QList<QList<int> > setsOrder;
    while (!f.atEnd())
    {
        QByteArray line = f.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#'))
            continue;

        QList<int> setIds;
        foreach (const QByteArray &range, line.left(line.indexOf('#')).trimmed().split(',')) //-V807
        {
            QList<QByteArray> minMaxId = range.split('-');
            int firstId = minMaxId.first().toInt();
            if (minMaxId.size() == 1)
                setIds << firstId;
            else
                for (int i = firstId, lastId = minMaxId.last().toInt(); i <= lastId; ++i)
                    setIds << i;
        }
        setsOrder << setIds;
    }
    f.close();

    // load misc items sorting order
    f.setFileName(ResourcePathManager::pathForSortOrderFileName("misc"));
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        showErrorLoadingSortingOrderFile(f);
        return;
    }

    QList<QByteArray> miscBaseTypesOrder, thngTypesOrder;
    bool isThngType = false;
    while (!f.atEnd())
    {
        QByteArray line = f.readLine();
        bool isLineIndented = line.startsWith('\t');
        line = line.trimmed();
        if (line.isEmpty() || line.startsWith('#'))
            continue;

        int end = -1;
        do
        {
            end = line.indexOf('#', end + 1);
        } while (end != -1 && line.at(end - 1) == '\\');

        QByteArray baseType = line.left(end != -1 ? end : line.length()).trimmed().replace("\\#", "#");
        if (isThngType)
        {
            if (isLineIndented)
            {
                thngTypesOrder << baseType;
                continue;
            }
            else
                isThngType = false;
        }

        if (baseType == kThngBaseType)
            isThngType = true;
        miscBaseTypesOrder << baseType;
    }
    f.close();

    // split items
    ItemsList selectedItems, tailItems;
    if (sortOptions.firstPage == 1 && sortOptions.lastPage == _lastNotEmptyPage)
        selectedItems = _allItems;
    else
    {
        foreach (ItemInfo *item, _allItems)
        {
            if (item->plugyPage >= sortOptions.firstPage && item->plugyPage <= sortOptions.lastPage)
                selectedItems += item;
            else if (item->plugyPage > sortOptions.lastPage)
                tailItems += item;
        }
    }

    _maxItemHeightInRow = 0;
    quint32 page = sortOptions.firstPage;
    if (sortOptions.isQualityOrderAscending)
    {
        sortMiscItems(    selectedItems, page, sortOptions, miscBaseTypesOrder, thngTypesOrder);
        ++page;
        sortWearableItems(selectedItems, page, sortOptions, gearBaseTypesOrder, setsOrder);
    }
    else
    {
        sortWearableItems(selectedItems, page, sortOptions, gearBaseTypesOrder, setsOrder);
        ++page;
        sortMiscItems(    selectedItems, page, sortOptions, miscBaseTypesOrder, thngTypesOrder);
    }

    // place everything else
    ++page;
    int row = 0, col = 0;
    foreach (const ItemsList &items, itemsSortedByType<QByteArray>(selectedItems))
        storeItemsOnPage(items, false, page, &row, &col);

    // shift all unprocessed items from last pages
    if (page > sortOptions.lastPage && !tailItems.isEmpty())
    {
        quint32 pageShift = page - sortOptions.lastPage;
        foreach (ItemInfo *item, tailItems)
        {
            item->plugyPage += pageShift;
            item->hasChanged = true;
        }
    }

    updateSpinbox();
    emit stashSorted();
}

void PlugyItemsSplitter::insertBlankPages(int pages, bool isAfter)
{
    foreach (ItemInfo *item, ItemDataBase::extractItemsFromPageRange(_allItems, currentPage() + isAfter, _lastNotEmptyPage))
    {
        item->plugyPage += pages;
        item->hasChanged = true;
    }
    if (!isAfter)
        _pageSpinBox->setValue(_pageSpinBox->value() + pages); // stay on the same page

    updateSpinbox();
    emit itemsChanged();
}

void PlugyItemsSplitter::removeCurrentPage()
{
    insertBlankPages(-1, true);
}


bool PlugyItemsSplitter::keyEventHasShift(QKeyEvent *keyEvent)
{
    return keyEvent->key() == Qt::Key_Shift || keyEvent->modifiers() & Qt::SHIFT;
}

void PlugyItemsSplitter::setShortcutTextInButtonTooltip(QPushButton *button, const QKeySequence &keySequence)
{
    button->setToolTip(keySequence.toString(QKeySequence::NativeText));
}

void PlugyItemsSplitter::setNewLastPage(quint32 newLastPage)
{
    _lastNotEmptyPage = newLastPage;

    _pageSpinBox->setSuffix(QString(" / %1").arg(_lastNotEmptyPage));
    _pageSpinBox->setRange(1, _lastNotEmptyPage);
}

void PlugyItemsSplitter::showItem(ItemInfo *item)
{
    if (item)
    {
        if (item->plugyPage)
            _pageSpinBox->setValue(item->plugyPage);
        ItemsPropertiesSplitter::showItem(item);
    }
}

QPair<bool, bool> PlugyItemsSplitter::updateDisenchantButtonsState(bool includeUniques, bool includeSets, bool toCrystals, ItemsList *pItems /*= 0*/)
{
    Q_UNUSED(pItems);
    return ItemsPropertiesSplitter::updateDisenchantButtonsState(includeUniques, includeSets, toCrystals, allOrCurrentPageItems());
}

QPair<bool, bool> PlugyItemsSplitter::updateUpgradeButtonsState(int reserveRunes, ItemsList *pItems /*= 0*/)
{
    Q_UNUSED(pItems);
    return ItemsPropertiesSplitter::updateUpgradeButtonsState(reserveRunes, allOrCurrentPageItems());
}

void PlugyItemsSplitter::setItems(const ItemsList &newItems)
{
    _allItems = newItems;

    ItemsList::const_iterator maxPageIter = std::max_element(_allItems.constBegin(), _allItems.constEnd(), compareItemsByPlugyPage);
    setNewLastPage(maxPageIter == _allItems.constEnd() ? 0 : (*maxPageIter)->plugyPage);

    updateItemsForCurrentPage(false);
}

void PlugyItemsSplitter::updateItemsForCurrentPage(bool pageChanged_ /*= true*/)
{
    bool wasPageEnteredManually = qApp->focusWidget() == _pageSpinBox;

    _pagedItems = ItemDataBase::extractItemsFromPage(_allItems, currentPage());
    updateItems(_pagedItems);

    if (pageChanged_)
    {
        emit itemCountChanged(_allItems.size());
        emit pageChanged();

        if (wasPageEnteredManually)
            _pageSpinBox->setFocus();
    }
}

void PlugyItemsSplitter::leftClicked()
{
    if (_isShiftPressed)
        _pageSpinBox->setValue(1);
    else
        _pageSpinBox->stepDown();
}

void PlugyItemsSplitter::rightClicked()
{
    if (_isShiftPressed)
        _pageSpinBox->setValue(_lastNotEmptyPage);
    else
        _pageSpinBox->stepUp();
}

void PlugyItemsSplitter::left10Clicked()
{
    quint32 step = _isShiftPressed ? 100 : 10;
    _pageSpinBox->setValue(qFloor((_pageSpinBox->value() - 1) / step) * step);
}

void PlugyItemsSplitter::right10Clicked()
{
    quint32 step = _isShiftPressed ? 100 : 10;
    _pageSpinBox->setValue(qCeil((_pageSpinBox->value() + 1) / step) * step);
}

void PlugyItemsSplitter::moveBetweenStashes()
{
    ItemsPropertiesSplitter::moveBetweenStashes();
    setCurrentStorageHasChanged();
}


void PlugyItemsSplitter::showErrorLoadingSortingOrderFile(const QFile &f)
{
    ERROR_BOX(tr("Sorting order not loaded from %1").arg(QDir::toNativeSeparators(f.fileName())) + "\n" + tr("Reason: %1", "error with file").arg(f.errorString()));
}

void PlugyItemsSplitter::sortWearableItems(ItemsList &selectedItems, quint32 &page, const StashSortOptions &sortOptions, const QList<QByteArray> &gearBaseTypesOrder, const QList<QList<int> > &setsOrder)
{
    // sort by quality
    QMap<int, ItemsList> itemsByQuality;
    foreach (ItemInfo *item, selectedItems)
    {
        int key;
        if (item->isQuest || ItemDataBase::Items()->value(item->itemType)->questId > 0)
            key = Quest;
        else if (item->isRW)
            key = RW;
        else
            key = itemQualityMapping().value(item->quality);
        itemsByQuality[key] << item;
    }
    if (itemsByQuality.isEmpty())
        return;

    QMap<int, ItemsList>::const_iterator    iter = sortOptions.isQualityOrderAscending ? itemsByQuality.constBegin() : itemsByQuality.constEnd() - 1;
    QMap<int, ItemsList>::const_iterator endIter = sortOptions.isQualityOrderAscending ? itemsByQuality.constEnd()   : itemsByQuality.constBegin() - 1;
    forever
    {
        int sortQuality = iter.key();
        if (sortQuality == Set)
        {
            // sets use their own ordering
            QHash<int, ItemsList> setItemsById;
            foreach (ItemInfo *item, iter.value())
                setItemsById[item->setOrUniqueId] << item;

            int setsProcessed = 0;
            foreach (const QList<int> &setIds, setsOrder)
            {
                // put together all items from one set
                ItemsList setItems;
                foreach (int setId, setIds)
                    setItems << setItemsById.value(setId);
                if (!setItems.isEmpty())
                {
                    storeItemsOnPage(setItems, false, page);
                    if (setsProcessed < setsOrder.size() - 1)
                        ++page; // start each set from new page
                    _maxItemHeightInRow = 0;

                    foreach (ItemInfo *item, setItems)
                        selectedItems.removeOne(item);
                }
                ++setsProcessed;
            }

            if (!sortOptions.isQualityOrderAscending)
                ++page; // force new page after sets
        }
        else
        {
            // sort items by base types using the text file (swords, axes, etc.)
            QHash<QByteArray, ItemsList> itemsByBaseType = itemsSortedByBaseType(iter.value());
            if (sortQuality != Quest)
            {
                if (sortOptions.shouldSeparateSacred)
                {
                    sortWearableQualityItems(selectedItems, page, sortOptions, gearBaseTypesOrder, itemsByBaseType, true);
                    if (!sortOptions.isEachTypeFromNewPage)
                    {
                        ++page;
                        _maxItemHeightInRow = 0;
                    }
                    sortWearableQualityItems(selectedItems, page, sortOptions, gearBaseTypesOrder, itemsByBaseType, false);
                }
                else
                    sortWearableQualityItems(selectedItems, page, sortOptions, gearBaseTypesOrder, itemsByBaseType);
            }
            else // Quest items
            {
                ItemsList questItems;
                foreach (const ItemsList &items, itemsByBaseType)
                {
                    questItems << items;
                    foreach (ItemInfo *item, items)
                        selectedItems.removeOne(item);
                }

                storeItemsOnPage(questItems, false, page);
                if (sortOptions.isEachTypeFromNewPage)
                    ++page;
            }
        }

        _maxItemHeightInRow = 0;

        sortOptions.isQualityOrderAscending ? ++iter : --iter;
        if (iter != endIter)
        {
            if (!sortOptions.isEachTypeFromNewPage && sortQuality != Set)
                ++page;
            page += sortOptions.diffQualitiesBlankPages;
        }
        else
            break;
    }
}

void PlugyItemsSplitter::sortWearableQualityItems(ItemsList &selectedItems, quint32 &page, const StashSortOptions &sortOptions, const QList<QByteArray> &gearBaseTypesOrder, QHash<QByteArray, ItemsList> &itemsByBaseType, bool isSacredOnly /*= true*/)
{
    bool noQualityItems = true, collectTiered, collectSacred;
    if (sortOptions.shouldSeparateSacred)
    {
        collectTiered = !isSacredOnly;
        collectSacred = isSacredOnly;
    }
    else
        collectTiered = collectSacred = true;

    int row = 0, col = 0, baseTypesProcessed = 0;
    foreach (const QByteArray &itemBaseType, gearBaseTypesOrder)
    {
        ItemsList itemBaseTypeItems;
        if (collectTiered)
            itemBaseTypeItems = itemsByBaseType.take(itemBaseType);
        if (collectSacred)
        {
            // add sacred items
            QHash<QByteArray, ItemsList>::iterator jter = itemsByBaseType.begin(), endJter = itemsByBaseType.end();
            while (jter != endJter)
            {
                ItemsList &items = jter.value();
                for (int i = 0; i < items.size(); ++i)
                {
                    ItemInfo *item = items.at(i);
                    bool itemShouldBeAdded = false;

                    Enums::ItemTypeGeneric::ItemTypeGenericEnum genericType = ItemDataBase::Items()->value(item->itemType)->genericType;
                    if ((genericType == Enums::ItemTypeGeneric::Weapon || genericType == Enums::ItemTypeGeneric::Armor) && isSacred(item))
                    {
                        // force correct ordering of some items (they don't 'inherit' from tiered versions)
                        static const char *kSacredMaskCode = "@45", *kSacredBoneHelmCode = "@46", *kSacredCrownCode = "@21", *kSacredSpikedShieldCode = "@44", *kSacredBoneShieldCode = "@43";
                        static const char *kSpecialHelmType = "bhlm", *kCrownType = "crow", *kSpecialShieldType = "bshi";

                        if (item->itemType == kSacredMaskCode || item->itemType == kSacredBoneHelmCode) // mask, bone helm
                            itemShouldBeAdded = itemBaseType == kSpecialHelmType;
                        else if (item->itemType == kSacredCrownCode) // crown
                            itemShouldBeAdded = itemBaseType == kCrownType;
                        else if (item->itemType == kSacredSpikedShieldCode || item->itemType == kSacredBoneShieldCode) // spiked/bone shield
                            itemShouldBeAdded = itemBaseType == kSpecialShieldType;
                        else // default filter
                            itemShouldBeAdded = ItemDataBase::ItemTypes()->value(jter.key()).baseItemTypes.contains(itemBaseType);
                    }

                    if (itemShouldBeAdded)
                    {
                        itemBaseTypeItems << item;
                        items.removeAt(i--);
                    }
                }

                if (items.isEmpty())
                    jter = itemsByBaseType.erase(jter);
                else
                    ++jter;
            }
        }

        if (!itemBaseTypeItems.isEmpty())
        {
            noQualityItems = false;
            // sort items by types (broad sword, scimitar, etc.). use item names (text before brackets) to determine sub-type.
            QMap<QString, ItemsList> sortedItemsByType; // using QMap instead of QHash to have determined order
            foreach (ItemInfo *item, itemBaseTypeItems)
            {
                ItemBase *baseInfo = ItemDataBase::Items()->value(item->itemType);
                QString itemName = baseInfo->name, key = itemName.left(itemName.lastIndexOf('(')).trimmed();
                if (itemBaseType == "hamm")
                {
                    // both 1h and 2h hammers have same type, so the key must be modified to force correct order
                    if (baseInfo->is2h)
                        key.prepend(QString::number(5 - baseInfo->height)); // maul has height 4, and great maul - 3
                    else
                        key.prepend("0");
                }
                sortedItemsByType[key] << item;

                selectedItems.removeOne(item);
            }
            // sort each group of items by tiers
            for (QMap<QString, ItemsList>::iterator jter = sortedItemsByType.begin(), endJter = sortedItemsByType.end(); jter != endJter; ++jter)
            {
                ItemsList &items = jter.value();
                // to sort by tiers, sort by rlvl. SUs are sorted by ID.
                qSort(items.begin(), items.end(), sortOptions.shouldSeparateEth ? compareItemsByRlvlAndEthereality : compareItemsByRlvl);
            }

            // save new order
            foreach (const ItemsList &items, sortedItemsByType)
            {
                storeItemsOnPage(items, sortOptions.isNewRowTier, page, &row, &col, sortOptions.isNewRowCotw);
                if (sortOptions.isEachTypeFromNewPage)
                {
                    ++page;
                    row = col = _maxItemHeightInRow = 0;
                }
            }

            if (sortOptions.isEachTypeFromNewPage && baseTypesProcessed++ < gearBaseTypesOrder.size() - 1)
                page += sortOptions.diffTypesBlankPages;
        }
    }

    if (noQualityItems && !sortOptions.isEachTypeFromNewPage)
        --page; // don't create empty pages if there're no items of current quality
}

void PlugyItemsSplitter::sortMiscItems(ItemsList &selectedItems, quint32 &page, const StashSortOptions &sortOptions, const QList<QByteArray> &miscBaseTypesOrder, const QList<QByteArray> &thngTypesOrder)
{
    // sort misc items
    int baseTypesProcessed = 0, row = 0, col = 0;
    QHash<QByteArray, ItemsList> itemsByBaseType = itemsSortedByBaseType(selectedItems);
    foreach (const QByteArray &itemBaseType, miscBaseTypesOrder)
    {
        if (itemBaseType != kThngBaseType)
        {
            ItemsList itemBaseTypeItems;
            if (itemBaseType.contains('.'))
            {
                QRegExp re(itemBaseType);
                QHash<QByteArray, ItemsList>::iterator jter = itemsByBaseType.begin(), endJter = itemsByBaseType.end();
                while (jter != endJter)
                {
                    if (re.indexIn(jter.key()) != -1)
                    {
                        itemBaseTypeItems << jter.value();
                        jter = itemsByBaseType.erase(jter);
                    }
                    else
                        ++jter;
                }
            }
            else
            {
                itemBaseTypeItems = itemsByBaseType.take(itemBaseType);
                // add 'inherited' items
                QHash<QByteArray, ItemsList>::iterator jter = itemsByBaseType.begin(), endJter = itemsByBaseType.end();
                while (jter != endJter)
                {
                    ItemsList &items = jter.value();
                    for (int i = 0; i < items.size(); ++i)
                    {
                        if (ItemDataBase::ItemTypes()->value(jter.key()).baseItemTypes.contains(itemBaseType))
                        {
                            itemBaseTypeItems << items.at(i);
                            items.removeAt(i--);
                        }
                    }

                    if (items.isEmpty())
                        jter = itemsByBaseType.erase(jter);
                    else
                        ++jter;
                }
            }

            if (!itemBaseTypeItems.isEmpty())
            {
                QMap<QByteArray, ItemsList> sortedItemsByType; // using QMap instead of QHash to have determined order
                bool isGem = itemBaseType.startsWith("gem"), isMO = itemBaseType.startsWith("asa");
                if (isGem)
                {
                    // sort gems by type at first using specified order
                    foreach (ItemInfo *item, itemBaseTypeItems)
                        sortedItemsByType[gemTypesOrderMapping().value(ItemDataBase::Items()->value(item->itemType)->types.first())] << item;
                    // to sort gems by quality, sort by rlvl
                    for (QMap<QByteArray, ItemsList>::iterator jter = sortedItemsByType.begin(), endJter = sortedItemsByType.end(); jter != endJter; ++jter)
                    {
                        ItemsList &items = jter.value();
                        qSort(items.begin(), items.end(), compareItemsByRlvl);
                    }
                }
                else if (isMO)
                {
                    // (U)MOs are sorted by image name at first
                    foreach (ItemInfo *item, itemBaseTypeItems)
                        sortedItemsByType[ItemDataBase::Items()->value(item->itemType)->imageName] << item;
                    // and then by type
                    for (QMap<QByteArray, ItemsList>::iterator jter = sortedItemsByType.begin(), endJter = sortedItemsByType.end(); jter != endJter; ++jter)
                    {
                        ItemsList &items = jter.value();
                        qSort(items.begin(), items.end(), compareItemsByCode);
                    }
                }
                else // everything else is simply sorted by item code
                    sortedItemsByType = itemsSortedByType<QByteArray>(itemBaseTypeItems);

                // save new order
                bool isClassCharm_ = itemBaseType.startsWith("ara"), isVisuallySame = isMO || isClassCharm_, isNewRow = sortOptions.isNewRowVisuallyDifferentMisc || isVisuallySame;
                if (isVisuallySame && !sortOptions.isNewRowVisuallyDifferentMisc)
                {
                    ++row;
                    col = 0;
                }
                foreach (const ItemsList &items, sortedItemsByType)
                {
                    storeItemsOnPage(items, isNewRow, page, &row, &col);

                    if (sortOptions.shouldPlaceSimilarMiscItemsOnOnePage)
                    {
                        if (isNewRow)
                        {
                            row += _maxItemHeightInRow;
                            col = _maxItemHeightInRow = 0;
                        }
                    }
                    else
                    {
                        ++page;
                        row = col = 0;
                    }

                    foreach (ItemInfo *item, items)
                        selectedItems.removeOne(item);
                }

                if (!sortOptions.shouldPlaceSimilarMiscItemsOnOnePage && baseTypesProcessed++ < miscBaseTypesOrder.size() - 1)
                    page += sortOptions.diffTypesBlankPages;
            }

            _maxItemHeightInRow = 0;
        }
        else // thng
        {
            ItemsList thngItems = itemsByBaseType.value(itemBaseType);
            foreach (const QByteArray &thngType, thngTypesOrder)
            {
                QRegExp re(QString::fromLatin1(thngType));
                ItemsList typeItems;
                for (int i = 0; i < thngItems.size(); ++i)
                {
                    ItemInfo *item = thngItems.at(i);
                    if (re.indexIn(item->itemType) != -1)
                    {
                        typeItems << item;

                        thngItems.removeAt(i--);
                        selectedItems.removeOne(item);
                    }
                }

                if (!typeItems.isEmpty())
                {
                    bool isEvilEye = thngType == "!@[1-5]", isBrain = thngType == "2x\\d";
                    bool isVisuallySame = isEvilEye || isBrain, isNewRow = sortOptions.isNewRowVisuallyDifferentMisc || isVisuallySame;
                    if (isVisuallySame && !sortOptions.isNewRowVisuallyDifferentMisc)
                    {
                        ++row;
                        col = 0;
                    }

                    qSort(typeItems.begin(), typeItems.end(), compareItemsByCode);
                    storeItemsOnPage(typeItems, isNewRow, page, &row, &col);

                    if (sortOptions.shouldPlaceSimilarMiscItemsOnOnePage)
                    {
                        if (isNewRow)
                        {
                            row += _maxItemHeightInRow;
                            col = _maxItemHeightInRow = 0;
                        }
                    }
                    else
                    {
                        ++page;
                        row = col = 0;
                    }

                    if (!sortOptions.shouldPlaceSimilarMiscItemsOnOnePage && baseTypesProcessed++ < miscBaseTypesOrder.size() - 1)
                        page += sortOptions.diffTypesBlankPages;
                }
            }
        }
    }
}

void PlugyItemsSplitter::storeItemsOnPage(const ItemsList &items, bool shouldStartAnotherTypeFromNewRow, quint32 &page, int *pRow /*= 0*/, int *pCol /*= 0*/, bool shouldStartAnotherCotwFromNewRow /*= false*/)
{
    static const int rows = _itemsModel->rowCount(), columns = _itemsModel->columnCount();
    static ItemsList pageItems;
    static bool isFillingPage = false;

    ItemInfo *previousItem = 0;
    int rowFoo = 0, colBar = 0;
    int &row = pRow ? *pRow : rowFoo, &col = pCol ? *pCol : colBar;
    if (row >= rows)
    {
        row = col = 0;
        ++page;
    }

    foreach (ItemInfo *item, items)
    {
        bool isCotw_ = isCotw(item), isNewRow = shouldStartAnotherTypeFromNewRow || (shouldStartAnotherCotwFromNewRow && isCotw_);
        ItemBase *baseInfo = ItemDataBase::Items()->value(item->itemType);
        if (isNewRow)
        {
            if (!previousItem)
                previousItem = item;
            else if (previousItem->itemType != item->itemType || (areBothItemsSetOrUnique(item, previousItem) && (!isCotw_ || shouldStartAnotherCotwFromNewRow) && previousItem->setOrUniqueId != item->setOrUniqueId))
            {
                // set wrong column value to switch to new row
                col = columns;
                previousItem = item;
            }
        }
        // fill stash by rows
        if (col + baseInfo->width > columns || row + baseInfo->height > rows || isFillingPage)
        {
            // switch to new row
            int oldRow = row;
            if (!isFillingPage)
            {
                col = 0;
                if (_maxItemHeightInRow)
                {
                    row += _maxItemHeightInRow;
                    _maxItemHeightInRow = 0;
                }
                else
                    row += baseInfo->height;
            }

            if (row + baseInfo->height > rows || isFillingPage)
            {
                if (!isNewRow)
                {
                    // slightly modified version of ItemDataBase::storeItemIn()
                    isFillingPage = true;
                    for (quint8 i = oldRow; i < rows; ++i)
                    {
                        for (quint8 j = 0; j < columns; ++j)
                        {
                            if (ItemDataBase::canStoreItemAt(i, j, item->itemType, pageItems, rows, columns))
                            {
                                row = i;
                                col = j;
                                goto STORE_ITEM; // goto woo-hoo!
                            }
                        }
                    }
                }

                // switch to new page if failed to find space
                ++page;
                row = col = _maxItemHeightInRow = 0;
                isFillingPage = false;
            }
        }

    STORE_ITEM:
        item->move(row, col, page);
        col += baseInfo->width;

        if (!pageItems.isEmpty() && pageItems.last()->plugyPage < page)
            pageItems.clear();
        pageItems += item;

        if (_maxItemHeightInRow < baseInfo->height)
            _maxItemHeightInRow = baseInfo->height;
    }
}


QHash<QByteArray, ItemsList> PlugyItemsSplitter::itemsSortedByBaseType(const ItemsList &items)
{
    QHash<QByteArray, ItemsList> itemsByBaseType;
    foreach (ItemInfo *item, items)
    {
        QList<QByteArray> baseTypes = ItemDataBase::Items()->value(item->itemType)->types;
        QByteArray baseType = baseTypes.first();
        // sacred belts also have first type 'belt' so that they could display 4 rows of potions
        if (baseType == "belt" && !isTiered(baseTypes))
            baseType = baseTypes.last();
        itemsByBaseType[baseType] << item;
    }
    return itemsByBaseType;
}

template<typename K>
QMap<K, ItemsList> PlugyItemsSplitter::itemsSortedByType(const ItemsList &items)
{
    QMap<K, ItemsList> itemsByType;
    foreach (ItemInfo *item, items)
        itemsByType[item->itemType] << item;
    return itemsByType;
}
