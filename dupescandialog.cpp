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
#include <QTimer>

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

QString dupedItemsStr(ItemInfo *item1, ItemInfo *item2)
{
    return QString("<b>%1</b>: GUID 0x%2 (%3), type '%4', quality <b>%5</b>; %6; %7").arg(ItemDataBase::Items()->value(item1->itemType)->name)
            .arg(item1->guid, 0, 16).arg(item1->guid).arg(item1->itemType.constData()).arg(metaEnumFromName<Enums::ItemQuality>("ItemQualityEnum").valueToKey(item1->quality))
            .arg(ItemParser::itemStorageAndCoordinatesString("<font color=blue>ITEM1</font>: location %1, row %2, col %3, equipped in %4", item1))
            .arg(ItemParser::itemStorageAndCoordinatesString("<font color=blue>ITEM2</font>: location %1, row %2, col %3, equipped in %4", item2));
}

void addItemName(ItemInfo *item, QString &itemsStr)
{
    static QRegExp runeRegex("r(\\d\\d)");
    if ((item->isExtended && shouldCheckItem(item)) || (runeRegex.indexIn(item->itemType) != -1 && runeRegex.cap(1).toInt() > 50)) // also save Great/Elemental runes
        itemsStr += ItemDataBase::completeItemName(item, false, false).replace(kHtmlLineBreak, QLatin1String("|")) + "\n";
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
    hbl2->addWidget(_progressBar);
    hbl2->addWidget(okButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(hbl);
    mainLayout->addWidget(_logBrowser);
    mainLayout->addLayout(hbl2);

    QStringList args = qApp->arguments();
    _isAutoLaunched = args.size() == 3;
    QString path = _isAutoLaunched ? args.last() : QFileInfo(currentPath).canonicalPath();
    _pathLineEdit->setText(QDir::toNativeSeparators(path));

    _logBrowser->setReadOnly(true);
    _saveButton->setDisabled(true);
    scanButton->setDefault(true);
    _progressBar->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    resize(600, 400);

    connect(browseButton, SIGNAL(clicked()), SLOT(selectPath()));
    connect(scanButton,   SIGNAL(clicked()), SLOT(scan()));
    connect(_saveButton,  SIGNAL(clicked()), SLOT(save()));
    connect(okButton,     SIGNAL(clicked()), SLOT(accept()));

    if (!isDumpItemsMode)
    {
        hbl2->insertWidget(2, _skipEmptyCheckBox);
        _skipEmptyCheckBox->setChecked(true);

        _futureWatcher = new QFutureWatcher<QString>;
        connect(_futureWatcher, SIGNAL(progressRangeChanged(int,int)), _progressBar, SLOT(setRange(int,int)));
        connect(_futureWatcher, SIGNAL(progressValueChanged(int)), _progressBar, SLOT(setValue(int)));
        connect(_futureWatcher, SIGNAL(resultReadyAt(int)), SLOT(crossCheckResultReady(int)));
        connect(_futureWatcher, SIGNAL(finished()), SLOT(scanFinished_()));
    }
    else
        _skipEmptyCheckBox->hide();

    if (_isAutoLaunched)
        QTimer::singleShot(0, this, SLOT(scan()));
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
    _progressBar->setMinimum(0);
    _progressBar->setFormat("%v / %m separate files processed");
    _saveButton->setDisabled(true);

    if (!_isDumpItemsMode)
    {
        _logBrowser->setUpdatesEnabled(false);
        parentWidget()->setUpdatesEnabled(false);
    }

    _timeCounter.start();
    QtConcurrent::run(this, &DupeScanDialog::scanCharactersInDir, path);
}

void DupeScanDialog::scanFinished_()
{
    if (!_isDumpItemsMode)
    {
        foreach (const ItemsList &items, _allItemsHash)
            qDeleteAll(items);
        _allItemsHash.clear();

        _logBrowser->append(QString("<font color=black>processing took %1 seconds in total</font>").arg(_timeCounter.elapsed() / 1000));
        _logBrowser->setUpdatesEnabled(true);
        parentWidget()->setUpdatesEnabled(true);
    }

    _saveButton->setEnabled(true);

    if (_isAutoLaunched)
    {
        if (!_isDumpItemsMode && !saveLog(baseDupeScanLogFileName() + QString("_%1.txt").arg(QDateTime::currentMSecsSinceEpoch())))
            qDebug("couldn't save log file");
        qApp->quit();
    }
    else
        emit scanFinished();
}

void DupeScanDialog::save()
{
    QString plainTextFilter = "plain text (*.txt)", htmlTextFilter = "HTML (*.html)";
    QString selectedFilter, fileName = QFileDialog::getSaveFileName(this, "Save dupe report", baseDupeScanLogFileName(),
                                                                    plainTextFilter + ";;" + htmlTextFilter, &selectedFilter);
    if (fileName.isEmpty())
        return;

    QRegExp re("\\(\\*(.+)\\)");
    re.indexIn(selectedFilter);
    QString extension = re.cap(1);

    if (!fileName.endsWith(extension))
        fileName += extension;
    QFile f(fileName);
    if (!saveLog(fileName, selectedFilter == plainTextFilter))
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

        QString fileName = fileInfo.fileName(), header = _isDumpItemsMode ? ("processing " + fileName) : (fileName + " dupe stats");
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
            {
                QMetaObject::invokeMethod(_progressBar, "setValue", Q_ARG(int, filesProcessed));
                continue;
            }

            emit loadFile(path);
        }
        if (!_skipEmptyCheckBox->isChecked() || !_loadingMessage.isEmpty())
        {
            appendStringToLog(header);
            if (!_isDumpItemsMode)
                appendStringToLog("\n");
            if (!_loadingMessage.isEmpty())
                appendStringToLog(_loadingMessage);
        }

        dupedItemFound = false;
        if (_isDumpItemsMode)
        {
            QString itemsStr;
            foreach (ItemInfo *item, CharacterInfo::instance().items.character)
            {
                addItemName(item, itemsStr);
                foreach (ItemInfo *socketableItem, item->socketablesInfo)
                    addItemName(socketableItem, itemsStr);
            }

            QFile f(fileInfo.absoluteFilePath() + ".txt");
            if (f.open(QIODevice::WriteOnly))
                f.write(itemsStr.toUtf8());
            else
                appendStringToLog(QString("error writing %1: %2").arg(f.fileName(), f.errorString()));
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

        if (!_isDumpItemsMode && (!_skipEmptyCheckBox->isChecked() || dupedItemFound || !_loadingMessage.isEmpty()))
            appendStringToLog("========================================");

        _loadingMessage.clear();
        QMetaObject::invokeMethod(_progressBar, "setValue", Q_ARG(int, filesProcessed));
    }

    appendStringToLog(QString("loading files & separate processing took %1 seconds").arg(_timeCounter.elapsed() / 1000));
    if (_isDumpItemsMode)
    {
        scanFinished_();
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

bool DupeScanDialog::saveLog(const QString &fileName, bool isPlainText)
{
    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly))
        return false;
    f.write((isPlainText ? _logBrowser->toPlainText() : _logBrowser->toHtml()).toUtf8());
    return true;
}

QString DupeScanDialog::baseDupeScanLogFileName()
{
    return _pathLineEdit->text() + "/MXLOT dupe stats";
}
