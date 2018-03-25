#include "dupescandialog.h"
#include "characterinfo.hpp"
#include "itemparser.h"
#include "itemdatabase.h"
#include "enums.h"

#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QCheckBox>
#include <QProgressBar>
#include <QApplication>

#include <QDir>
#include <QFile>
#include <QRegExp>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif

#if IS_QT5
#include <QtConcurrent/QtConcurrentRun>
#include <QtConcurrent/QtConcurrentMap>
#else
#include <QtConcurrentRun>
#include <QtConcurrentMap>
#endif


bool shouldCheckItem(ItemInfo *item)
{
    // ignore tomes, keys and non-magical quivers
    return !(ItemDataBase::isTomeWithScrolls(item) || item->itemType == "key" || ((item->itemType == "aqv" || item->itemType == "cqv") && item->quality < Enums::ItemQuality::Magic));
}

bool areItemsSame(ItemInfo *a, ItemInfo *b)
{
    return a->isExtended && a->guid == b->guid && a->itemType == b->itemType;
}

QString itemString(ItemInfo *item)
{
    return QString("<b>%1</b>: GUID 0x%2 (%3), type '%4', quality <b>%5</b>").arg(ItemDataBase::Items()->value(item->itemType)->name)
            .arg(item->guid, 0, 16).arg(item->guid).arg(item->itemType.constData()).arg(metaEnumFromName<Enums::ItemQuality>("ItemQualityEnum").valueToKey(item->quality));
}

QString dupedItemsStr(ItemInfo *item1, ItemInfo *item2)
{
    return itemString(item1) + QString("; %6; %7")
            .arg(ItemParser::itemStorageAndCoordinatesString("<font color=blue>ITEM1</font>: location %1, row %2, col %3, equipped in %4", item1))
            .arg(ItemParser::itemStorageAndCoordinatesString("<font color=blue>ITEM2</font>: location %1, row %2, col %3, equipped in %4", item2));
}


struct CrossCompareTask
{
    bool skipEmptyResults;
    ItemsHashIterator frontBegin, backBegin, end;
    CrossCompareTask(bool skip, ItemsHashIterator endIter) : skipEmptyResults(skip), end(endIter) {}
};

QString crossCompareItems(const CrossCompareTask &task)
{
    bool isFirst, dupedItemFound;
    QString result;
    foreach (ItemsHashIterator iter, QList<ItemsHashIterator>() << task.frontBegin << task.backBegin)
    {
        dupedItemFound = false;
        if (!task.skipEmptyResults)
            result += "<br>" + iter.key();

        const ItemsList &iItems = iter.value();
        for (ItemsHashIterator jter = iter + 1; jter != task.end; ++jter)
        {
            isFirst = true;
            foreach (ItemInfo *iItem, iItems)
            {
                if (iItem->isExtended && shouldCheckItem(iItem))
                {
                    foreach (ItemInfo *jItem, jter.value())
                    {
                        if (areItemsSame(jItem, iItem))
                        {
                            if (!dupedItemFound && task.skipEmptyResults)
                            {
                                result += iter.key();
                                dupedItemFound = true;
                            }
                            if (isFirst)
                            {
                                result += "<br><br>" + jter.key() + ":";
                                isFirst = false;
                            }
                            result += "<br>" + dupedItemsStr(iItem, jItem);
                        }
                    }
                }
            }
        }

        if (!task.skipEmptyResults || dupedItemFound)
            result += "<br>========================================";
    }
    return result;
}


