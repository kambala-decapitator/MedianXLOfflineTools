#ifndef FINDITEMSWIDGET_H
#define FINDITEMSWIDGET_H

#include "ui_finditemsdialog.h"

#include "structs.h"

#include <QDialog>
#include <QPointer>


class FindResultsDialog;

class FindItemsDialog : public QDialog
{
    Q_OBJECT

public:
    Ui::FindItemsDialog ui;

    FindItemsDialog(QWidget *parent = 0);

    void saveSettings();

public slots:
    void resetSearchStatus();
    void show();

signals:
    void itemFound(ItemInfo *item);

private slots:
    void findNext();
    void findPrevious();
    void showResults();
    void updateCurrentIndexForItem(ItemInfo *item);

    void searchTextChanged();

private:
    ItemsList _searchResult;
    bool _searchPerformed;
    int _currentIndex;
    QPointer<FindResultsDialog> _resultsDialog;

    void performSearch();
    void nothingFound();

    void loadSettings();
    void updateWindowTitle();
    void changeItem(bool changeResultsSelection = true);
    void setButtonsDisabled(bool disabled, bool updateResultButton = true);
};

#endif // FINDITEMSWIDGET_H
