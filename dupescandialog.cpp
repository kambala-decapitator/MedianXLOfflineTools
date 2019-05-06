#include "dupescandialog.h"
#include "characterinfo.hpp"
#include "itemparser.h"
#include "itemdatabase.h"
#include "enums.h"
#include "propertiesdisplaymanager.h"
#include "resourcepathmanager.hpp"

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

#include <QXmlStreamWriter>

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


DupeScanDialog::DupeScanDialog(const QString &currentPath, bool isDumpItemsMode, QWidget *parent) : QDialog(parent), _currentCharPath(currentPath), _isDumpItemsMode(isDumpItemsMode), _futureWatcher(0), _isVerbose(false),
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
    _isAutoLaunched = args.size() >= 3;
    if (args.size() == 4)
        _isVerbose = args.at(2).startsWith(QLatin1String("-v"));
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

    // ugly copypaste
    QByteArray fileData = ItemDataBase::decompressedFileData(ResourcePathManager::dataPathForFileName("exptable.dat"), QString());
    if (!fileData.isEmpty())
    {
        _experienceTable.reserve(Enums::CharacterStats::MaxLevel);
        foreach (const QByteArray &numberString, fileData.split('\n'))
            if (!numberString.isEmpty())
                _experienceTable.append(numberString.trimmed().toUInt());
    }

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

    QFileInfo fi(path);
    QFileInfoList list = fi.isDir() ? QDir(path).entryInfoList(QDir::Files) : QFileInfoList() << fi;
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
        const CharacterInfo &ci = CharacterInfo::instance();
        if (_isDumpItemsMode)
        {
            QString filePath = fileInfo.absoluteFilePath();
            QFile xmlFile(filePath + ".xml");
            if (!xmlFile.open(QIODevice::WriteOnly))
            {
                appendStringToLog(QString("error writing %1: %2").arg(xmlFile.fileName(), xmlFile.errorString()));
                goto CHARACTER_END;
            }

            QXmlStreamWriter xml(&xmlFile);
            xml.setAutoFormatting(true);
            xml.writeStartDocument();
            xml.writeStartElement(QLatin1String("char"));

            const CharacterInfo::CharacterInfoBasic &bci = ci.basicInfo;
            xml.writeStartElement(QLatin1String("info"));
            xml.writeTextElement(QLatin1String("title"), Enums::Progression::titleNameAndMaxDifficultyFromValue(bci.titleCode, bci.classCode >= Enums::ClassName::Necromancer && bci.classCode <= Enums::ClassName::Druid, bci.isHardcore).first);
            xml.writeTextElement(QLatin1String("name"), bci.originalName);
            xml.writeTextElement(QLatin1String("class"), Enums::ClassName::classes().at(bci.classCode));
            xml.writeTextElement(QLatin1String("schc"), bci.isHardcore ? QLatin1String("HC") : QLatin1String("SC"));
            xml.writeTextElement(QLatin1String("status"), bci.isHardcore && bci.hadDied ? QLatin1String("dead") : QLatin1String("alive"));
            xml.writeTextElement(QLatin1String("ladder"), QString("%1Ladder").arg(bci.isLadder ? QLatin1String(0) : QLatin1String("Non-")));
            xml.writeEndElement(); // info

            quint32 exp = ci.valueOfStatistic(Enums::CharacterStats::Experience), prevExp = _experienceTable.at(bci.level - 1);
            xml.writeStartElement(QLatin1String("stats"));
            xml.writeTextElement(QLatin1String("level"), QString::number(bci.level));
            xml.writeTextElement(QLatin1String("experience"), QString::number(exp));
            xml.writeTextElement(QLatin1String("progress"), QString::number(static_cast<double>(exp - prevExp) / (_experienceTable.at(bci.level) - prevExp) * 100, 'f', 0) + QLatin1String("%"));
            xml.writeTextElement(QLatin1String("strength"), QString::number(ci.valueOfStatistic(Enums::CharacterStats::Strength)));
            xml.writeTextElement(QLatin1String("life"), QString::number(ci.valueOfStatistic(Enums::CharacterStats::Life)));
            xml.writeTextElement(QLatin1String("base_life"), QString::number(ci.valueOfStatistic(Enums::CharacterStats::BaseLife)));
            xml.writeTextElement(QLatin1String("dexterity"), QString::number(ci.valueOfStatistic(Enums::CharacterStats::Dexterity)));
            xml.writeTextElement(QLatin1String("mana"), QString::number(ci.valueOfStatistic(Enums::CharacterStats::Mana)));
            xml.writeTextElement(QLatin1String("base_mana"), QString::number(ci.valueOfStatistic(Enums::CharacterStats::BaseMana)));
            xml.writeTextElement(QLatin1String("vitality"), QString::number(ci.valueOfStatistic(Enums::CharacterStats::Vitality)));
            xml.writeTextElement(QLatin1String("energy"), QString::number(ci.valueOfStatistic(Enums::CharacterStats::Energy)));
            xml.writeTextElement(QLatin1String("stash_gold"), QString::number(ci.valueOfStatistic(Enums::CharacterStats::StashGold)));
            xml.writeTextElement(QLatin1String("free_stat_points"), QString::number(ci.valueOfStatistic(Enums::CharacterStats::FreeStatPoints)));
            xml.writeTextElement(QLatin1String("sol_used"), QString::number(ci.valueOfStatistic(Enums::CharacterStats::SignetsOfLearningEaten)));
            xml.writeEndElement(); // stats

            xml.writeStartElement(QLatin1String("skills"));
            QList<int> skills = Enums::Skills::currentCharacterSkillsIndexes().second;
            for (int i = 0; i < skills.size(); ++i)
            {
                int skillIndex = skills.at(i);
                SkillInfo *skill = ItemDataBase::Skills()->value(skillIndex);
                xml.writeStartElement(QLatin1String("skill"));
                xml.writeTextElement(QLatin1String("name"), skill->name);
                xml.writeTextElement(QLatin1String("id"), QString::number(skillIndex));
                xml.writeTextElement(QLatin1String("points"), QString::number(bci.skillsReadable.at(i)));
                xml.writeTextElement(QLatin1String("page"), QString::number(skill->tab));
                xml.writeTextElement(QLatin1String("column"), QString::number(skill->col));
                xml.writeTextElement(QLatin1String("row"), QString::number(skill->row));
                xml.writeEndElement(); // skill
            }
            xml.writeEndElement(); // skills

            xml.writeStartElement(QLatin1String("items"));
            foreach (ItemInfo *item, ci.items.character)
            {
                addItemInfoToXml(item, xml);
                xml.writeTextElement(QLatin1String("socketsNumber"), QString::number(item->socketsNumber));
                xml.writeTextElement(QLatin1String("socketablesNumber"), QString::number(item->socketablesNumber));
                xml.writeTextElement(QLatin1String("isEthereal"), QString::number(item->isEthereal));
                xml.writeTextElement(QLatin1String("isRW"), QString::number(item->isRW));
                xml.writeTextElement(QLatin1String("placement"), QString("location %1, ").arg(metaEnumFromName<Enums::ItemLocation>("ItemLocationEnum").valueToKey(item->location)) + ItemParser::itemStorageAndCoordinatesString("storage %1, row %2, col %3, equipped in %4", item));

                if (ItemDataBase::isUberCharm(item))
                    xml.writeTextElement(QLatin1String("isCharm"), QLatin1String("1"));
                else
                {
                    static const QRegExp trophyRegex("^\\[\\d\\d$");
                    if (QString(item->itemType).contains(trophyRegex))
                        xml.writeTextElement(QLatin1String("isTrophy"), QLatin1String("1"));
                }

                if (!item->socketablesInfo.isEmpty())
                {
                    xml.writeStartElement(QLatin1String("socketables"));
                    foreach (ItemInfo *socketableItem, item->socketablesInfo)
                    {
                        addItemInfoToXml(socketableItem, xml);
                        xml.writeEndElement(); // socketableItem
                    }
                    xml.writeEndElement(); // socketables
                }

                xml.writeEndElement(); // item
            }
            xml.writeEndElement(); // items

            xml.writeEndElement(); // char
            xml.writeEndDocument();
        }
        else
        {
            ItemsList itemsAndSocketables = ci.items.character, checkedItems;
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

CHARACTER_END:
        if (!_isDumpItemsMode && (!_skipEmptyCheckBox->isChecked() || dupedItemFound || !_loadingMessage.isEmpty()))
            appendStringToLog("========================================");

        _loadingMessage.clear();
        QMetaObject::invokeMethod(_progressBar, "setValue", Q_ARG(int, filesProcessed));
    }

    appendStringToLog(QString("loading files & separate processing took %1 seconds").arg(_timeCounter.elapsed() / 1000));
    if (_isDumpItemsMode)
    {
        QMetaObject::invokeMethod(this, "scanFinished_");
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

void DupeScanDialog::addItemInfoToXml(ItemInfo *item, QXmlStreamWriter &xml)
{
    xml.writeStartElement(QLatin1String("item"));

    xml.writeStartElement(QLatin1String("name"));
    QStringList nameList = ItemDataBase::completeItemName(item, false, false).split(kHtmlLineBreak);
    if (nameList.size() > 1)
        xml.writeAttribute(QLatin1String("special"), nameList.first());
    xml.writeCharacters(nameList.last());
    xml.writeEndElement(); // name

    ItemBase *baseInfo = ItemDataBase::Items()->value(item->itemType);
    QString imageName;
    if (item->quality == Enums::ItemQuality::Unique || item->quality == Enums::ItemQuality::Set)
    {
        SetOrUniqueItemInfo *setOrUniqueInfo = item->quality == Enums::ItemQuality::Set ? static_cast<SetOrUniqueItemInfo *>(ItemDataBase::Sets()   ->value(item->setOrUniqueId))
                                                                                        : static_cast<SetOrUniqueItemInfo *>(ItemDataBase::Uniques()->value(item->setOrUniqueId));
        if (setOrUniqueInfo && !setOrUniqueInfo->imageName.isEmpty())
            imageName = setOrUniqueInfo->imageName;
    }
    if (imageName.isEmpty())
    {
        if (item->variableGraphicIndex)
        {
            int i = item->variableGraphicIndex - 1;
            const QList<QByteArray> &variableImageNames = ItemDataBase::ItemTypes()->value(baseInfo->types.at(0)).variableImageNames;
            imageName = i < variableImageNames.size() ? variableImageNames.at(i) : variableImageNames.last();
        }
        else
            imageName = baseInfo->imageName;
    }
    xml.writeTextElement(QLatin1String("image"), imageName.toLower());

    xml.writeTextElement(QLatin1String("ilvl"), QString::number(item->ilvl));
    xml.writeTextElement(QLatin1String("type"), item->itemType.constData());
    xml.writeTextElement(QLatin1String("quality"), metaEnumFromName<Enums::ItemQuality>("ItemQualityEnum").valueToKey(item->quality));

    QString desc = PropertiesDisplayManager::completeItemDescription(item, true);
    xml.writeTextElement(QLatin1String("completeDescription"), desc.replace(QLatin1String("\n\n"), QLatin1String("\n")).trimmed());
}
