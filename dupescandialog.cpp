#include "dupescandialog.h"
#include "characterinfo.hpp"
#include "itemparser.h"
#include "itemdatabase.h"
#include "enums.h"
#include "propertiesdisplaymanager.h"
#include "resourcepathmanager.hpp"
#include "xmlwriter.h"
#include "jsonwriter.h"

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
#include <QMenu>

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

static const QLatin1String XmlFormat("xml"), JsonFormat("json");


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

QString addBool(const QString &s, bool b) { return s + QLatin1String(b ? "1" : "0"); }
QString boolListToString(const QList<bool> &list) { return std::accumulate(list.constBegin(), list.constEnd(), QString(), addBool); }


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
    if (isDumpItemsMode)
    {
        QMenu *formatMenu = new QMenu(scanButton);
        foreach (const QString &format, QStringList() << XmlFormat << JsonFormat)
            formatMenu->addAction(format.toUpper());
        scanButton->setMenu(formatMenu);
        connect(formatMenu, SIGNAL(triggered(QAction *)), SLOT(dumpFormatSelected(QAction *)));
    }
    else
        connect(scanButton, SIGNAL(clicked()), SLOT(scan()));

    QHBoxLayout *hbl2 = new QHBoxLayout;
    hbl2->addWidget(scanButton);
    hbl2->addWidget(_saveButton);
    hbl2->addWidget(_progressBar);
    hbl2->addWidget(okButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(hbl);
    mainLayout->addWidget(_logBrowser);
    mainLayout->addLayout(hbl2);

    QString path;
    QStringList args = qApp->arguments();
    int argsSize = args.size();
    if ((_isAutoLaunched = argsSize >= 3))
    {
        path = args.last();

        bool isJson = false;
        for (int i = 2; i < argsSize - 1; ++i)
        {
            QString arg = args.at(i);
            if (arg.startsWith(QLatin1String("-v")))
                _isVerbose = true;
            else if (arg.endsWith(JsonFormat, Qt::CaseInsensitive))
                isJson = true;
        }
        _dumpFormat = isJson ? JsonFormat : XmlFormat;
    }
    else
        path = QFileInfo(currentPath).canonicalPath();
    _pathLineEdit->setText(QDir::toNativeSeparators(path));

    _logBrowser->setReadOnly(true);
    _saveButton->setDisabled(true);
    scanButton->setDefault(true);
    _progressBar->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    resize(600, 400);

    connect(browseButton, SIGNAL(clicked()), SLOT(selectPath()));
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

void DupeScanDialog::dumpFormatSelected(QAction *action)
{
    _dumpFormat = action->text().toLower();
    scan();
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
            IKeyValueWriter *charDumper;
            if (_dumpFormat == XmlFormat)
                charDumper = new XMLWriter(QLatin1String("char"));
            else
                charDumper = new JSONWriter;

            // info
            const CharacterInfo::CharacterInfoBasic &bci = ci.basicInfo;
            QVariantMap keyValue;
            keyValue[QLatin1String("title")] = Enums::Progression::titleNameAndMaxDifficultyFromValue(bci.titleCode, bci.classCode >= Enums::ClassName::Necromancer && bci.classCode <= Enums::ClassName::Druid, bci.isHardcore).first;
            keyValue[QLatin1String("name")] = bci.originalName;
            keyValue[QLatin1String("class")] = Enums::ClassName::classes().at(bci.classCode);
            keyValue[QLatin1String("schc")] = bci.isHardcore ? QLatin1String("HC") : QLatin1String("SC");
            keyValue[QLatin1String("status")] = bci.isHardcore && bci.hadDied ? QLatin1String("dead") : QLatin1String("alive");
            keyValue[QLatin1String("ladder")] = QString("%1Ladder").arg(bci.isLadder ? QLatin1String(0) : QLatin1String("Non-"));
            charDumper->addDataFromMap(QLatin1String("info"), keyValue);

            // stats
            quint32 exp = ci.valueOfStatistic(Enums::CharacterStats::Experience), prevExp = _experienceTable.at(bci.level - 1);
            keyValue.clear();
            keyValue[QLatin1String("level")] = bci.level;
            keyValue[QLatin1String("experience")] = exp;
            keyValue[QLatin1String("progress")] = QString("%1%").arg(static_cast<double>(exp - prevExp) / (_experienceTable.at(bci.level) - prevExp) * 100, 0, 'f', 0);
            keyValue[QLatin1String("strength")] = ci.valueOfStatistic(Enums::CharacterStats::Strength);
            keyValue[QLatin1String("life")] = ci.valueOfStatistic(Enums::CharacterStats::Life);
            keyValue[QLatin1String("base_life")] = ci.valueOfStatistic(Enums::CharacterStats::BaseLife);
            keyValue[QLatin1String("dexterity")] = ci.valueOfStatistic(Enums::CharacterStats::Dexterity);
            keyValue[QLatin1String("mana")] = ci.valueOfStatistic(Enums::CharacterStats::Mana);
            keyValue[QLatin1String("base_mana")] = ci.valueOfStatistic(Enums::CharacterStats::BaseMana);
            keyValue[QLatin1String("vitality")] = ci.valueOfStatistic(Enums::CharacterStats::Vitality);
            keyValue[QLatin1String("energy")] = ci.valueOfStatistic(Enums::CharacterStats::Energy);
            keyValue[QLatin1String("stash_gold")] = ci.valueOfStatistic(Enums::CharacterStats::StashGold);
            keyValue[QLatin1String("free_stat_points")] = ci.valueOfStatistic(Enums::CharacterStats::FreeStatPoints);
            keyValue[QLatin1String("sol_used")] = ci.valueOfStatistic(Enums::CharacterStats::SignetsOfLearningEaten);
            {
                QVariantList achievements = bci.statsDynamicData.values(Enums::CharacterStats::Achievements);
                QVariantList achievementsKeyValue;
                foreach (const QVariant &achievement, achievements)
                {
                    QVariantList achievementValues = achievement.toList();
                    QVariantMap achievementKeyValue;
                    achievementKeyValue[QLatin1String("layer")] = achievementValues.at(0);
                    achievementKeyValue[QLatin1String("value")] = achievementValues.at(1);
                    achievementsKeyValue += achievementKeyValue;
                }
                keyValue[QLatin1String("achievements")] = achievementsKeyValue;
            }
            charDumper->addDataFromMap(QLatin1String("stats"), keyValue);

            // quests
            keyValue.clear();
            keyValue[QLatin1String("denOfEvil")] = boolListToString(ci.questsInfo.denOfEvil);
            keyValue[QLatin1String("radament")] = boolListToString(ci.questsInfo.radament);
            keyValue[QLatin1String("goldenBird")] = boolListToString(ci.questsInfo.goldenBird);
            keyValue[QLatin1String("lamEsensTome")] = boolListToString(ci.questsInfo.lamEsensTome);
            keyValue[QLatin1String("izual")] = boolListToString(ci.questsInfo.izual);
            charDumper->addDataFromMap(QLatin1String("quests"), keyValue);

            // merc
            if (ci.mercenary.exists)
            {
                keyValue.clear();
                keyValue[QLatin1String("type")] = Enums::Mercenary::types().at(Enums::Mercenary::mercCodeFromValue(ci.mercenary.code));
                keyValue[QLatin1String("level")] = ci.mercenary.level;
                charDumper->addDataFromMap(QLatin1String("merc"), keyValue);
            }

            // skills
            QList<QVariantMap> skillsKeyValue;
            QList<int> skills = Enums::Skills::currentCharacterSkillsIndexes().second;
            for (int i = 0; i < skills.size(); ++i)
            {
                int skillIndex = skills.at(i);
                SkillInfo *skill = ItemDataBase::Skills()->value(skillIndex);
                keyValue.clear();
                keyValue[QLatin1String("name")] = skill->name;
                keyValue[QLatin1String("id")] = skillIndex;
                keyValue[QLatin1String("points")] = bci.skillsReadable.at(i);
                keyValue[QLatin1String("page")] = skill->tab;
                keyValue[QLatin1String("column")] = skill->col;
                keyValue[QLatin1String("row")] = skill->row;
                skillsKeyValue += keyValue;
            }
            charDumper->addDataFromArray(QLatin1String("skills"), QLatin1String("skill"), skillsKeyValue);

            // hotkeyedSkills
            keyValue.clear();
            keyValue[QLatin1String("main_lmb")] = keyValueFromSkillId(ci.basicInfo.mainHandSkills.lmb);
            keyValue[QLatin1String("main_rmb")] = keyValueFromSkillId(ci.basicInfo.mainHandSkills.rmb);
            keyValue[QLatin1String("alt_lmb")] = keyValueFromSkillId(ci.basicInfo.altHandSkills.lmb);
            keyValue[QLatin1String("alt_rmb")] = keyValueFromSkillId(ci.basicInfo.altHandSkills.rmb);
            QVariantList hotkeyedSkillsKeyValue;
            for (int i = 0; i < ci.basicInfo.hotkeyedSkills.size(); ++i)
            {
                quint32 skillId = ci.basicInfo.hotkeyedSkills.at(i);
                if (skillId != 65535)
                {
                    QVariantMap v = keyValueFromSkillId(skillId);
                    v[QLatin1String("index")] = i;
                    hotkeyedSkillsKeyValue += v;
                }
            }
            keyValue[QLatin1String("assigned")] = hotkeyedSkillsKeyValue;
            charDumper->addDataFromMap(QLatin1String("skills_hotkeyed"), keyValue);

            // items
            QList<QVariantMap> itemsKeyValue;
            foreach (ItemInfo *item, ci.items.character)
            {
                keyValue = keyValueFromItem(item);
                if (item->isExtended)
                {
                    keyValue[QLatin1String("socketsNumber")] = item->isSocketed ? item->socketsNumber : 0;
                    keyValue[QLatin1String("socketablesNumber")] = item->socketablesNumber;
                }
                keyValue[QLatin1String("isEthereal")] = item->isEthereal;
                keyValue[QLatin1String("isRW")] = item->isRW;
                keyValue[QLatin1String("placement")] = QString("location %1, ").arg(metaEnumFromName<Enums::ItemLocation>("ItemLocationEnum").valueToKey(item->location)) + ItemParser::itemStorageAndCoordinatesString("storage %1, row %2, col %3, equipped in %4", item);

                if (ItemDataBase::isUberCharm(item))
                {
                    keyValue[QLatin1String("isCharm")] = QLatin1String("1");
                    keyValue[QLatin1String("isClassCharm")] = QLatin1String(ItemDataBase::isClassCharm(item) ? "1" : "0");
                    keyValue[QLatin1String("hasTrophy")] = QLatin1String((item->props.find(Enums::ItemProperties::Trophy) != item->props.end()
                      || item->props.find(Enums::ItemProperties::ShrineBless) != item->props.end()) ? "1" : "0");

                    for (int cubeUpgradeStat = Enums::ItemProperties::CubeUpgrade1; cubeUpgradeStat <= Enums::ItemProperties::CubeUpgrade4; ++cubeUpgradeStat)
                        if (item->props.find(cubeUpgradeStat) != item->props.end())
                            keyValue[QLatin1String("isUpgraded")] = QLatin1String("1");
                }
                else
                {
                    static const QRegExp trophyRegex("^\\[\\d\\d$");
                    if (QString(item->itemType).contains(trophyRegex))
                        keyValue[QLatin1String("isTrophy")] = QLatin1String("1");
                }

                if (!item->socketablesInfo.isEmpty())
                {
                    QVariantList socketables;
                    foreach (ItemInfo *socketableItem, item->socketablesInfo)
                        socketables += keyValueFromItem(socketableItem);
                    keyValue[QLatin1String("socketables")] = socketables;
                }

                itemsKeyValue += keyValue;
            }
            charDumper->addDataFromArray(QLatin1String("items"), QLatin1String("item"), itemsKeyValue);

            QFile outFile(fileInfo.absoluteFilePath() + QString(".%1").arg(_dumpFormat));
            if (outFile.open(QIODevice::WriteOnly))
                outFile.write(charDumper->write());
            else
                appendStringToLog(QString("error writing %1: %2").arg(outFile.fileName(), outFile.errorString()));
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

QVariantMap DupeScanDialog::keyValueFromItem(ItemInfo *item)
{
    QVariantMap keyValue;

    QStringList nameList = ItemDataBase::completeItemName(item, false, false).split(kHtmlLineBreak);
    keyValue[QLatin1String("name")] = nameList.last();
    if (nameList.size() > 1)
        keyValue[QLatin1String("name_special")] = nameList.first();

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
    keyValue[QLatin1String("image")] = imageName.toLower();

    keyValue[QLatin1String("ilvl")] = item->ilvl;
    keyValue[QLatin1String("type")] = item->itemType.constData();
    keyValue[QLatin1String("quality")] = metaEnumFromName<Enums::ItemQuality>("ItemQualityEnum").valueToKey(item->quality);
    keyValue[QLatin1String("completeDescription")] = PropertiesDisplayManager::completeItemDescription(item, true).replace(QLatin1String("\n\n"), QLatin1String("\n")).trimmed();
    return keyValue;
}

QVariantMap DupeScanDialog::keyValueFromSkillId(quint32 skillId)
{
    static const quint32 LeftMouseSkillMask = 0x8000;
    QVariantMap keyValue;
    keyValue[QLatin1String("mouse")] = skillId & LeftMouseSkillMask ? QLatin1String("left") : QLatin1String("right");

    skillId &= LeftMouseSkillMask - 1; // upper bits also contain info about charges
    keyValue[QLatin1String("id")] = skillId;
    if (SkillInfo *skill = ItemDataBase::Skills()->value(skillId))
        keyValue[QLatin1String("name")] = skill->name;
    return keyValue;
}
