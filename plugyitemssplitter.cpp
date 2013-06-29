#include "plugyitemssplitter.h"
#include "itemstoragetableview.h"
#include "itemdatabase.h"
#include "itemsviewerdialog.h"
#include "progressbarmodal.hpp"
#include "itemstoragetablemodel.h"
#include "resourcepathmanager.hpp"
#include "itemparser.h"
#include "reversebitwriter.h"

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


PlugyItemsSplitter::PlugyItemsSplitter(ItemStorageTableView *itemsView, QWidget *parent) : ItemsPropertiesSplitter(itemsView, parent), _shouldApplyActionToAllPages(true)
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
    _pageSpinBox->setRange(1, (std::numeric_limits<quint32>::max)());
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

void PlugyItemsSplitter::removeItemFromList(ItemInfo *item, bool emitSignal /*= true*/)
{
    ItemsPropertiesSplitter::removeItemFromList(item, emitSignal);
    if (isItemInCurrentStorage(item))
        _pagedItems.removeOne(item);
}

bool PlugyItemsSplitter::storeItemInStorage(ItemInfo *item, int storage)
{
    for (quint32 i = 1; i <= _lastNotEmptyPage; ++i)
    {
        if (ItemDataBase::storeItemIn(item, static_cast<Enums::ItemStorage::ItemStorageEnum>(storage), ItemsViewerDialog::rowsInStorageAtIndex(storage), i))
        {
            addItemToList(item, false);
            return true;
        }
    }
    return false;
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
    //pItems = itemsForSelectedRange();
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
            // TODO: [0.4+] optimize
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

    ItemsPropertiesSplitter::upgradeGems(itemsForSelectedRange());
    updateSpinbox();
}

