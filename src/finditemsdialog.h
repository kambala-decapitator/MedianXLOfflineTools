#ifndef FINDITEMSWIDGET_H
#define FINDITEMSWIDGET_H

#include "findresultswidget.h"

#include <QDialog>


class ItemInfo;

namespace Ui { class FindItemsDialog; }
class QShowEvent;

class HelpWindowDisplayManager;

class FindItemsDialog : public QDialog
{
    Q_OBJECT

public:
    FindItemsDialog(QWidget *parent = 0);
    virtual ~FindItemsDialog();

    void saveSettings();
    void clearResults();
    void sortAndUpdateSearchResult();

public slots:
    void resetSearchStatus();
    virtual void reject();

protected:
    virtual void showEvent(QShowEvent *e);

signals:
    void itemFound(ItemInfo *item);

private slots:
    void findNext();
    void findPrevious();
    void toggleResults();

    void updateCurrentIndexForItem(ItemInfo *item);
    void searchTextChanged();
    void changeComboboxCaseSensitivity(bool isCaseSensitive);

private:
    Ui::FindItemsDialog *ui;
    FindResultsWidget *_resultsWidget;
    HelpWindowDisplayManager *_helpDislplayManager;

    QList<SearchResultItem> _searchResult; // item and matched string
    bool _wasSearchPerformed, _searchResultsChanged;
    int _currentIndex;
    int _lastResultsHeight;

    void performSearch();
    void nothingFound(bool wasSearchDone = true);

    void loadSettings();
    void updateWindowTitle();
    void changeItem(bool changeResultsSelection = true);
    void setButtonsDisabled(bool disabled, bool updateResultButton = true);
};

#endif // FINDITEMSWIDGET_H
