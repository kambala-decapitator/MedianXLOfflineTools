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
#include <QtConcurrentRun>


DupeScanDialog::DupeScanDialog(const QString &currentPath, QWidget *parent) : QDialog(parent), _currentCharPath(currentPath), _pathLineEdit(new QLineEdit(this)), _logBrowser(new QTextEdit(this)),
    _saveButton(new QPushButton("Save...", this)), _dontWriteCheckBox(new QCheckBox("Don't write empty results", this)), _progressBar(new QProgressBar(this))
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle("Dupe Scanner");

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
    hbl2->addWidget(_dontWriteCheckBox);
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
    _dontWriteCheckBox->setChecked(true);
    _progressBar->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    resize(600, 400);

    connect(browseButton, SIGNAL(clicked()), SLOT(selectPath()));
    connect(scanButton,   SIGNAL(clicked()), SLOT(scan()));
    connect(_saveButton,  SIGNAL(clicked()), SLOT(save()));
    connect(okButton,     SIGNAL(clicked()), SLOT(accept()));
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

    QtConcurrent::run(this, &DupeScanDialog::scanCharactersInDir, path);
}

void DupeScanDialog::scanFinished()
{
    _logBrowser->append("That's all Folks!");
    _logBrowser->setUpdatesEnabled(true);

    _saveButton->setEnabled(true);
    qApp->alert(parentWidget());
}

void DupeScanDialog::save()
{
    QString plainTextFilter = "plain text (*.txt)", htmlTextFilter = "HTML (*.html)";
    QString selectedFilter, fileName = QFileDialog::getSaveFileName(this, "Save dupe report", QFileInfo(_currentCharPath).canonicalPath() + "/MXLOT dupe stats", plainTextFilter + ";;" + htmlTextFilter, &selectedFilter);
    if (fileName.isEmpty())
        return;

    QRegExp re("\\(\\*(.+)\\)");
    re.indexIn(selectedFilter);
    QString extension = re.cap(1);

    if (!fileName.endsWith(extension))
        fileName += extension;
    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly))
    {
        ERROR_BOX("failed to save file");
        return;
    }
    f.write((selectedFilter == plainTextFilter ? _logBrowser->toPlainText() : _logBrowser->toHtml()).toUtf8());
}

void DupeScanDialog::updateProgressbarForCrossCheck(int n)
{
    _progressBar->setValue(0);
    _progressBar->setMaximum(n * (n - 1) / 2);
    _progressBar->setFormat("cross-check %p%");
}

void DupeScanDialog::logDupedItemsInfo(ItemInfo *item1, ItemInfo *item2)
{
    appendStringToLog(QString("'%1': GUID 0x%2 (%3), type '%4', quality %5; %6; %7").arg(ItemDataBase::Items()->value(item1->itemType)->name, QString::number(item1->guid, 16)).arg(item1->guid)
                      .arg(item1->itemType.constData()).arg(metaEnumFromName<Enums::ItemQuality>("ItemQualityEnum").valueToKey(item1->quality))
                      .arg(ItemParser::itemStorageAndCoordinatesString("<font color=blue>ITEM1</font>: location %1, row %2, col %3, equipped in %4", item1))
                      .arg(ItemParser::itemStorageAndCoordinatesString("<font color=blue>ITEM2</font>: location %1, row %2, col %3, equipped in %4", item2)));
}

void DupeScanDialog::appendStringToLog(const QString &s)
{
    QMetaObject::invokeMethod(_logBrowser, "append", Q_ARG(QString, s));
}

void DupeScanDialog::scanCharactersInDir(const QString &path)
{
    appendStringToLog("<b>SEPARATE CHARACTER CHECK</b><br/>----------------------------------------");

    QHash<QString, ItemsList> allItemsHash;
    bool isFirst = true, dupedItemFound;
    QString pathWithSlashes = QDir::fromNativeSeparators(_currentCharPath);

    QFileInfoList list = QDir(path).entryInfoList(QDir::Files);
    list.prepend(QFileInfo(_currentCharPath)); // for currently loaded file
    QMetaObject::invokeMethod(_progressBar, "setMaximum", Q_ARG(int, list.size()));

    int filesProcessed = 0;
    foreach (const QFileInfo &fileInfo, list)
    {
        ++filesProcessed;

        QString fileName = fileInfo.fileName(), header = QString("%1 dupe stats").arg(fileName);
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
        if (!_dontWriteCheckBox->isChecked())
            appendStringToLog(header + "\n");

        dupedItemFound = false;
        ItemsList itemsAndSocketables = CharacterInfo::instance().items.character, checkedItems;
        for (int i = 0; i < itemsAndSocketables.size() - 1; ++i)
        {
            ItemInfo *iItem = itemsAndSocketables.at(i);
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

            if (iItem->isExtended)
            {
                for (int j = i + 1; j < itemsAndSocketables.size(); ++j)
                {
                    ItemInfo *jItem = itemsAndSocketables.at(j);
                    if (jItem->isExtended && iItem->guid == jItem->guid)
                    {
                        checkedItems += iItem;
                        if (!dupedItemFound && _dontWriteCheckBox->isChecked())
                        {
                            appendStringToLog(header + "\n");
                            dupedItemFound = true;
                        }
                        logDupedItemsInfo(iItem, jItem);
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
        allItemsHash[fileName] = itemsCopy;

        if (!_dontWriteCheckBox->isChecked() || dupedItemFound)
            appendStringToLog("========================================");

        QMetaObject::invokeMethod(_progressBar, "setValue", Q_ARG(int, filesProcessed));
    }

    appendStringToLog("<br/><b>CROSS-CHARACTER CHECK</b><br/>----------------------------------------");
    QMetaObject::invokeMethod(this, "updateProgressbarForCrossCheck", Q_ARG(int, allItemsHash.size()));

    filesProcessed = 1;
    for (QHash<QString, ItemsList>::const_iterator iter = allItemsHash.constBegin(); iter != allItemsHash.constEnd() - 1; ++iter)
    {
        dupedItemFound = false;
        if (!_dontWriteCheckBox->isChecked())
            appendStringToLog(iter.key());

        const ItemsList &iItems = iter.value();
        for (QHash<QString, ItemsList>::const_iterator jter = iter + 1; jter != allItemsHash.constEnd(); ++jter, ++filesProcessed)
        {
            isFirst = true;
            foreach (ItemInfo *iItem, iItems)
            {
                if (iItem->isExtended)
                {
                    foreach (ItemInfo *jItem, jter.value())
                    {
                        if (jItem->isExtended && iItem->guid == jItem->guid)
                        {
                            if (!dupedItemFound && _dontWriteCheckBox->isChecked())
                            {
                                appendStringToLog(iter.key());
                                dupedItemFound = true;
                            }
                            if (isFirst)
                            {
                                QMetaObject::invokeMethod(_logBrowser, "insertPlainText", Q_ARG(QString, "\n\n" + jter.key() + ":"));
                                isFirst = false;
                            }
                            logDupedItemsInfo(iItem, jItem);
                        }
                    }
                }
            }
        }

        if (!_dontWriteCheckBox->isChecked() || dupedItemFound)
            appendStringToLog("========================================");

        QMetaObject::invokeMethod(_progressBar, "setValue", Q_ARG(int, filesProcessed));
    }

    foreach (const ItemsList &items, allItemsHash)
        qDeleteAll(items);
    QMetaObject::invokeMethod(this, "scanFinished");
}