DupeScanDialog::DupeScanDialog(const QString &currentPath, bool isDumpItemsMode, QWidget *parent) : QDialog(parent), _currentCharPath(currentPath), _isDumpItemsMode(isDumpItemsMode), _futureWatcher(0),
    _pathLineEdit(new QLineEdit(this)), _logBrowser(new QTextEdit(this)), _saveButton(new QPushButton("Save...", this)), _skipEmptyCheckBox(new QCheckBox("Skip empty results", this)), _progressBar(new QProgressBar(this))
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(isDumpItemsMode ? "Dump Items" : "Dupe Scanner");

    QLabel *label = new QLabel("Characters path:", this);
    QPushButton *browseButton = new QPushButton("Browse...", this);
    QHBoxLayout *hbl = new QHBoxLayout;
    hbl->addWidget(label);
    hbl->addWidget(_pathLineEdit);
    hbl->addWidget(browseButton);

    QPushButton *scanButton = new QPushButton("Scan!", this), *okButton = new QPushButton("OK", this);
    QHBoxLayout *hbl2 = new QHBoxLayout;
    hbl2->addWidget(scanButton);
    hbl2->addWidget(_saveButton);
    hbl2->addWidget(_skipEmptyCheckBox);
    hbl2->addWidget(_progressBar);
    hbl2->addWidget(okButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(hbl);
    mainLayout->addWidget(_logBrowser);
    mainLayout->addLayout(hbl2);

    _pathLineEdit->setText(QDir::toNativeSeparators(QFileInfo(currentPath).canonicalPath()));
    _logBrowser->setReadOnly(true);
    _saveButton->setDisabled(true);
    scanButton->setDefault(true);
    _skipEmptyCheckBox->setChecked(true);
    _progressBar->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    resize(600, 400);

    connect(browseButton, SIGNAL(clicked()), SLOT(selectPath()));
    connect(scanButton,   SIGNAL(clicked()), SLOT(scan()));
    connect(_saveButton,  SIGNAL(clicked()), SLOT(save()));
    connect(okButton,     SIGNAL(clicked()), SLOT(accept()));

    if (!isDumpItemsMode)
    {
        _futureWatcher = new QFutureWatcher<QString>;
        connect(_futureWatcher, SIGNAL(progressRangeChanged(int,int)), _progressBar, SLOT(setRange(int,int)));
        connect(_futureWatcher, SIGNAL(progressValueChanged(int)), _progressBar, SLOT(setValue(int)));
        connect(_futureWatcher, SIGNAL(resultReadyAt(int)), SLOT(crossCheckResultReady(int)));
        connect(_futureWatcher, SIGNAL(finished()), SLOT(scanFinished()));
    }
}

DupeScanDialog::~DupeScanDialog()
{
    foreach (const ItemsList &items, _allItemsHash)
        qDeleteAll(items);
}

void DupeScanDialog::done(int r)
{
    if (_futureWatcher)
    {
        _futureWatcher->cancel();
        _futureWatcher->waitForFinished();
    }

    parentWidget()->setUpdatesEnabled(true);
    QDialog::done(r);
}

void DupeScanDialog::selectPath()
{
    QString path = QFileDialog::getExistingDirectory(this, "Select path to scan", _pathLineEdit->text());
    if (!path.isEmpty())
        _pathLineEdit->setText(QDir::toNativeSeparators(path));
}

void DupeScanDialog::scan()
{
    QString path = _pathLineEdit->text();
    if (path.isEmpty())
    {
        selectPath();
        path = _pathLineEdit->text();
        if (path.isEmpty())
            return;
    }

    _logBrowser->clear();
    _logBrowser->setUpdatesEnabled(false);
    _progressBar->setMinimum(0);
    _progressBar->setFormat("%v / %m separate files processed");
    _saveButton->setDisabled(true);

    parentWidget()->setUpdatesEnabled(false);

    _timeCounter.start();
    QtConcurrent::run(this, &DupeScanDialog::scanCharactersInDir, path);
}

void DupeScanDialog::scanFinished()
{
    if (!_isDumpItemsMode)
    {
        foreach (const ItemsList &items, _allItemsHash)
            qDeleteAll(items);
        _allItemsHash.clear();

        _logBrowser->append(QString("<font color=black>processing took %1 seconds in total</font>").arg(_timeCounter.elapsed() / 1000));
    }

    parentWidget()->setUpdatesEnabled(true);
    _logBrowser->setUpdatesEnabled(true);

    _saveButton->setEnabled(true);
    qApp->alert(parentWidget());
}

void DupeScanDialog::save()
{
    QString plainTextFilter = "plain text (*.txt)", htmlTextFilter = "HTML (*.html)";
    QString selectedFilter, fileName = QFileDialog::getSaveFileName(this, "Save dupe report", QFileInfo(_currentCharPath).canonicalPath() + "/MXLOT dupe stats",
                                                                    plainTextFilter + ";;" + htmlTextFilter, &selectedFilter);
    if (fileName.isEmpty())
        return;

    QRegExp re("\\(\\*(.+)\\)");
    re.indexIn(selectedFilter);
    QString extension = re.cap(1);

    if (!fileName.endsWith(extension))
        fileName += extension;
    QFile f(fileName);
    if (f.open(QIODevice::WriteOnly))
        f.write((selectedFilter == plainTextFilter ? _logBrowser->toPlainText() : _logBrowser->toHtml()).toUtf8());
    else
        ERROR_BOX("failed to save file");
}

void DupeScanDialog::crossCheckResultReady(int i)
{
    QString result = _futureWatcher->future().resultAt(i);
    if (!result.isEmpty())
        appendStringToLog(result);
}

void DupeScanDialog::appendStringToLog(const QString &s)
{
    QMetaObject::invokeMethod(_logBrowser, "append", Q_ARG(QString, s));
}