void PlugyItemsSplitter::upgradeRunes(ItemsList *items /*= 0*/)
{
    Q_UNUSED(items);

    ProgressBarModal progressBar;
    progressBar.centerInWidget(this);
    progressBar.show();

    ItemsPropertiesSplitter::upgradeRunes(itemsForSelectedRange());
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
        foreach (const QByteArray &range, line.left(line.indexOf('#')).trimmed().split(','))
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

    QList<QByteArray> miscBaseTypesOrder;
    while (!f.atEnd())
    {
        QByteArray line = f.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#'))
            continue;

        int end = line.indexOf('#');
        if (line.at(end - 1) == '\\')
            end = line.indexOf('#', end + 1);
        miscBaseTypesOrder << line.left(end).trimmed().replace("\\#", "#");
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
            key = itemQualityMapping()[item->quality];
        itemsByQuality[key] << item;
    }

    const QHash<QByteArray, ItemBase *> *const kItemsBaseInfo = ItemDataBase::Items();
    const QHash<QByteArray, QList<QByteArray> > *const kItemTypesInfo = ItemDataBase::ItemTypes();
    bool noNewPageInsideGemsAndRunes = true, startEachGemAndRuneFromNewRow = true, noNewPageInsideClassCharms = true, startEachClassCharmFromNewRow = true;

    quint32 page = sortOptions.firstPage;
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

            foreach (const QList<int> &setIds, setsOrder)
            {
                // put together all items from one set
                ItemsList setItems;
                foreach (int setId, setIds)
                    setItems << setItemsById.value(setId);
                if (!setItems.isEmpty())
                {
                    int row = 0, col = 0;
                    storeItemsOnPage(setItems, page, row, col, false);
                    ++page; // start each set from new page

                    foreach (ItemInfo *item, setItems)
                        selectedItems.removeOne(item);
                }
            }
        }
        else
        {
            // sort items by base types using the text file (swords, axes, etc.)
            QHash<QByteArray, ItemsList> itemsByBaseType;
            foreach (ItemInfo *item, iter.value())
                itemsByBaseType[kItemsBaseInfo->value(item->itemType)->types.first()] << item;

            int baseTypesProcessed = 0;
            foreach (const QByteArray &itemBaseType, gearBaseTypesOrder)
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
                    itemBaseTypeItems = itemsByBaseType.take(itemBaseType);
                // add sacred items or charms
                QHash<QByteArray, ItemsList>::iterator jter = itemsByBaseType.begin(), endJter = itemsByBaseType.end();
                while (jter != endJter)
                {
                    ItemsList &items = jter.value();
                    for (int i = 0; i < items.size(); ++i)
                    {
                        // force correct ordering of some items (they don't 'inherit' from tiered versions)
                        static const char *kSacredMaskCode = "@45", *kSacredBoneHelmCode = "@46", *kSacredCrownCode = "@21", *kSacredSpikedShieldCode = "@44", *kSacredBoneShieldCode = "@43";
                        static const char *kSpecialHelmType = "bhlm", *kCrownType = "crow", *kSpecialShieldType = "bshi";

                        ItemInfo *item = items.at(i);
                        bool itemShouldBeAdded;
                        if (item->itemType == kSacredMaskCode || item->itemType == kSacredBoneHelmCode) // mask, bone helm
                            itemShouldBeAdded = itemBaseType == kSpecialHelmType;
                        else if (item->itemType == kSacredCrownCode) // crown
                            itemShouldBeAdded = itemBaseType == kCrownType;
                        else if (item->itemType == kSacredSpikedShieldCode || item->itemType == kSacredBoneShieldCode) // spiked/bone shield
                            itemShouldBeAdded = itemBaseType == kSpecialShieldType;
                        else // default filter
                            itemShouldBeAdded = kItemTypesInfo->value(jter.key()).contains(itemBaseType);

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

                if (!itemBaseTypeItems.isEmpty())
                {
                    QMap<QString, ItemsList> sortedItemsByType;
                    bool isRune = itemBaseType == "rune", isClassCharm_ = itemBaseType.startsWith("ara");
                    if (isRune || isClassCharm_)
                    {
                        // runes and class charms are simply sorted by item code
                        QMap<QString, ItemsList> runesByType;
                        foreach (ItemInfo *item, itemBaseTypeItems)
                            runesByType[item->itemType] << item;
                        sortedItemsByType = runesByType;
                    }
                    else
                    {
                        // sort items by types (broad sword, scimitar, etc.). use item names (text before brackets) to determine sub-type.
                        QMap<QString, ItemsList> itemsByType; // using QMap instead of QHash to have determined order
                        foreach (ItemInfo *item, itemBaseTypeItems)
                        {
                            QString itemName = kItemsBaseInfo->value(item->itemType)->name;
                            itemsByType[itemName.left(itemName.indexOf('(')).trimmed()] << item;
                        }
                        // sort each group of items by tiers
                        for (QMap<QString, ItemsList>::iterator jter = itemsByType.begin(), endJter = itemsByType.end(); jter != endJter; ++jter)
                        {
                            ItemsList &items = jter.value();
                            // to sort by tiers, sort by rlvl. SUs are sorted by ID.
                            qSort(items.begin(), items.end(), compareItemsByRlvl);
                        }
                        sortedItemsByType = itemsByType;
                    }
                    // save new order
                    bool isGemOrRune = itemBaseType.startsWith("gem") || isRune;
                    int row = 0, col = 0;
                    foreach (const ItemsList &items, sortedItemsByType)
                    {
                        storeItemsOnPage(items, page, row, col, sortQuality == Quest ? false : (isGemOrRune ? startEachGemAndRuneFromNewRow : true));

                        col = 0;
                        if (startEachGemAndRuneFromNewRow && ((noNewPageInsideGemsAndRunes && isGemOrRune) || (noNewPageInsideClassCharms && isClassCharm_)))
                            ++row;
                        else
                        {
                            ++page; // start new sub-type from new page
                            row = 0;
                        }

                        foreach (ItemInfo *item, items)
                            selectedItems.removeOne(item);
                    }
                    if ((noNewPageInsideGemsAndRunes && isGemOrRune) || (noNewPageInsideClassCharms && isClassCharm_))
                        ++page;

                    if (baseTypesProcessed++ < gearBaseTypesOrder.size() - 1)
                        page += sortOptions.diffTypesBlankPages;
                }
            }
        }

        sortOptions.isQualityOrderAscending ? ++iter : --iter;
        if (iter != endIter)
            page += sortOptions.diffQualitiesBlankPages;
        else
            break;
    }

    // sort misc items

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

