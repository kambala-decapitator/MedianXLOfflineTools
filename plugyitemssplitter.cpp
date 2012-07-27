#include "plugyitemssplitter.h"
#include "itemstoragetableview.h"
#include "itemdatabase.h"
#include "itemsviewerdialog.h"

#include <QPushButton>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QProgressBar>

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
    return item->plugyPage == _pageSpinBox->value();
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
    bool result;
    for (quint32 i = 1; i <= _lastNotEmptyPage; ++i)
    {
        result = ItemDataBase::storeItemIn(item, static_cast<Enums::ItemStorage::ItemStorageEnum>(storage), ItemsViewerDialog::rowsInStorageAtIndex(storage), i);
        if (result)
            break;
    }
    if (result)
        addItemToList(item, false);
    return result;
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

class ProgressBarModal : public QProgressBar
{
public:
    ProgressBarModal(QWidget *parent = 0) : QProgressBar(parent)
    {
        setRange(0, 0);
        setTextVisible(false);
        setWindowTitle(tr("Please wait..."));
        setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);
        setWindowModality(Qt::ApplicationModal);
        adjustSize();
        setFixedSize(size());
    }

    void centerInWidget(QWidget *w) { move(w->mapToGlobal(QPoint((w->size().width() - size().width()) / 2, (w->size().height() - size().height()) / 2))); }

protected:
    void closeEvent(QCloseEvent *e) { e->ignore(); }
};

void PlugyItemsSplitter::disenchantAllItems(bool toShards, bool upgradeToCrystals, bool eatSignets, bool includeUniques, bool includeSets, ItemsList *items /*= 0*/)
{
    items = _shouldApplyActionToAllPages ? &_allItems : &_pagedItems;
    ItemsPropertiesSplitter::disenchantAllItems(toShards, upgradeToCrystals, eatSignets, includeUniques, includeSets, items);
    if (_shouldApplyActionToAllPages)
    {
        // move signets/shards to the beginning
        if ((toShards && !upgradeToCrystals) || !toShards)
        {
            ProgressBarModal progressBar;
            //progressBar.setRange(0, 0);
            //progressBar.setTextVisible(false);
            //progressBar.setWindowTitle(tr("Please wait..."));
            //progressBar.setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);
            //progressBar.setWindowModality(Qt::ApplicationModal);
            //progressBar.adjustSize();
            //progressBar.setFixedSize(progressBar.size());
            //progressBar.move(mapToGlobal(QPoint((size().width() - progressBar.size().width()) / 2, (size().height() - progressBar.size().height()) / 2)));
            progressBar.centerInWidget(this);
            progressBar.show();

            foreach (ItemInfo *item, *items)
            {
                if ((toShards && isArcaneShard(item)) || (!toShards && isSignetOfLearning(item)))
                {
                    qApp->processEvents();
                    storeItemInStorage(item, item->storage);
                }
            }
            // TODO: [0.4+] optimize
//            QFutureWatcher<void> *watcher = new QFutureWatcher<void>;
//            A *a = new A;
//            connect(watcher, SIGNAL(started()), a, SLOT(started()));
//            connect(watcher, SIGNAL(progressRangeChanged(int, int)), a, SLOT(progressRangeChanged(int, int)));
//            connect(watcher, SIGNAL(progressTextChanged ( const QString &)), a, SLOT(progressTextChanged ( const QString &)));
//            connect(watcher, SIGNAL(progressValueChanged ( int)), a, SLOT(progressValueChanged ( int)));
//            connect(watcher, SIGNAL(finished()), a, SLOT(finished()));
//            QFuture<void> f = QtConcurrent::run(this, &PlugyItemsSplitter::moveItemsToFirstPages, items, toShards);
////            QFuture<void> f = QtConcurrent::map(*items, &PlugyItemsSplitter::moveItemsToFirstPages);
//            watcher->setFuture(f);
        }
        setItems(_allItems); // update spinbox value and range
    }
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

QPair<bool, bool> PlugyItemsSplitter::updateDisenchantButtonsState(bool includeUniques, bool includeSets, bool toCrystals, ItemsList *items /*= 0*/)
{
    Q_UNUSED(items);
    return ItemsPropertiesSplitter::updateDisenchantButtonsState(includeUniques, includeSets, toCrystals, _shouldApplyActionToAllPages ? &_allItems : &_pagedItems);
}

QPair<bool, bool> PlugyItemsSplitter::updateUpgradeButtonsState(ItemsList *items /*= 0*/)
{
    Q_UNUSED(items);
    return ItemsPropertiesSplitter::updateUpgradeButtonsState(_shouldApplyActionToAllPages ? &_allItems : &_pagedItems);
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
    quint32 currentPage = static_cast<quint32>(_pageSpinBox->value());
    _pagedItems.clear();
    foreach (ItemInfo *item, _allItems)
        if (item->plugyPage == currentPage)
            _pagedItems += item;
    updateItems(_pagedItems);

    if (pageChanged_)
    {
        emit itemCountChanged(_allItems.size());
        emit pageChanged();
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
