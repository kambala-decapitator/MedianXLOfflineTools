#include    "finditemsdialog.h"
#include "ui_finditemsdialog.h"
#include "itemdatabase.h"
#include "propertiesdisplaymanager.h"
#include "structs.h"
#include "characterinfo.hpp"
#include "helpwindowdisplaymanager.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QLineEdit>
#include <QCompleter>

#include <QRegExp>
#include <QSettings>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif


bool compareSearchResultItemsByPlugyPage(const SearchResultItem &a, const SearchResultItem &b)
{
    return compareItemsByPlugyPage(a.first, b.first);
}


FindItemsDialog::FindItemsDialog(QWidget *parent) : QDialog(parent), ui(new Ui::FindItemsDialog), _resultsWidget(new FindResultsWidget(this)),
    _wasSearchPerformed(false), _searchResultsChanged(false), _lastResultsHeight(-1)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    updateWindowTitle();

    QGridLayout *checkboxGrid = new QGridLayout;
    checkboxGrid->addWidget(ui->caseSensitiveCheckBox, 0, 0);
    checkboxGrid->addWidget(ui->searchPropsCheckBox, 1, 0);
    checkboxGrid->addWidget(ui->wrapAroundCheckBox, 2, 0);
    checkboxGrid->addWidget(ui->regexCheckBox, 0, 1);
    checkboxGrid->addWidget(ui->minimalMatchCheckBox, 1, 1);
    checkboxGrid->addWidget(ui->multilineMatchCheckBox, 2, 1);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(ui->nextButton);
    vbox->addWidget(ui->previousButton);
    vbox->addWidget(ui->searchResultsButton);
    vbox->addStretch();
    vbox->addWidget(ui->buttonBox);

    QGridLayout *mainGrid = new QGridLayout;
    mainGrid->addWidget(ui->searchComboBox, 0, 0, Qt::AlignTop);
    mainGrid->addLayout(checkboxGrid, 1, 0);
    mainGrid->addLayout(vbox, 0, 1, 2, 1, Qt::AlignRight);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(mainGrid);
    mainLayout->addWidget(_resultsWidget);

    toggleResults();
    _resultsWidget->hide();

    ui->nextButton->setToolTip(QKeySequence(QKeySequence::FindNext).toString(QKeySequence::NativeText));
    ui->previousButton->setToolTip(QKeySequence(QKeySequence::FindPrevious).toString(QKeySequence::NativeText));

    connect(ui->nextButton, SIGNAL(clicked()), SLOT(findNext()));
    connect(ui->previousButton, SIGNAL(clicked()), SLOT(findPrevious()));
    connect(ui->searchResultsButton, SIGNAL(clicked()), SLOT(toggleResults()));
    connect(ui->searchComboBox, SIGNAL(editTextChanged(const QString &)), SLOT(searchTextChanged()));
    connect(_resultsWidget->selectItemDelegate, SIGNAL(showItem(ItemInfo *)), SLOT(updateCurrentIndexForItem(ItemInfo *)));

    QList<QCheckBox *> checkBoxes = QList<QCheckBox *>() << ui->caseSensitiveCheckBox << ui->minimalMatchCheckBox << ui->regexCheckBox << ui->multilineMatchCheckBox << ui->searchPropsCheckBox;
    foreach (QCheckBox *checkBox, checkBoxes)
        connect(checkBox, SIGNAL(toggled(bool)), SLOT(resetSearchStatus()));

    connect(ui->caseSensitiveCheckBox, SIGNAL(toggled(bool)), SLOT(changeComboboxCaseSensitivity(bool)));

    loadSettings();
    adjustSize();
    setMaximumHeight(qApp->desktop()->availableGeometry().height() - 50);

    _helpDislplayManager = new HelpWindowDisplayManager(tr("Search help"),
        tr(
        "<h3>Item names</h3>"
        "<p>All items except non-magical ones have their quality listed inside [] at the very beginning of item description. Valid values are: magic, rare, unique, set, crafted, honorific.</p>"
        "<p>Runewords and charms are considered a special type of quality, so they have [runeword] and [charm] respectively.</p>"
        "<p>Ethereal items also have [ethereal] in the end of the item name.</p>"
        "<p>Set items have complete set name listed inside [] after set item name.</p>"
        "<p>Personalized items have character name as it appears in game. The exception are items with affixes because affix display isn't supported in the current version of the application.</p>"
        "<p>To see an example of such an item description, simply hover your mouse upon any item in the items window and look at the tooltip.</p>"
        "<h3>Item properties</h3>"
        "<p>If the 'Search in properties' checkbox is checked, then the search is made not only by item name (as explained above), but also in item properties.</p>"
        "<p>Properties appear the same way as they do in the item description view. Diablo color codes are also present here to simplify search for e.g. elite reanimates.</p>"
        "<h3>Regular expressions</h3>"
        "<p>Regular expressions syntax is mostly Perl-compatible, but there're some limitations. "
        "Refer to the <a href=\"%1\">Qt regular expressions description</a> for more information.</p>"
        "<p>Regular expressions-only checkboxes in the dialog have tooltips on what they mean if it's not clear.</p>"
        "<p>Hint: enter . (period) as a search text to see all your items :)</p>"
        "<h3>Search results</h3>"
        "<p>Hovering upon an item in the search results drop-down will display matched line with an actual match highlighted in <b>bold</b>.</p>"
        "<p>Double-clicking or pressing Return/Enter on an item shows it in the items window.</p>"
        ).arg("http://qt-project.org/doc/qt-4.8/qregexp.html#details"), this);
    connect(ui->buttonBox, SIGNAL(helpRequested()), _helpDislplayManager, SLOT(showHelp()));
}

