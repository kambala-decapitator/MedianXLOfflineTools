#include "finditemsdialog.h"
#include "itemdatabase.h"
#include "findresultsdialog.h"
#include "propertiesdisplaymanager.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QLineEdit>

#include <QRegExp>
#include <QSettings>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


FindItemsDialog::FindItemsDialog(QWidget *parent) : QDialog(parent), _searchPerformed(false)
{
    ui.setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    updateWindowTitle();

    QGridLayout *checkboxGrid = new QGridLayout;
    checkboxGrid->addWidget(ui.caseSensitiveCheckBox, 0, 0);
    checkboxGrid->addWidget(ui.exactMatchCheckBox, 0, 1);
    checkboxGrid->addWidget(ui.wrapAroundCheckBox, 1, 0);
    checkboxGrid->addWidget(ui.regexCheckBox, 1, 1);
    checkboxGrid->addWidget(ui.searchPropsCheckBox, 2, 0);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(ui.nextButton);
    vbox->addWidget(ui.previousButton);
    vbox->addWidget(ui.searchResultsButton);
    vbox->addStretch();
    vbox->addWidget(ui.closeButton);

    QGridLayout *mainGrid = new QGridLayout(this);
    mainGrid->addWidget(ui.searchComboBox, 0, 0, Qt::AlignTop);
    mainGrid->addLayout(checkboxGrid, 1, 0);
    mainGrid->addLayout(vbox, 0, 1, 2, 1);

    setFixedHeight(height());

    connect(ui.nextButton, SIGNAL(clicked()), SLOT(findNext()));
    connect(ui.previousButton, SIGNAL(clicked()), SLOT(findPrevious()));
    connect(ui.searchResultsButton, SIGNAL(clicked()), SLOT(showResults()));
    connect(ui.searchComboBox, SIGNAL(editTextChanged(const QString &)), SLOT(searchTextChanged()));

    connect(ui.caseSensitiveCheckBox, SIGNAL(toggled(bool)), SLOT(resetSearchStatus()));
    connect(ui.exactMatchCheckBox, SIGNAL(toggled(bool)), SLOT(resetSearchStatus()));
    connect(ui.regexCheckBox, SIGNAL(toggled(bool)), SLOT(resetSearchStatus()));
    connect(ui.searchPropsCheckBox, SIGNAL(toggled(bool)), SLOT(resetSearchStatus()));

    loadSettings();
}

void FindItemsDialog::activateWindow()
{
    QDialog::activateWindow();
    ui.searchComboBox->lineEdit()->selectAll();
}

void FindItemsDialog::findNext()
{
    if (_searchPerformed)
    {
        if (_searchResult.size())
        {
            if (_currentIndex + 1 == _searchResult.size())
            {
                if (ui.wrapAroundCheckBox->isChecked())
                {
                    _currentIndex = -1;
                    qApp->beep();
                }
                else
                {
                    WARNING_BOX(tr("No more items found"));
                    return;
                }
            }
            ++_currentIndex;
            changeItem();
        }
        else
        {
            nothingFound();
        }
    }
    else
    {
        performSearch();
        _currentIndex = -1;
        findNext(); // show the first result
    }
}

void FindItemsDialog::findPrevious()
{
    if (_searchPerformed)
    {
        if (_searchResult.size())
        {
            if (_currentIndex - 1 == -1)
            {
                if (ui.wrapAroundCheckBox->isChecked())
                {
                    _currentIndex = _searchResult.size() - 1;
                    qApp->beep();
                }
                else
                {
                    WARNING_BOX(tr("No more items found"));
                    return;
                }
            }
            --_currentIndex;
            changeItem();
        }
        else
        {
            nothingFound();
        }
    }
    else
    {
        performSearch();
        _currentIndex = _searchResult.size();
        findPrevious(); // show the first result
    }
}

void FindItemsDialog::showResults()
{
    if (!_resultsDialog)
    {
        _resultsDialog = new FindResultsDialog(&_searchResult, this);
        _resultsDialog->show();
        _resultsDialog->selectItem(_searchResult[_currentIndex]);
        connect(_resultsDialog, SIGNAL(showItem(ItemInfo *)), SLOT(updateCurrentIndexForItem(ItemInfo *)));
    }
    _resultsDialog->activateWindow();
}

void FindItemsDialog::updateCurrentIndexForItem(ItemInfo *item)
{
    _currentIndex = _searchResult.indexOf(item);
    changeItem(false);
}

void FindItemsDialog::searchTextChanged()
{
    resetSearchStatus();
    updateWindowTitle();
}