void PlugyItemsSplitter::insertBlankPages(int pages)
{
    foreach (ItemInfo *item, ItemDataBase::extractItemsFromPageRange(_allItems, currentPage() + 1, _lastNotEmptyPage))
        item->plugyPage += pages;

    updateSpinbox();
    emit itemsChanged();
}


bool PlugyItemsSplitter::keyEventHasShift(QKeyEvent *keyEvent)
{
    return keyEvent->key() == Qt::Key_Shift || keyEvent->modifiers() & Qt::SHIFT;
}

void PlugyItemsSplitter::setShortcutTextInButtonTooltip(QPushButton *button, const QKeySequence &keySequence)
{
    button->setToolTip(keySequence.toString(QKeySequence::NativeText));
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
    return ItemsPropertiesSplitter::updateDisenchantButtonsState(includeUniques, includeSets, toCrystals, itemsForSelectedRange());
}

QPair<bool, bool> PlugyItemsSplitter::updateUpgradeButtonsState(ItemsList *pItems /*= 0*/)
{
    Q_UNUSED(pItems);
    return ItemsPropertiesSplitter::updateUpgradeButtonsState(itemsForSelectedRange());
}

void PlugyItemsSplitter::setItems(const ItemsList &newItems)
{
    _allItems = newItems;

    // using _allItems.last()->plugyPage would've been easy, but it's not always correct (new items added via app are added to the end)
    ItemsList::const_iterator maxPageIter = std::max_element(_allItems.constBegin(), _allItems.constEnd(), compareItemsByPlugyPage);
    _lastNotEmptyPage = maxPageIter == _allItems.constEnd() ? 0 : (*maxPageIter)->plugyPage;

    _pageSpinBox->setSuffix(QString(" / %1").arg(_lastNotEmptyPage));
    _pageSpinBox->setRange(1, _lastNotEmptyPage);

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


void PlugyItemsSplitter::showErrorLoadingSortingOrderFile(const QFile &f)
{
    ERROR_BOX(tr("Sorting order not loaded from %1").arg(QDir::toNativeSeparators(f.fileName())) + "\n" + tr("Reason: %1", "error with file").arg(f.errorString()));
}

void PlugyItemsSplitter::storeItemsOnPage(const ItemsList &items, quint32 &page, int &row, int &col, bool shouldStartAnotherTypeFromNewRow)
{
    static const int rows = _itemsModel->rowCount(), columns = _itemsModel->columnCount();

    ItemInfo *previousItem = 0;
    foreach (ItemInfo *item, items)
    {
        ItemBase *baseInfo = ItemDataBase::Items()->value(item->itemType);
        if (shouldStartAnotherTypeFromNewRow)
        {
            if (!previousItem)
                previousItem = item;
            else if (previousItem->itemType != item->itemType || (areBothItemsSetOrUnique(item, previousItem) && previousItem->setOrUniqueId != item->setOrUniqueId))
            {
                // if another tier item or set piece fits current row, simulate the opposite
                if (col + baseInfo->width <= columns)
                    col = columns;
                previousItem = item;
            }
        }
        // fill stash by rows
        if (col + baseInfo->width > columns)
        {
            // switch to new row
            row += baseInfo->height;
            col = 0;

            if (row + baseInfo->height > rows)
            {
                // switch to new page
                ++page;
                row = 0;
            }
        }

        item->move(row, col, page);

        col += baseInfo->width;
    }
}

quint32 PlugyItemsSplitter::currentPage() const
{
    return static_cast<quint32>(_pageSpinBox->value());
}