FindItemsDialog::~FindItemsDialog()
{
    delete ui;
}

void FindItemsDialog::clearResults()
{
    _searchResult.clear();
    searchTextChanged(); // text doesn't actually change here, I just don't want to create new method that only calls this one
    nothingFound(false);
}

void FindItemsDialog::sortAndUpdateSearchResult()
{
    qSort(_searchResult.begin(), _searchResult.end(), compareSearchResultItemsByPlugyPage);
    _resultsWidget->updateItems(&_searchResult);
}


void FindItemsDialog::resetSearchStatus()
{
    _wasSearchPerformed = false;
    setButtonsDisabled(ui->searchComboBox->currentText().isEmpty(), false);
}

void FindItemsDialog::reject()
{
    _helpDislplayManager->closeHelp();
    saveSettings();
    QDialog::reject();
}


void FindItemsDialog::findNext()
{
    if (_wasSearchPerformed)
    {
        if (!_searchResult.isEmpty())
        {
            if (_currentIndex + 1 == _searchResult.size())
            {
                if (ui->wrapAroundCheckBox->isChecked())
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
            nothingFound();
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
    if (_wasSearchPerformed)
    {
        if (!_searchResult.isEmpty())
        {
            if (_currentIndex - 1 == -1)
            {
                if (ui->wrapAroundCheckBox->isChecked())
                {
                    _currentIndex = _searchResult.size();
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
            nothingFound();
    }
    else
    {
        performSearch();
        _currentIndex = _searchResult.size();
        findPrevious(); // show the first result
    }
}

void FindItemsDialog::toggleResults()
{
    if (_resultsWidget->isVisible())
        _lastResultsHeight = _resultsWidget->height();
    _resultsWidget->setVisible(!_resultsWidget->isVisible());

    if (_resultsWidget->isVisible())
    {
        ui->searchResultsButton->setText(tr("Hide results"));

        QSize oldSize = size();
        int newHeight = oldSize.height() + (_lastResultsHeight == -1 ? _resultsWidget->sizeHint().height() : _lastResultsHeight);
        setMinimumHeight(minimumHeight() + _resultsWidget->minimumHeight()
#ifdef Q_OS_MAC
                        + 35 // dialog becomes smaller without this for some reason
#endif
                         );
        resize(oldSize.width(), newHeight);

        if (_searchResultsChanged)
        {
            _resultsWidget->updateItems(&_searchResult);
            _resultsWidget->selectItem(_searchResult[_currentIndex].first);
            _searchResultsChanged = false;
        }
    }
    else
    {
        ui->searchResultsButton->setText(tr("Show results"));

        QSize oldSize = size();
        int newHeight = oldSize.height() - _lastResultsHeight;
        setMinimumHeight(newHeight);
        resize(oldSize.width(), newHeight);
    }
}

void FindItemsDialog::updateCurrentIndexForItem(ItemInfo *item)
{
    _currentIndex = 0;
    foreach (const SearchResultItem &foundItem, _searchResult)
    {
        if (foundItem.first == item)
            break;
        ++_currentIndex;
    }
    changeItem(false);
}

void FindItemsDialog::searchTextChanged()
{
    resetSearchStatus();
    updateWindowTitle();
}

void FindItemsDialog::changeComboboxCaseSensitivity(bool isCaseSensitive)
{
    ui->searchComboBox->completer()->setCaseSensitivity(isCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
}

void FindItemsDialog::performSearch()
{
    QString searchText = ui->searchComboBox->currentText();
    _searchResult.clear();
    foreach (ItemInfo *item, CharacterInfo::instance().items.character) //-V807
    {
        QString itemText = ui->searchPropsCheckBox->isChecked() ? PropertiesDisplayManager::completeItemDescription(item) : ItemDataBase::completeItemName(item, false);
        Qt::CaseSensitivity cs = static_cast<Qt::CaseSensitivity>(ui->caseSensitiveCheckBox->isChecked());
        if (ui->regexCheckBox->isChecked())
        {
            QRegExp rx(searchText, cs, QRegExp::RegExp2);
            rx.setMinimal(ui->minimalMatchCheckBox->isChecked());
            if (ui->multilineMatchCheckBox->isChecked())
            {
                int matchIndex = rx.indexIn(itemText);
                if (matchIndex != -1)
                {
                    int matchLength = rx.cap().length(), previousLineBreak = itemText.lastIndexOf("\n", matchIndex) + 1, nextLineBreak = itemText.indexOf("\n", matchIndex + matchLength);
                    QString matchedLine = nextLineBreak != -1 ? itemText.mid(previousLineBreak, nextLineBreak - previousLineBreak) : itemText.mid(previousLineBreak);
                    matchIndex = rx.indexIn(matchedLine);
                    matchedLine.insert(matchIndex, "<b>");
                    matchedLine.insert(matchIndex + matchLength + 3, "</b>");
                    matchedLine.replace("\n", kHtmlLineBreak);
                    _searchResult += qMakePair(item, matchedLine);
                }
            }
            else
            {
                QStringList lines = itemText.split("\n");
                foreach (QString line, lines)
                {
                    int matchIndex = rx.indexIn(line);
                    if (matchIndex != -1)
                    {
                        line.insert(matchIndex, "<b>");
                        line.insert(matchIndex + rx.cap().length() + 3, "</b>");
                        _searchResult += qMakePair(item, line);
                    }
                }
            }
        }
        else
        {
            int matchIndex = itemText.indexOf(searchText, 0, cs);
            if (matchIndex != -1)
            {
                int previousLineBreak = itemText.lastIndexOf("\n", matchIndex) + 1, nextLineBreak = itemText.indexOf("\n", matchIndex + searchText.length());
                QString matchedLine = nextLineBreak != -1 ? itemText.mid(previousLineBreak, nextLineBreak - previousLineBreak) : itemText.mid(previousLineBreak);
                matchIndex = matchedLine.indexOf(searchText, 0, cs);
                matchedLine.insert(matchIndex, "<b>");
                matchedLine.insert(matchIndex + searchText.length() + 3, "</b>");
                _searchResult += qMakePair(item, matchedLine);
            }
        }
    }

    _wasSearchPerformed = _searchResultsChanged = true;
    ui->searchResultsButton->setEnabled(!_searchResult.isEmpty());
    sortAndUpdateSearchResult();

    // search text isn't added if a user presses find next/previous button directly
    if (ui->searchComboBox->findText(searchText) == -1)
    {
        ui->searchComboBox->insertItem(0, searchText);
        ui->searchComboBox->setCurrentIndex(0);
    }

    // move the search string to the top of the last searches list if it is present there and not on the top
    if (ui->searchComboBox->currentIndex() > 0)
    {
        QStringList history;
        for (int i = 0; i < ui->searchComboBox->count(); ++i)
            history += ui->searchComboBox->itemText(i);
        history.move(ui->searchComboBox->currentIndex(), 0);

        ui->searchComboBox->clear();
        ui->searchComboBox->addItems(history);
    }
}

void FindItemsDialog::nothingFound(bool wasSearchDone /*= true*/)
{
    emit itemFound(0);

    if (wasSearchDone)
        setButtonsDisabled(true);

    _resultsWidget->updateItems(&_searchResult);
    if (_resultsWidget->isVisible())
        toggleResults();

    if (wasSearchDone)
        ERROR_BOX(tr("No items found"));
}

void FindItemsDialog::loadSettings()
{
    QSettings settings;
    settings.beginGroup("findDialog");
    if (settings.contains("pos"))
        move(settings.value("pos").toPoint());
    ui->searchComboBox->addItems(settings.value("searchHistory").toStringList());
    ui->caseSensitiveCheckBox->setChecked(settings.value("caseSensitive").toBool());
    ui->minimalMatchCheckBox->setChecked(settings.value("regexMinimalMatch").toBool());
    ui->multilineMatchCheckBox->setChecked(settings.value("regexMultilineMatch").toBool());
    ui->regexCheckBox->setChecked(settings.value("regex").toBool());
    ui->searchPropsCheckBox->setChecked(settings.value("searchProps").toBool());
    ui->wrapAroundCheckBox->setChecked(settings.value("wrapAround", true).toBool());
    settings.endGroup();
}

void FindItemsDialog::saveSettings()
{
    QStringList history; // save 10 strings max
    for (int i = 0; i < ui->searchComboBox->count() && i < 10; ++i)
        history += ui->searchComboBox->itemText(i);

    QSettings settings;
    settings.beginGroup("findDialog");
    settings.setValue("pos", pos());
    settings.setValue("searchHistory", history);
    settings.setValue("caseSensitive", ui->caseSensitiveCheckBox->isChecked());
    settings.setValue("regexMinimalMatch", ui->minimalMatchCheckBox->isChecked());
    settings.setValue("regexMultilineMatch", ui->multilineMatchCheckBox->isChecked());
    settings.setValue("regex", ui->regexCheckBox->isChecked());
    settings.setValue("searchProps", ui->searchPropsCheckBox->isChecked());
    settings.setValue("wrapAround", ui->wrapAroundCheckBox->isChecked());
    settings.endGroup();
}

void FindItemsDialog::updateWindowTitle()
{
    QString title = tr("Find items");
    if (_wasSearchPerformed)
        title += QString(" [%1/%2]").arg(_currentIndex + 1).arg(_searchResult.size());
    setWindowTitle(title);
}

void FindItemsDialog::changeItem(bool changeResultsSelection /*= true*/)
{
    ItemInfo *item = _searchResult[_currentIndex].first;
    emit itemFound(item);
    updateWindowTitle();

    if (changeResultsSelection && _resultsWidget)
        _resultsWidget->selectItem(item);
}

void FindItemsDialog::setButtonsDisabled(bool disabled, bool updateResultButton /*= true*/)
{
    ui->nextButton->setDisabled(disabled);
    ui->previousButton->setDisabled(disabled);
    if (updateResultButton)
        ui->searchResultsButton->setDisabled(disabled);
}


void FindItemsDialog::showEvent(QShowEvent *e)
{
    Q_UNUSED(e);

    if (ui->searchComboBox->currentIndex() == -1 || ui->searchComboBox->currentText().isEmpty())
        ui->searchComboBox->setCurrentIndex(0);
    ui->searchComboBox->lineEdit()->selectAll();
}