void FindItemsDialog::performSearch()
{
    QString searchText = ui.searchComboBox->currentText();
    _searchResult.clear();
    foreach (ItemInfo *item, *ItemDataBase::currentCharacterItems)
    {
        QString itemText = ui.searchPropsCheckBox->isChecked() ? PropertiesDisplayManager::completeItemDescription(item) : ItemDataBase::completeItemName(item, false);
        Qt::CaseSensitivity cs = static_cast<Qt::CaseSensitivity>(ui.caseSensitiveCheckBox->isChecked());
        if (ui.regexCheckBox->isChecked())
        {
            QRegExp rx(searchText, cs);
            if (ui.exactMatchCheckBox->isChecked())
            {
                if (rx.exactMatch(itemText))
                    _searchResult += item;
            }
            else
            {
                if (rx.indexIn(itemText) != -1)
                    _searchResult += item;
            }
        }
        else
        {
            if (ui.exactMatchCheckBox->isChecked())
            {
                if (!itemText.compare(searchText, cs))
                    _searchResult += item;
            }
            else
            {
                if (itemText.contains(searchText, cs))
                    _searchResult += item;
            }
        }
    }

    _searchPerformed = true;
    ui.searchResultsButton->setEnabled(!_searchResult.isEmpty());

    if (_resultsDialog)
        _resultsDialog->updateItems(&_searchResult);

    // move the search string to the top of the last searches list if it is present there and not on the top
    if (ui.searchComboBox->currentIndex() > 0)
    {
        QStringList history;
        for (int i = 0; i < ui.searchComboBox->count(); ++i)
            history += ui.searchComboBox->itemText(i);
        history.move(ui.searchComboBox->currentIndex(), 0);

        ui.searchComboBox->clear();
        ui.searchComboBox->addItems(history);
    }
}

void FindItemsDialog::nothingFound()
{
    emit itemFound(0);
    setButtonsDisabled(true);

    if (_resultsDialog)
        _resultsDialog->updateItems(&_searchResult);

    ERROR_BOX(tr("No items found"));
}

void FindItemsDialog::loadSettings()
{
    QSettings settings;
    settings.beginGroup("findDialog");
    restoreGeometry(settings.value("geometry").toByteArray());
    ui.searchComboBox->addItems(settings.value("searchHistory").toStringList());
    ui.caseSensitiveCheckBox->setChecked(settings.value("caseSensitive").toBool());
    ui.exactMatchCheckBox->setChecked(settings.value("exactMatch").toBool());
    ui.wrapAroundCheckBox->setChecked(settings.value("wrapAround", true).toBool());
    ui.regexCheckBox->setChecked(settings.value("regex").toBool());
    ui.searchPropsCheckBox->setChecked(settings.value("searchProps").toBool());
    settings.endGroup();
}

void FindItemsDialog::saveSettings()
{
    QStringList history; // 10 strings max
    for (int i = 0; i < ui.searchComboBox->count() && i < 10; ++i)
        history += ui.searchComboBox->itemText(i);

    QSettings settings;
    settings.beginGroup("findDialog");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("searchHistory", history);
    settings.setValue("caseSensitive", ui.caseSensitiveCheckBox->isChecked());
    settings.setValue("exactMatch", ui.exactMatchCheckBox->isChecked());
    settings.setValue("wrapAround", ui.wrapAroundCheckBox->isChecked());
    settings.setValue("regex", ui.regexCheckBox->isChecked());
    settings.setValue("searchProps", ui.searchPropsCheckBox->isChecked());
    settings.endGroup();

    if (_resultsDialog)
        _resultsDialog->saveSettings();
}

void FindItemsDialog::show()
{
    QDialog::show();

    if (ui.searchComboBox->currentIndex() == -1 || ui.searchComboBox->currentText().isEmpty())
        ui.searchComboBox->setCurrentIndex(0);
}

void FindItemsDialog::updateWindowTitle()
{
    QString title = tr("Find items");
    if (_searchPerformed)
        title += QString(" [%1/%2]").arg(_currentIndex + 1).arg(_searchResult.size());
    setWindowTitle(title);
}

void FindItemsDialog::changeItem(bool changeResultsSelection /*= true*/)
{
    ItemInfo *item = _searchResult[_currentIndex];
    emit itemFound(item);
    updateWindowTitle();

    if (changeResultsSelection && _resultsDialog)
        _resultsDialog->selectItem(item);
}

void FindItemsDialog::setButtonsDisabled(bool disabled, bool updateResultButton /*= true*/)
{
    ui.nextButton->setDisabled(disabled);
    ui.previousButton->setDisabled(disabled);
    if (updateResultButton)
        ui.searchResultsButton->setDisabled(disabled);
}

void FindItemsDialog::resetSearchStatus()
{
    _searchPerformed = false;
    setButtonsDisabled(ui.searchComboBox->currentText().isEmpty(), false);
}
