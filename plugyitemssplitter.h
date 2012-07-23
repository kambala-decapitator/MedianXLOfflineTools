#ifndef PLUGYITEMSSPLITTER_H
#define PLUGYITEMSSPLITTER_H

#include "itemspropertiessplitter.h"


class QDoubleSpinBox;
class QKeySequence;

class PlugyItemsSplitter : public ItemsPropertiesSplitter
{
    Q_OBJECT

public:
    explicit PlugyItemsSplitter(ItemStorageTableView *itemsView, QWidget *parent = 0);
    virtual ~PlugyItemsSplitter() {}

    virtual void setItems(const ItemsList &newItems);
    virtual void showItem(ItemInfo *item);

public slots:
    // these 8 are connected to main menu actions
    void previous10Pages() { left10Clicked();  }
    void previousPage()    { leftClicked();    }
    void nextPage()        { rightClicked();   }
    void next10Pages()     { right10Clicked(); }

    // emulating pressed shift if action was pressed by mouse
    void previous100Pages() { emulateShiftAndInvokeMethod(&PlugyItemsSplitter::left10Clicked);  }
    void firstPage()        { emulateShiftAndInvokeMethod(&PlugyItemsSplitter::leftClicked);    }
    void lastPage()         { emulateShiftAndInvokeMethod(&PlugyItemsSplitter::rightClicked);   }
    void next100Pages()     { emulateShiftAndInvokeMethod(&PlugyItemsSplitter::right10Clicked); }

protected:
    void keyPressEvent(QKeyEvent *keyEvent);
    void keyReleaseEvent(QKeyEvent *keyEvent);

private slots:
    void updateItemsForCurrentPage(bool pageChanged = true);
    void leftClicked();
    void rightClicked();
    void left10Clicked();
    void right10Clicked();

    void applyActionToAllPagesChanged();

private:
    QPushButton *_left10Button, *_leftButton, *_rightButton, *_right10Button;
    QDoubleSpinBox *_pageSpinBox;
    QFrame *_hline;
    QCheckBox *_applyActionToAllPagesCheckbox;

    quint32 _lastNotEmptyPage;
    bool _isShiftPressed;
    ItemsList _pagedItems;

    void emulateShiftAndInvokeMethod(void (PlugyItemsSplitter::*method)(void)) { _isShiftPressed = true; (this->*method)(); _isShiftPressed = false; }
    bool keyEventHasShift(QKeyEvent *keyEvent);
    void setShortcutTextInButtonTooltip(QPushButton *button, const QKeySequence &keySequence);
};

#endif // PLUGYITEMSSPLITTER_H
