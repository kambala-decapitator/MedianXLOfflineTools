#include "plugyitemssplitter.h"
#include "itemstoragetableview.h"

#include <QPushButton>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QCheckBox>

#include <qmath.h>

#include <limits>


static const QString kIconPathFormat(":/PlugyArrows/icons/plugy/%1.png");

bool compareItemsByPlugyPage(ItemInfo *a, ItemInfo *b)
{
    return a->plugyPage < b->plugyPage;
}


PlugyItemsSplitter::PlugyItemsSplitter(ItemStorageTableView *itemsView, QWidget *parent) : ItemsPropertiesSplitter(itemsView, parent)
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

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(_left10Button);
    hlayout->addWidget(_leftButton);
    hlayout->addWidget(_pageSpinBox);
    hlayout->addWidget(_rightButton);
    hlayout->addWidget(_right10Button);
    // glue everything together (used mainly for Mac OS X)
    hlayout->setSpacing(0);
    hlayout->setContentsMargins(QMargins());

    _hline = new QFrame(this);
    _hline->setFrameShape(QFrame::HLine);

    _applyActionToAllPagesCheckbox = new QCheckBox(tr("Apply to all pages"), this);
    _applyActionToAllPagesCheckbox->setChecked(true);

    QVBoxLayout *vlayout = static_cast<QVBoxLayout *>(widget(0)->layout());
    vlayout->insertLayout(1, hlayout);
    vlayout->insertWidget(2, _hline);
    vlayout->insertWidget(3, _applyActionToAllPagesCheckbox, 0, Qt::AlignCenter);
//    vlayout->setSpacing(0);
//    vlayout->setContentsMargins(QMargins());

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

void PlugyItemsSplitter::setItems(const ItemsList &newItems)
{
    _allItems = newItems;

    // using _allItems.last()->plugyPage would've been easy, but it's not always correct (new items added via app are added to the end)
    ItemsList::iterator maxPageIter = std::max_element(_allItems.begin(), _allItems.end(), compareItemsByPlugyPage);
    _lastNotEmptyPage = maxPageIter == _allItems.end() ? 0 : (*maxPageIter)->plugyPage;

    _pageSpinBox->setSuffix(QString(" / %1").arg(_lastNotEmptyPage));
    _pageSpinBox->setRange(1, _lastNotEmptyPage);

    updateItemsForCurrentPage(false);
}

void PlugyItemsSplitter::updateItemsForCurrentPage(bool pageChanged /*= true*/)
{
    ItemsList pagedItems;
    foreach (ItemInfo *item, _allItems)
        if (item->plugyPage == static_cast<quint32>(_pageSpinBox->value()))
            pagedItems += item;
    updateItems(pagedItems);

    if (pageChanged)
        emit itemCountChanged(_allItems.size());
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