void DupeScanDialog::scanCharactersInDir(const QString &path)
{
    if (!_isDumpItemsMode)
        appendStringToLog("<h3>SEPARATE CHARACTER CHECK</h3>----------------------------------------");

    bool isFirst, dupedItemFound;
    QString pathWithSlashes = QDir::fromNativeSeparators(_currentCharPath);

    QFileInfoList list = QDir(path).entryInfoList(QDir::Files);
    if ((isFirst = !_currentCharPath.isEmpty()))
        list.prepend(QFileInfo(_currentCharPath)); // for currently loaded file
    QMetaObject::invokeMethod(_progressBar, "setMaximum", Q_ARG(int, list.size()));

    int filesProcessed = 0;
    foreach (const QFileInfo &fileInfo, list)
    {
        ++filesProcessed;

        QString fileName = fileInfo.fileName(), header = fileName + (_isDumpItemsMode ? " items" : " dupe stats");
        qDebug("loading %s", qPrintable(fileName));
        if (isFirst)
        {
            header += " (currently loaded)";
            isFirst = false;
        }
        else
        {
            QString path = fileInfo.canonicalFilePath();
            if (path == pathWithSlashes || (!fileInfo.suffix().isEmpty() && fileInfo.suffix() != "d2s"))
                continue;

            emit loadFile(path);
        }
        if (!_skipEmptyCheckBox->isChecked() || !_loadingMessage.isEmpty())
        {
            appendStringToLog(header + "\n");
            if (!_loadingMessage.isEmpty())
                appendStringToLog(_loadingMessage);
        }

        dupedItemFound = false;
        if (_isDumpItemsMode)
        {
            foreach (ItemInfo *item, CharacterInfo::instance().items.character)
                if (shouldCheckItem(item))
                    appendStringToLog(itemString(item));
        }
        else
        {
            ItemsList itemsAndSocketables = CharacterInfo::instance().items.character, checkedItems;
            for (int i = 0; i < itemsAndSocketables.size() - 1; ++i)
            {
                ItemInfo *iItem = itemsAndSocketables.at(i);
                if (iItem->isExtended && shouldCheckItem(iItem))
                {
                    itemsAndSocketables << iItem->socketablesInfo;

                    bool alreadyChecked = false;
                    foreach (ItemInfo *item, checkedItems)
                    {
                        if (iItem->guid == item->guid)
                        {
                            alreadyChecked = true;
                            break;
                        }
                    }
                    if (alreadyChecked)
                        continue;

                    for (int j = i + 1; j < itemsAndSocketables.size(); ++j)
                    {
                        ItemInfo *jItem = itemsAndSocketables.at(j);
                        if (areItemsSame(jItem, iItem))
                        {
                            checkedItems += iItem;
                            if (!dupedItemFound && _skipEmptyCheckBox->isChecked())
                            {
                                appendStringToLog(header + "\n");
                                dupedItemFound = true;
                            }
                            appendStringToLog(dupedItemsStr(iItem, jItem));
                        }
                    }
                }
            }

            ItemsList itemsCopy;
            foreach (ItemInfo *item, itemsAndSocketables)
            {
                ItemInfo *itemCopy = new ItemInfo(*item);
                itemCopy->shouldDeleteEverything = false; // because it's a shallow copy
                itemsCopy += itemCopy;
            }
            _allItemsHash[fileName] = itemsCopy;
        }

        if (!_skipEmptyCheckBox->isChecked() || dupedItemFound || !_loadingMessage.isEmpty())
            appendStringToLog("========================================");

        _loadingMessage.clear();
        QMetaObject::invokeMethod(_progressBar, "setValue", Q_ARG(int, filesProcessed));
    }

    appendStringToLog(QString("loading files & separate processing took %1 seconds").arg(_timeCounter.elapsed() / 1000));
    if (_isDumpItemsMode)
    {
        scanFinished();
        return;
    }

    appendStringToLog("<br><h3>CROSS-CHARACTER CHECK</h3>----------------------------------------");
    _progressBar->setFormat("cross-check %p%");

    QList<CrossCompareTask> tasks;
    CrossCompareTask task(_skipEmptyCheckBox->isChecked(), _allItemsHash.constEnd());
    ItemsHashIterator begin = _allItemsHash.constBegin();
    for (int i = 0, n = _allItemsHash.size() / 2 + _allItemsHash.size() % 2; i < n; ++i)
    {
        task.frontBegin = begin + i;

        ItemsHashIterator beginFromEndIter = task.end - 1 - i;
        task.backBegin = beginFromEndIter != task.frontBegin ? beginFromEndIter : task.end - 1;

        tasks << task;
    }
    _futureWatcher->setFuture(QtConcurrent::mapped(tasks, crossCompareItems));
}
