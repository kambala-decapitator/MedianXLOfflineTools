#ifndef FINDITEMSWIDGET_H
#define FINDITEMSWIDGET_H

#include "findresultswidget.h"

#include <QDialog>


class ItemInfo;

namespace Ui { class FindItemsDialog; }

class QShowEvent;

class FindItemsDialog : public QDialog
{
    Q_OBJECT

public:
    Ui::FindItemsDialog *ui;

    FindItemsDialog(QWidget *parent = 0);
    virtual ~FindItemsDialog() { delete ui; }

    void saveSettings();
    void clearResults();

public slots:
    void resetSearchStatus();
    virtual void reject() { saveSettings(); QDialog::reject(); }

protected:
    virtual void showEvent(QShowEvent *e);

signals:
    void itemFound(ItemInfo *item);

private slots:
    void findNext();
    void findPrevious();
    void toggleResults();
    void showHelp();

    void updateCurrentIndexForItem(ItemInfo *item);
    void searchTextChanged();

private:
    QList<SearchResultItem> _searchResult; // item and matched string
    bool _searchPerformed, _searchResultsChanged;
    int _currentIndex;
    FindResultsWidget *_resultsWidget;
    int _lastResultsHeight;

    void performSearch();
    void nothingFound(bool wasSearchDone = true);

    void loadSettings();
    void updateWindowTitle();
    void changeItem(bool changeResultsSelection = true);
    void setButtonsDisabled(bool disabled, bool updateResultButton = true);
};

#endif // FINDITEMSWIDGET_H
