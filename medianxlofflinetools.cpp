#include "medianxlofflinetools.h"
#include "colors.hpp"
#include "qd2charrenamer.h"
#include "helpers.h"
#include "itemsviewerdialog.h"
#include "itemdatabase.h"
#include "propertiesviewerwidget.h"
#include "finditemsdialog.h"
#include "resourcepathmanager.hpp"
#include "reversebitwriter.h"
#include "itemparser.h"
#include "reversebitreader.h"
#include "itemspropertiessplitter.h"
#include "characterinfo.hpp"
#include "skillplandialog.h"

#include <QCloseEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QGridLayout>
#include <QFileDialog>
#include <QLabel>

#include <QSettings>
#include <QFile>
#include <QDataStream>
#include <QTranslator>
#include <QUrl>

#if defined(Q_WS_WIN32)
#include <Shlobj.h>
#include <Shobjidl.h>
#elif defined(Q_WS_MACX)
#include <ApplicationServices/ApplicationServices.h>
#endif

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif

#include <cmath>


static const QString lastSavePathKey("lastSavePath"), releaseDate("24.03.2012"), backupExtension("bak"), readonlyCss("background-color: rgb(227, 227, 227)");
static const QString characterExtension("d2s"), characterExtensionWithDot = "." + characterExtension;

//#define MAKE_HC
//#define ENABLE_PERSONALIZE


// static const

const QString MedianXLOfflineTools::compoundFormat("%1, %2");
const quint32 MedianXLOfflineTools::fileSignature = 0xAA55AA55;
const int MedianXLOfflineTools::skillsNumber = 30;
const int MedianXLOfflineTools::difficultiesNumber = 3;
const int MedianXLOfflineTools::statPointsPerLevel = 5;
const int MedianXLOfflineTools::skillPointsPerLevel = 1;
const int MedianXLOfflineTools::maxRecentFiles = 10;


// ctor

MedianXLOfflineTools::MedianXLOfflineTools(const QString &cmdPath, QWidget *parent, Qt::WFlags flags) : QMainWindow(parent, flags), _findItemsDialog(0),
    hackerDetected(tr("1337 hacker detected! Please, play legit.")), //difficulties(QStringList() << tr("Hatred") << tr("Nightmare") << tr("Destruction")),
    maxValueFormat(tr("Max: %1")), minValueFormat(tr("Min: %1")), investedValueFormat(tr("Invested: %1")), _isLoaded(false)
{
    ui.setupUi(this);

    checkFileAssociations();
    loadData();
    createLanguageMenu();
    createLayout();
    loadSettings();
    fillMaps();
    connectSignals();
    
    ui.actionFindNext->setShortcut(QKeySequence::FindNext);
    ui.actionFindPrevious->setShortcut(QKeySequence::FindPrevious);

#ifdef Q_WS_MACX
    qApp->installEventFilter(this);
#endif

    if (!cmdPath.isEmpty())
        loadFile(cmdPath);
    else if (ui.actionLoadLastUsedCharacter->isChecked() && !_recentFilesList.isEmpty())
        loadFile(_recentFilesList.at(0));
    else
        updateWindowTitle();
}

bool MedianXLOfflineTools::loadFile(const QString &charPath)
{
    if (charPath.isEmpty() || !charPath.endsWith(characterExtensionWithDot) || !maybeSave())
        return false;

    bool result;
    if ((result = processSaveFile(charPath)))
    {
        _charPath = charPath;
        updateUI();
        addToRecentFiles(charPath);

        if (_itemsDialog)
            _itemsDialog->updateItems(getPlugyStashesExistenceHash());
        if (_itemsDialog || ui.actionOpenItemsAutomatically->isChecked())
            showItems();

        QSettings settings;
        settings.beginGroup("recentItems");
        settings.setValue(lastSavePathKey, QFileInfo(charPath).canonicalPath());
    }
    else
    {
        _saveFileContents.clear();
        _charPath.clear();

        clearUI();
        updateWindowTitle();
    }

    if (_findItemsDialog)
        _findItemsDialog->clearResults();

    return result;
}


// slots

void MedianXLOfflineTools::switchLanguage(QAction *languageAction)
{
    QByteArray newLocale = languageAction->statusTip().toLatin1();
    if (LanguageManager::instance().currentLocale != newLocale)
    {
        LanguageManager::instance().currentLocale = newLocale;

        static bool wasMessageBoxShown = false;
        if (!wasMessageBoxShown)
        {
            INFO_BOX(tr("Language will be changed next time you run the application"));
            wasMessageBoxShown = true;
        }
    }
}

void MedianXLOfflineTools::wasModified(bool modified)
{
    setWindowModified(modified);
    ui.actionReloadCharacter->setEnabled(modified);
}

void MedianXLOfflineTools::loadCharacter()
{
    QSettings settings;
    settings.beginGroup("recentItems");
    QString lastSavePath = settings.value(lastSavePathKey).toString();

    QString charPath = QFileDialog::getOpenFileName(this, tr("Load Character"), lastSavePath, tr("Diablo 2 Save Files") + QString(" (*%1)").arg(characterExtensionWithDot));
    if (loadFile(charPath))
        ui.statusBar->showMessage(tr("Character loaded"), 3000);
}

void MedianXLOfflineTools::openRecentFile()
{
    loadFile(qobject_cast<QAction *>(sender())->statusTip());
}

void MedianXLOfflineTools::reloadCharacter()
{
    if (loadFile(_charPath))
        ui.statusBar->showMessage(tr("Character reloaded"), 3000);
}

void MedianXLOfflineTools::saveCharacter()
{
    QByteArray statsBytes = statisticBytes();
    if (statsBytes.isEmpty())
        return;

    CharacterInfo &charInfo = CharacterInfo::instance();
    QByteArray tempFileContents(_saveFileContents);
    tempFileContents.replace(Enums::Offsets::StatsData, charInfo.skillsOffset - Enums::Offsets::StatsData, statsBytes);
    int diff = Enums::Offsets::StatsData + statsBytes.size() - charInfo.skillsOffset;
    charInfo.skillsOffset = Enums::Offsets::StatsData + statsBytes.size();
    charInfo.itemsOffset += diff;
    charInfo.itemsEndOffset += diff;

    if (ui.respecSkillsCheckBox->isChecked())
        tempFileContents.replace(charInfo.skillsOffset + 2, skillsNumber, QByteArray(skillsNumber, 0));

    if (ui.activateWaypointsCheckBox->isChecked())
    {
        QByteArray activatedWaypointsBytes(5, 0xFF); // 40 x '1'
        for (int startPos = Enums::Offsets::WaypointsData + 2, i = 0; i < difficultiesNumber; ++i, startPos += 24)
            tempFileContents.replace(startPos, activatedWaypointsBytes.size(), activatedWaypointsBytes);
    }

    if (ui.convertToSoftcoreCheckBox->isChecked())
        charInfo.basicInfo.isHardcore = false;

#ifdef MAKE_HC
    charInfo.basicInfo.isHardcore = true;
    charInfo.basicInfo.hadDied = false;
#endif
    char statusValue = tempFileContents[Enums::Offsets::Status];
    if (charInfo.basicInfo.hadDied)
        statusValue |= Enums::StatusBits::HadDied;
    else
        statusValue &= ~Enums::StatusBits::HadDied;
    if (charInfo.basicInfo.isHardcore)
        statusValue |= Enums::StatusBits::IsHardcore;
    else
        statusValue &= ~Enums::StatusBits::IsHardcore;
    tempFileContents[Enums::Offsets::Status] = statusValue;

    QDataStream outputDataStream(&tempFileContents, QIODevice::ReadWrite);
    outputDataStream.setByteOrder(QDataStream::LittleEndian);

    //quint8 curDiff[difficultiesNumber] = {0, 0, 0};
    //curDiff[ui.currentDifficultyComboBox->currentIndex()] = 128 + ui.currentActSpinBox->value() - 1; // 10000xxx
    //outputDataStream.skipRawData(Enums::Offsets::CurrentLocation);
    //for (int i = 0; i < difficultiesNumber; ++i)
    //    outputDataStream << curDiff[i];

    //outputDataStream.skipRawData(Enums::Offsets::Progression);
    //outputDataStream << quint16(13); // set max act to 4 in Destruction
    //outputDataStream.device()->reset();

    //quint16 one = 1;
    //outputDataStream.skipRawData(Enums::Offsets::QuestsData + 2 * Enums::Quests::Size + 30); // Destruction A3
    //outputDataStream << one;
    //outputDataStream.skipRawData(14); // A4
    //outputDataStream << one;
    //outputDataStream.skipRawData(4); // A4Q3 (A4Q2 is skipRawData(6) for some reason)
    //outputDataStream << one;         // is completed
    //outputDataStream.skipRawData(2); // A5
    //outputDataStream << one;
    //outputDataStream.device()->reset();
    //outputDataStream.skipRawData(Enums::Offsets::CurrentLocation + 2); // set current diff to Dest and act to 4
    //quint8 v = 131; // 10000011
    //outputDataStream << v;
    //outputDataStream.device()->reset();
#ifdef ENABLE_PERSONALIZE
    for (int i = 0; i < difficultiesNumber; ++i)
    {
        outputDataStream.device()->seek(Enums::Offsets::QuestsData + i * Enums::Quests::Size + Enums::Quests::Nihlathak);
        outputDataStream << quint16(Enums::Quests::IsTaskDone);
    }
    outputDataStream.device()->reset();
#endif

    QString newName = charInfo.basicInfo.newName;
    bool hasNameChanged = !newName.isEmpty() && charInfo.basicInfo.originalName != newName;
    if (hasNameChanged)
    {
        outputDataStream.device()->seek(Enums::Offsets::Name);
#ifdef Q_WS_MACX
        QByteArray newNameByteArray = macTextCodec()->fromUnicode(newName);
#else
        QByteArray newNameByteArray = newName.toLocal8Bit();
#endif
        newNameByteArray += QByteArray(QD2CharRenamer::maxNameLength + 1 - newName.length(), '\0'); // add trailing nulls
        outputDataStream.writeRawData(newNameByteArray.constData(), newNameByteArray.length());
    }
    else
        newName = charInfo.basicInfo.originalName;

    quint8 newClvl = ui.levelSpinBox->value();
    if (charInfo.basicInfo.level != newClvl)
    {
        charInfo.basicInfo.level = newClvl;
        charInfo.basicInfo.totalSkillPoints = ui.freeSkillPointsLineEdit->text().toUShort();
        recalculateStatPoints();

        outputDataStream.device()->seek(Enums::Offsets::Level);
        outputDataStream << newClvl;

        ui.levelSpinBox->setMaximum(newClvl);
    }

    if (charInfo.mercenary.exists)
    {
        quint16 newMercValue = Enums::Mercenary::mercBaseValueFromCode(charInfo.mercenary.code) + ui.mercTypeComboBox->currentIndex();
        charInfo.mercenary.code = Enums::Mercenary::mercCodeFromValue(newMercValue);
        charInfo.mercenary.nameIndex = ui.mercNameComboBox->currentIndex();
        outputDataStream.device()->seek(Enums::Offsets::Mercenary + 4);
        outputDataStream << charInfo.mercenary.nameIndex << newMercValue;
    }

    int characterItemsSize = 2;
    ItemsList characterItems;
    QHash<Enums::ItemStorage::ItemStorageEnum, ItemsList > plugyItemsHash;
    foreach (ItemInfo *item, charInfo.items.character)
    {
        switch (item->storage)
        {
        case Enums::ItemStorage::PersonalStash: case Enums::ItemStorage::SharedStash: case Enums::ItemStorage::HCStash:
            plugyItemsHash[static_cast<Enums::ItemStorage::ItemStorageEnum>(item->storage)] += item;
            break;
        default:
            characterItems += item;
            characterItemsSize += 2 + item->bitString.length() / 8; // JM + item bytes
            foreach (ItemInfo *socketableItem, item->socketablesInfo)
                characterItemsSize += 2 + socketableItem->bitString.length() / 8; // JM + item bytes
            break;
        }
    }

    // TODO: place lower
    for (QHash<Enums::ItemStorage::ItemStorageEnum, PlugyStashInfo>::const_iterator iter = _plugyStashesHash.constBegin(); iter != _plugyStashesHash.constEnd(); ++iter)
    {
        const ItemsList &items = plugyItemsHash[iter.key()];
        if (std::find_if(items.constBegin(), items.constEnd(), hasChanged) == items.constEnd())
            continue;

        const PlugyStashInfo &info = iter.value();
        QFile inputFile(info.path);
        if (!inputFile.exists())
            continue;

        backupFile(inputFile);
        if (!inputFile.open(QIODevice::WriteOnly))
        {
            showErrorMessageBoxForFile(tr("Error creating file '%1'"), inputFile);
            continue;
        }

        QDataStream plugyFileDataStream(&inputFile);
        plugyFileDataStream.setByteOrder(QDataStream::LittleEndian);
        plugyFileDataStream.writeRawData(info.header.constData(), info.header.size());
        plugyFileDataStream << info.version;
        if (info.hasGold)
            plugyFileDataStream << info.gold;
        plugyFileDataStream << info.lastPage;

        for (quint32 page = 1; page <= info.lastPage; ++page)
        {
            ItemsList pageItems;
            foreach (ItemInfo *anItem, items)
                if (anItem->plugyPage == page)
                    pageItems += anItem;

            plugyFileDataStream.writeRawData(ItemParser::plugyPageHeader.constData(), ItemParser::plugyPageHeader.size() + 1); // write '\0'
            plugyFileDataStream.writeRawData(ItemParser::itemHeader.constData(), ItemParser::itemHeader.size()); // do not write '\0'
            plugyFileDataStream << static_cast<quint16>(pageItems.size());
            ItemParser::writeItems(pageItems, plugyFileDataStream);
        }
    }

    tempFileContents.replace(charInfo.itemsOffset, charInfo.itemsEndOffset - charInfo.itemsOffset, QByteArray(characterItemsSize, 0));
    outputDataStream.device()->seek(charInfo.itemsOffset);
    outputDataStream << static_cast<quint16>(characterItems.size());
    ItemParser::writeItems(characterItems, outputDataStream);

    quint32 fileSize = tempFileContents.size();
    outputDataStream.device()->seek(Enums::Offsets::FileSize);
    outputDataStream << fileSize;
    outputDataStream << checksum(tempFileContents);

    QSettings settings;
    settings.beginGroup("recentItems");
    QString savePath = settings.value(lastSavePathKey).toString(), fileName = savePath + "/" + newName, saveFileName = fileName + characterExtensionWithDot;

    QFile outputFile(saveFileName);
    if (hasNameChanged)
    {
        QFile oldFile(QString("%1/%2.%3").arg(savePath, charInfo.basicInfo.originalName, characterExtension));
        backupFile(oldFile);
    }
    else
        backupFile(outputFile);

    if (outputFile.open(QIODevice::WriteOnly))
    {
        int bytesWritten = outputFile.write(tempFileContents);
        if (bytesWritten == static_cast<int>(fileSize)) // shut the compiler up
        {
            outputFile.flush();
            _saveFileContents = tempFileContents;

            if (hasNameChanged)
            {
                // delete .d2s and rename all other related files like .d2x, .key, .ma0, etc.
                bool isOldNameEmpty = QRegExp(QString("[ %1]+").arg(QChar(QChar::Nbsp))).exactMatch(charInfo.basicInfo.originalName);
                bool hasNonAsciiChars = false;
                for (int i = 0; i < charInfo.basicInfo.originalName.length(); ++i)
                    if (charInfo.basicInfo.originalName.at(i).unicode() > 255)
                    {
                        hasNonAsciiChars = true;
                        break;
                    }

                bool isStrangeName = hasNonAsciiChars || isOldNameEmpty;
                QDir sourceFileDir(savePath, isStrangeName ? "*" : charInfo.basicInfo.originalName + ".*");
                foreach (const QFileInfo &fileInfo, sourceFileDir.entryInfoList())
                {
                    QString extension = fileInfo.suffix();
                    if ((isStrangeName && fileInfo.baseName() != charInfo.basicInfo.originalName) || extension == backupExtension)
                        continue;

                    QFile sourceFile(fileInfo.canonicalFilePath());
                    if (extension == characterExtension) // delete
                    {
                        if (!sourceFile.remove())
                            showErrorMessageBoxForFile(tr("Error removing file '%1'"), sourceFile);
                    }
                    else // rename
                    {
                        if (!sourceFile.rename(fileName + "." + extension) && !isOldNameEmpty)
                            showErrorMessageBoxForFile(tr("Error renaming file '%1'"), sourceFile);
                    }
                }

                charInfo.basicInfo.originalName = newName;

                _recentFilesList[0] = saveFileName;
                updateRecentFilesActions();
            }

            _charPath = saveFileName;
            wasModified(false);
            loadFile(_charPath); // update all UI at once by reloading the file

            INFO_BOX(tr("File '%1' successfully saved!").arg(QDir::toNativeSeparators(saveFileName)));
        }
        else
            showErrorMessageBoxForFile(tr("Error writing file '%1'"), outputFile);
    }
    else
        showErrorMessageBoxForFile(tr("Error creating file '%1'"), outputFile);
}

void MedianXLOfflineTools::statChanged(int newValue)
{
    QSpinBox *senderSpinBox = qobject_cast<QSpinBox *>(sender());
    int minimum = senderSpinBox->minimum();
    updateMinCompoundStatusTip(senderSpinBox, minimum, newValue - minimum);

    int *pOldValue = &_oldStatValues[_spinBoxesStatsMap.key(senderSpinBox)], diff = newValue - *pOldValue;
    if (_isLoaded && diff)
    {
        *pOldValue = newValue;

        ui.freeStatPointsLineEdit->setText(QString::number(ui.freeStatPointsLineEdit->text().toUInt() - diff));
        QString s = ui.freeStatPointsLineEdit->statusTip();
        int start = s.indexOf(": ") + 2, end = s.indexOf(","), total = s.mid(start, end - start).toInt();
        updateMaxCompoundStatusTip(ui.freeStatPointsLineEdit, total, investedStatPoints());

        foreach (QSpinBox *spinBox, _spinBoxesStatsMap)
            if (spinBox != senderSpinBox)
                spinBox->setMaximum(spinBox->maximum() - diff);
        // explicitly update status bar
        QStatusTipEvent event(senderSpinBox->statusTip());
        qApp->sendEvent(senderSpinBox, &event);

        updateTableStats(_baseStatsMap[CharacterInfo::instance().basicInfo.classCode].statsPerPoint, diff, senderSpinBox);
        wasModified(true);
    }
}

void MedianXLOfflineTools::respecStats()
{
    for (int i = Enums::CharacterStats::Strength; i <= Enums::CharacterStats::Vitality; ++i)
    {
        Enums::CharacterStats::StatisticEnum statCode = static_cast<Enums::CharacterStats::StatisticEnum>(i);
        int baseStat = _baseStatsMap[CharacterInfo::instance().basicInfo.classCode].statsAtStart.statFromCode(statCode);
        _spinBoxesStatsMap[statCode]->setValue(baseStat);
    }
    ui.statusBar->clearMessage();
    wasModified(true);
}

void MedianXLOfflineTools::respecSkills(bool shouldRespec)
{
    quint16 skills = CharacterInfo::instance().basicInfo.totalSkillPoints;
    ui.freeSkillPointsLineEdit->setText(QString::number(shouldRespec ? skills : CharacterInfo::instance().basicInfo.statsDynamicData.property("FreeSkillPoints").toUInt()));
    updateMaxCompoundStatusTip(ui.freeSkillPointsLineEdit, skills, shouldRespec ? 0 : skills - CharacterInfo::instance().basicInfo.statsDynamicData.property("FreeSkillPoints").toUInt());
    wasModified(true);
}

void MedianXLOfflineTools::rename()
{
    QString &newName = CharacterInfo::instance().basicInfo.newName;
    QD2CharRenamer renameWidget(newName, ui.actionWarnWhenColoredName->isChecked(), this);
    if (renameWidget.exec())
    {
        newName = renameWidget.name();
        QD2CharRenamer::updateNamePreview(ui.charNamePreview, newName);
        wasModified(true);
    }
}

void MedianXLOfflineTools::levelChanged(int newClvl)
{
    int lvlDiff = _oldClvl - newClvl;
    const CharacterInfo::CharacterInfoBasic &basicInfo = CharacterInfo::instance().basicInfo;
    if (_isLoaded && qAbs(lvlDiff) < basicInfo.level)
    {
        updateTableStats(_baseStatsMap[basicInfo.classCode].statsPerLevel, -lvlDiff);

        int statsDiff = lvlDiff * statPointsPerLevel;
        _oldClvl = newClvl;
        if (ui.freeStatPointsLineEdit->text().toInt() - statsDiff < 0)
            respecStats();
        ui.freeStatPointsLineEdit->setText(QString::number(ui.freeStatPointsLineEdit->text().toInt() - statsDiff));
        foreach (QSpinBox *spinBox, _spinBoxesStatsMap)
            spinBox->setMaximum(spinBox->maximum() - statsDiff);

        bool hasLevelChanged = newClvl != basicInfo.level;
        if (_resurrectionPenalty != ResurrectPenaltyDialog::Skills)
        {
            ui.respecSkillsCheckBox->setChecked(hasLevelChanged);
            ui.respecSkillsCheckBox->setDisabled(hasLevelChanged);
        }

        int newSkillPoints = basicInfo.totalSkillPoints, investedSkillPoints = 0;
        if (hasLevelChanged)
        {
            newSkillPoints -= (basicInfo.level - newClvl) * skillPointsPerLevel;
            ui.freeSkillPointsLineEdit->setText(QString::number(newSkillPoints));
        }
        else if (_resurrectionPenalty == ResurrectPenaltyDialog::Skills)
            respecSkills(true);
        else
            investedSkillPoints = newSkillPoints - CharacterInfo::instance().basicInfo.statsDynamicData.property("FreeSkillPoints").toInt();

        int investedStats = investedStatPoints();
        updateStatusTips(ui.freeStatPointsLineEdit->text().toInt() + investedStats, investedStats, newSkillPoints, investedSkillPoints);
    }
}

void MedianXLOfflineTools::resurrect()
{
    CharacterInfo::CharacterInfoBasic &basicInfo = CharacterInfo::instance().basicInfo;
    ResurrectPenaltyDialog dlg(this);
    if (dlg.exec())
    {
        basicInfo.hadDied = false;
        updateHardcoreUIElements();
        updateCharacterTitle(true);

        ui.levelSpinBox->setMaximum(basicInfo.level);
        ui.levelSpinBox->setValue(basicInfo.level);

        _resurrectionPenalty = dlg.resurrectionPenalty();
        switch (_resurrectionPenalty)
        {
        case ResurrectPenaltyDialog::Levels:
        {
            int newLevel = basicInfo.level - ResurrectPenaltyDialog::levelPenalty;
            if (newLevel < 1)
                newLevel = 1;
            ui.levelSpinBox->setMaximum(newLevel); // setValue() is invoked implicitly

            break;
        }
        case ResurrectPenaltyDialog::Skills:
        {
            if (!ui.respecSkillsCheckBox->isChecked())
                ui.respecSkillsCheckBox->setChecked(true);
            ui.respecSkillsCheckBox->setDisabled(true);

            ushort newSkillPoints = ui.freeSkillPointsLineEdit->text().toUShort();
            newSkillPoints -= newSkillPoints * ResurrectPenaltyDialog::skillPenalty;
            basicInfo.totalSkillPoints = newSkillPoints;

            ui.freeSkillPointsLineEdit->setText(QString::number(newSkillPoints));
            updateMaxCompoundStatusTip(ui.freeSkillPointsLineEdit, newSkillPoints, 0);

            break;
        }
        case ResurrectPenaltyDialog::Stats:
        {
            respecStats();

            ushort newStatPoints = ui.freeStatPointsLineEdit->text().toUShort(), diff = newStatPoints * ResurrectPenaltyDialog::statPenalty;
            newStatPoints -= diff;
            basicInfo.totalStatPoints = newStatPoints;

            ui.freeStatPointsLineEdit->setText(QString::number(newStatPoints));
            updateMaxCompoundStatusTip(ui.freeStatPointsLineEdit, newStatPoints, 0);

            foreach (QSpinBox *spinBox, _spinBoxesStatsMap)
                spinBox->setMaximum(spinBox->maximum() - diff);

            break;
        }
        default:
            break;
        }

        wasModified(true);
    }
}

void MedianXLOfflineTools::convertToSoftcore(bool isSoftcore)
{
    updateCharacterTitle(!isSoftcore);
    wasModified(true);
}

//void MedianXLOfflineTools::currentDifficultyChanged(int newDifficulty)
//{
//    quint8 progression = CharacterInfo::instance().basicInfo.titleCode, maxDifficulty = ui.currentDifficultyComboBox->count() - 1, maxAct;
//    if (progression == Enums::Progression::Completed - 1 || newDifficulty != maxDifficulty)
//        maxAct = 5;
//    else
//    {
//        maxAct = 1 + (progression - maxDifficulty) % 4;
//        if (maxAct == 4)
//        {
//            // check if A5 is enabled
//            if (_saveFileContents.at(Enums::Offsets::QuestsData + newDifficulty * Enums::Quests::Size + Enums::Quests::Act5Offset))
//                ++maxAct;
//        }
//    }
//    ui.currentActSpinBox->setMaximum(maxAct);
//}

void MedianXLOfflineTools::findItem()
{
    if (!_findItemsDialog)
    {
        _findItemsDialog = new FindItemsDialog(this);
        connect(ui.actionFindNext, SIGNAL(triggered()), _findItemsDialog, SLOT(findNext()));
        connect(ui.actionFindPrevious, SIGNAL(triggered()), _findItemsDialog, SLOT(findPrevious()));
        connect(_findItemsDialog, SIGNAL(itemFound(ItemInfo *)), SLOT(showFoundItem(ItemInfo *)));
    }
    _findItemsDialog->show();
    _findItemsDialog->activateWindow();
}

void MedianXLOfflineTools::showFoundItem(ItemInfo *item)
{
    ui.actionFindNext->setDisabled(!item);
    ui.actionFindPrevious->setDisabled(!item);
    if (item)
    {
        showItems(false);
        _itemsDialog->showItem(item);
    }
}

void MedianXLOfflineTools::showItems(bool activate /*= true*/)
{
    if (_itemsDialog)
    {
        if (activate)
            _itemsDialog->activateWindow();
    }
    else
    {
        _itemsDialog = new ItemsViewerDialog(getPlugyStashesExistenceHash(), this);
        connect(_itemsDialog->tabWidget(), SIGNAL(currentChanged(int)), SLOT(itemStorageTabChanged(int)));
        connect(_itemsDialog, SIGNAL(cubeDeleted(bool)), ui.actionGiveCube, SLOT(setEnabled(bool)));
        connect(_itemsDialog, SIGNAL(closing(bool)), ui.menuGoToPage, SLOT(setDisabled(bool)));
        connect(_itemsDialog, SIGNAL(itemsChanged(bool)), SLOT(wasModified(bool)));
        _itemsDialog->show();

        if (!activate)
        {
            _findItemsDialog->activateWindow();
            _findItemsDialog->raise();
        }
    }
}

void MedianXLOfflineTools::itemStorageTabChanged(int tabIndex)
{
    bool isPlugyStorage = _itemsDialog->isPlugyStorageIndex(tabIndex);
    ui.menuGoToPage->setEnabled(isPlugyStorage);

    static const QList<QAction *> plugyNavigationActions = QList<QAction *>() << ui.actionPrevious10 << ui.actionPreviousPage << ui.actionNextPage << ui.actionNext10
                                                                              << ui.actionPrevious100 << ui.actionFirstPage << ui.actionLastPage << ui.actionNext100;
    foreach (QAction *action, plugyNavigationActions)
        action->disconnect();

    if (isPlugyStorage)
    {
        static const QList<const char *> plugyNavigationSlots = QList<const char *>() << SLOT(previous10Pages()) << SLOT(previousPage()) << SLOT(nextPage()) << SLOT(next10Pages())
                                                                                      << SLOT(previous100Pages()) << SLOT(firstPage()) << SLOT(lastPage()) << SLOT(next100Pages());
        ItemsPropertiesSplitter *plugyTab = _itemsDialog->splitterAtIndex(tabIndex);
        for (int i = 0; i < plugyNavigationActions.size(); ++i)
            connect(plugyNavigationActions[i], SIGNAL(triggered()), plugyTab, plugyNavigationSlots[i]);
    }
}

void MedianXLOfflineTools::giveCube()
{
    ItemInfo *cube = ItemDataBase::loadItemFromFile("cube");
    if (!cube)
        return;
    
    if (!ItemDataBase::storeItemIn(cube, Enums::ItemStorage::Inventory, ItemsViewerDialog::rows.at(ItemsViewerDialog::tabIndexFromItemStorage(Enums::ItemStorage::Inventory))) &&
        !ItemDataBase::storeItemIn(cube, Enums::ItemStorage::Stash,     ItemsViewerDialog::rows.at(ItemsViewerDialog::tabIndexFromItemStorage(Enums::ItemStorage::Stash))))
    {
        ERROR_BOX(tr("You have no free space in inventory and stash to store the Cube"));
        delete cube;
        return;
    }

    // predefined position is (0,0) in inventory
    if (cube->column)
        ReverseBitWriter::replaceValueInBitString(cube->bitString, Enums::ItemOffsets::Column, cube->column);
    if (cube->row)
        ReverseBitWriter::replaceValueInBitString(cube->bitString, Enums::ItemOffsets::Row, cube->row);

    QHash<int, bool> plugyStashesExistenceHash = getPlugyStashesExistenceHash();
    if (cube->storage != Enums::ItemStorage::Inventory && plugyStashesExistenceHash[Enums::ItemStorage::PersonalStash])
    {
        ReverseBitWriter::replaceValueInBitString(cube->bitString, Enums::ItemOffsets::Storage, cube->storage);

        ItemInfo *plugyCube = new ItemInfo(*cube);
        plugyCube->storage = Enums::ItemStorage::PersonalStash;
        plugyCube->plugyPage = 1;
        CharacterInfo::instance().items.character += plugyCube;
    }
    CharacterInfo::instance().items.character += cube;

    if (_itemsDialog)
        _itemsDialog->updateItems(plugyStashesExistenceHash);

    ui.actionGiveCube->setDisabled(true);
    wasModified(true);
    INFO_BOX(itemStorageAndCoordinatesString(tr("Cube has been stored in %1 at (%2,%3)"), cube));
}

void MedianXLOfflineTools::getSkillPlan()
{
    SkillplanDialog dlg(this);
    dlg.exec();
}

void MedianXLOfflineTools::backupSettingTriggered(bool checked)
{
    if (!checked && QUESTION_BOX_YESNO(tr("Are you sure you want to disable automatic backups? Then don't blame me if your character gets corrupted."), QMessageBox::No) == QMessageBox::No)
        ui.actionBackup->setChecked(true);
}

void MedianXLOfflineTools::aboutApp()
{
    QMessageBox aboutBox(this);
    aboutBox.setWindowTitle(tr("About %1").arg(qApp->applicationName()));
    aboutBox.setIconPixmap(windowIcon().pixmap(64));
    aboutBox.setTextFormat(Qt::RichText);
    QString appFullName = qApp->applicationName() + " " + qApp->applicationVersion();
    aboutBox.setText(QString("<b>%1</b>%2").arg(appFullName, htmlLineBreak) + tr("Released: %1").arg(releaseDate));
    QString email("decapitator@ukr.net");
    aboutBox.setInformativeText(
        tr("<i>Author:</i> Filipenkov Andrey (<b>kambala</b>)") + QString("%1<i>ICQ:</i> 287764961%1<i>E-mail:</i> <a href=\"mailto:%2?subject=%3\">%2</a>%1%1").arg(htmlLineBreak, email, appFullName) +
        tr("<a href=\"http://modsbylaz.14.forumer.com/viewtopic.php?t=23147\">Official Median XL Forum thread</a><br>"
           "<a href=\"http://forum.worldofplayers.ru/showthread.php?t=34489\">Official Russian Median XL Forum thread</a>") + htmlLineBreak + htmlLineBreak +
        tr("<b>Credits:</b>"
           "<ul>"
             "<li><a href=\"http://modsbylaz.hugelaser.com/\">BrotherLaz</a> for this awesome mod</li>"
             "<li><a href=\"http://modsbylaz.14.forumer.com/profile.php?mode=viewprofile&u=33805\">grig</a> for the Perl source of "
                 "<a href=\"http://grig.vlexofree.com/\">Median XL Online Tools</a> and tips</li>"
             "<li><a href=\"http://phrozenkeep.hugelaser.com/index.php?ind=reviews&op=section_view&idev=4\">Phrozen Keep File Guides</a> for tons of useful information on txt sources</li>"
             "<li><a href=\"http://modsbylaz.14.forumer.com/profile.php?mode=viewprofile&u=44046\">FixeR</a> and <a href=\"http://forum.worldofplayers.ru/member.php?u=84592\">Zelgadiss</a> "
                 "for intensive testing and tips on GUI & functionality</li>"
           "</ul>")
    );
    aboutBox.exec();
}


// overridden protected methods

void MedianXLOfflineTools::closeEvent(QCloseEvent *e)
{
    if (maybeSave())
    {
        saveSettings();
        e->accept();
    }
    else
        e->ignore();
}

void MedianXLOfflineTools::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls())
    {
        QString path = mimeData->urls().at(0).toLocalFile();
        if (path.right(4).toLower() == characterExtensionWithDot)
            event->acceptProposedAction();
    }
}

void MedianXLOfflineTools::dropEvent(QDropEvent *event)
{
    loadFile(event->mimeData()->urls().at(0).toLocalFile());
    event->acceptProposedAction();
}


// private methods

void MedianXLOfflineTools::checkFileAssociations()
{
#if defined(Q_WS_WIN32)
    QString appPath = QDir::toNativeSeparators(qApp->applicationFilePath()), binaryName = QFileInfo(appPath).fileName();
    if (QSysInfo::windowsVersion() < QSysInfo::WV_VISTA)
    {
        QString fileType("Diablo2.savefile.character");
        QString cmdOpenFileFormat("%1/shell/open/command/."), defaultApplicationRegistryPath = cmdOpenFileFormat.arg(fileType);
        // TODO: write also to HKEY_CURRENT_USER\\Software\\Classes (in case user is not admin)
        QSettings settings("HKEY_CURRENT_USER\\Software\\Classes", QSettings::NativeFormat);
        // TODO: also check      HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.d2s\UserChoice\Progid (now it's Applications\XVI32.exe)
        // on XP must be checked HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.d2s\Application (if present, contains some binary name)
        if (!settings.value(defaultApplicationRegistryPath).toString().startsWith(appPath))
        {
            QString defaultValueFormat("%1/.");
            settings.setValue(defaultValueFormat.arg(characterExtensionWithDot), fileType);
            settings.setValue(defaultValueFormat.arg(fileType), "Diablo 2 character save file");
            settings.setValue(defaultApplicationRegistryPath, appPath + " \"%1\"");

            QString registryAppPath = QString("Applications/%1").arg(binaryName);
            settings.setValue(cmdOpenFileFormat.arg(registryAppPath), appPath + " \"%1\"");
            settings.setValue(QString("%1/SupportedTypes/%2").arg(registryAppPath, characterExtensionWithDot), QString(""));

            QSettings hklm("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths", QSettings::NativeFormat);
            hklm.setValue(defaultValueFormat.arg(binaryName), appPath);
            hklm.setValue(QString("%1/Path").arg(binaryName), QDir::toNativeSeparators(qApp->applicationDirPath()));

            SHChangeNotify(SHCNE_ASSOCCHANGED, 0, 0, 0);
            INFO_BOX("file association changed");
        }
    }
    else
    {
        IApplicationAssociationRegistration *pAAR;
        HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistration, NULL, CLSCTX_INPROC, __uuidof(IApplicationAssociationRegistration), (void **)&pAAR);
        qDebug("CoCreateInstance result: %#x", hr);
        if (SUCCEEDED(hr))
        {
            QStdWString appNameStdWstr = binaryName.toStdWString(), extensionWithDotStdWstr = characterExtensionWithDot.toStdWString();
            LPCWSTR appNameWstr = appNameStdWstr.c_str(), extensionWithDotWstr = extensionWithDotStdWstr.c_str();
            OutputDebugString(appNameWstr);
            BOOL isDefault;
            hr = pAAR->QueryAppIsDefault(extensionWithDotWstr, AT_FILEEXTENSION, AL_EFFECTIVE, appNameWstr, &isDefault); // returns 0x80070002 if app doesn't exist in registry
            qDebug("\nQueryAppIsDefault result: %#x, is default: %d", hr, isDefault);
            if (!isDefault)
            {
                LPWSTR defaultAppNameWstr;
                hr = pAAR->QueryCurrentDefault(extensionWithDotWstr, AT_FILEEXTENSION, AL_EFFECTIVE, &defaultAppNameWstr);
                OutputDebugString(defaultAppNameWstr);
                qDebug("\nQueryCurrentDefault result: %#x", hr);

                //hr = pAAR->SetAppAsDefault(appNameWstr, extensionWithDotWstr, AT_FILEEXTENSION);
                //qDebug("SetAppAsDefault result: %#x", hr);
                //if (SUCCEEDED(hr))
                //    qDebug("app is default now");
            }
            pAAR->Release();
        }
    }
#elif defined(Q_WS_MACX)
    FSRef defaultAppRef = {{0}}; // shut clang up
    OSStatus err = LSGetApplicationForInfo(kLSUnknownType, kLSUnknownCreator, CFSTR("d2s"), kLSRolesAll, &defaultAppRef, NULL);
    if (err != noErr)
    {
        qDebug("error getting default app: %d", err);
        return;
    }

    CFURLRef bundlePath = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    if (LSRegisterURL(bundlePath) == noErr)
        qDebug("registered");
    CFRelease(bundlePath);

    CFStringRef displayName = NULL;
    err = LSCopyDisplayNameForRef(&defaultAppRef, &displayName);
    CFShow(displayName);
    CFIndex l = CFStringGetLength(displayName);
    char *cstr = new char[l+1];
    qDebug("was copying string successful: %d", CFStringGetCString(displayName, cstr, l+1, kCFStringEncodingMacRoman));
    INFO_BOX(cstr);
    CFRelease(displayName);
#else
#error Add code to check file association or remove/comment this line
#endif
}

void MedianXLOfflineTools::loadData()
{
    loadExpTable();
    loadMercNames();
    loadBaseStats();
    SkillplanDialog::loadModVersion();
}

void MedianXLOfflineTools::loadExpTable()
{
    QFile f;
    if (!ItemDataBase::createUncompressedTempFile(ResourcePathManager::dataPathForFileName("exptable.dat"), tr("Experience table data not loaded."), &f))
        return;

    QList<QByteArray> expLines = f.readAll().split('\n');
    experienceTable.reserve(Enums::CharacterStats::MaxLevel);
    foreach (const QByteArray &numberString, expLines)
        if (!numberString.isEmpty())
            experienceTable.append(numberString.trimmed().toUInt());
    f.remove();
}

void MedianXLOfflineTools::loadMercNames()
{
    QFile f;
    if (!ItemDataBase::createUncompressedTempFile(ResourcePathManager::localizedPathForFileName("mercs"), tr("Mercenary names not loaded."), &f))
        return;

    QList<QByteArray> mercLines = f.readAll().split('\n');
    QStringList actNames;
    mercNames.reserve(4);
    foreach (const QByteArray &mercName, mercLines)
    {
        if (mercName.startsWith("-"))
        {
            mercNames += actNames;
            actNames.clear();
        }
        else
            actNames += QString::fromUtf8(mercName.trimmed());
    }
    f.remove();
}

void MedianXLOfflineTools::loadBaseStats()
{
    QFile f;
    if (!ItemDataBase::createUncompressedTempFile(ResourcePathManager::dataPathForFileName("basestats.dat"), tr("Base stats data not loaded, using predefined one."), &f))
    {
        _baseStatsMap[Enums::ClassName::Amazon]      = BaseStats(BaseStats::StatsAtStart(25, 25, 20, 15, 84), BaseStats::StatsPerLevel(100, 40, 60), BaseStats::StatsPerPoint( 8, 8, 18));
        _baseStatsMap[Enums::ClassName::Sorceress]   = BaseStats(BaseStats::StatsAtStart(10, 25, 15, 35, 74), BaseStats::StatsPerLevel(100, 40, 60), BaseStats::StatsPerPoint( 8, 8, 18));
        _baseStatsMap[Enums::ClassName::Necromancer] = BaseStats(BaseStats::StatsAtStart(15, 25, 20, 25, 79), BaseStats::StatsPerLevel( 80, 20, 80), BaseStats::StatsPerPoint( 4, 8, 24));
        _baseStatsMap[Enums::ClassName::Paladin]     = BaseStats(BaseStats::StatsAtStart(25, 20, 25, 15, 89), BaseStats::StatsPerLevel(120, 60, 40), BaseStats::StatsPerPoint(12, 8, 12));
        _baseStatsMap[Enums::ClassName::Barbarian]   = BaseStats(BaseStats::StatsAtStart(30, 20, 30,  5, 92), BaseStats::StatsPerLevel(120, 60, 40), BaseStats::StatsPerPoint(12, 8, 12));
        _baseStatsMap[Enums::ClassName::Druid]       = BaseStats(BaseStats::StatsAtStart(25, 20, 15, 25, 84), BaseStats::StatsPerLevel( 80, 20, 80), BaseStats::StatsPerPoint( 4, 8, 24));
        _baseStatsMap[Enums::ClassName::Assassin]    = BaseStats(BaseStats::StatsAtStart(20, 35, 15, 15, 95), BaseStats::StatsPerLevel(100, 40, 60), BaseStats::StatsPerPoint( 8, 8, 18));
        return;
    }

    QList<QByteArray> lines = f.readAll().split('\n');
    foreach (const QByteArray &s, lines)
        if (!s.isEmpty() && s.at(0) != '#')
        {
            QList<QByteArray> numbers = s.trimmed().split('\t');
            _baseStatsMap[static_cast<Enums::ClassName::ClassNameEnum>(numbers.at(0).toUInt())] = BaseStats
                (
                BaseStats::StatsAtStart (numbers.at(1).toInt(), numbers.at(2).toInt(), numbers.at(4).toInt(), numbers.at(3).toInt(), numbers.at(5).toInt()), // order is correct - energy value comes before vitality in the file
                BaseStats::StatsPerLevel(numbers.at(6).toInt(), numbers.at(7).toInt(), numbers.at(8).toInt()),
                BaseStats::StatsPerPoint(numbers.at(9).toInt(), numbers.at(10).toInt(), numbers.at(11).toInt())
                );
        }
    f.remove();
}

void MedianXLOfflineTools::createLanguageMenu()
{
    QString appTranslationName = qApp->applicationName().remove(' ').toLower();
    QStringList fileNames = QDir(LanguageManager::instance().translationsPath, QString("%1_*.qm").arg(appTranslationName)).entryList(QDir::Files);
    if (!fileNames.isEmpty())
    {
        QMenu *languageMenu = new QMenu(tr("&Language", "Language menu"), this);
        ui.menuOptions->addSeparator();
        ui.menuOptions->addMenu(languageMenu);

        QActionGroup *languageActionGroup = new QActionGroup(this);
        connect(languageActionGroup, SIGNAL(triggered(QAction *)), SLOT(switchLanguage(QAction *)));

        // HACK: insert English language
        fileNames.prepend(QString("%1_%2.qm").arg(appTranslationName, LanguageManager::instance().defaultLocale));
        foreach (const QString &fileName, fileNames)
        {
            QTranslator translator;
            translator.load(fileName, LanguageManager::instance().translationsPath);
            QString language = translator.translate("Language", "English", "Your language name");
            if (language.isEmpty())
                language = "English";

            QAction *action = new QAction(language, this);
            action->setCheckable(true);
            QString locale = fileName.mid(fileName.indexOf('_') + 1, 2);
            action->setStatusTip(locale);
            languageMenu->addAction(action);
            languageActionGroup->addAction(action);

            if (locale == LanguageManager::instance().currentLocale)
                action->setChecked(true);
        }
    }
}

void MedianXLOfflineTools::createLayout()
{
    QList<QWidget *> widgetsToFixSize = QList<QWidget *>() << ui.charNamePreview << ui.classLineEdit << ui.titleLineEdit << ui.freeSkillPointsLineEdit
                                                           << ui.freeStatPointsLineEdit << ui.signetsOfLearningEatenLineEdit << ui.signetsOfSkillEatenLineEdit << ui.strengthSpinBox << ui.dexteritySpinBox
                                                           << ui.vitalitySpinBox << ui.energySpinBox << ui.inventoryGoldLineEdit << ui.stashGoldLineEdit << ui.mercLevelLineEdit;
    foreach (QWidget *w, widgetsToFixSize)
        w->setFixedSize(w->size());

    QList<QLineEdit *> readonlyLineEdits = QList<QLineEdit *>() << ui.freeSkillPointsLineEdit << ui.freeStatPointsLineEdit << ui.signetsOfLearningEatenLineEdit << ui.signetsOfSkillEatenLineEdit
                                                                << ui.inventoryGoldLineEdit << ui.stashGoldLineEdit << ui.mercLevelLineEdit << ui.classLineEdit;
    foreach (QLineEdit *lineEdit, readonlyLineEdits)
        lineEdit->setStyleSheet(readonlyCss);

    createCharacterGroupBoxLayout();
    createMercGroupBoxLayout();
    createStatsGroupBoxLayout();

    //QGridLayout *grid = new QGridLayout(centralWidget());
    //grid->addWidget(ui.characterGroupBox, 0, 0);
    //grid->addWidget(ui.mercGroupBox, 1, 0);
    //grid->addWidget(ui.statsGroupBox, 0, 1, 2, 1);

    QVBoxLayout *vbl = new QVBoxLayout;
    vbl->addWidget(ui.characterGroupBox);
    vbl->addStretch();
    vbl->addWidget(ui.mercGroupBox);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget());
    mainLayout->addLayout(vbl);
    mainLayout->addWidget(ui.statsGroupBox);

    ui.statsTableWidget->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui.statsTableWidget->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui.statsTableWidget->setFixedHeight(ui.statsTableWidget->height());

    _charPathLabel = new QLabel(this);
    ui.statusBar->addPermanentWidget(_charPathLabel);

    // on Mac OS X some UI elements become ugly if main window is set to minimumSize()
#ifndef Q_WS_MACX
    resize(minimumSizeHint());
#endif
}

void MedianXLOfflineTools::createCharacterGroupBoxLayout()
{
    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->addWidget(new QLabel(tr("Name")), 0, 0, Qt::AlignRight);
    gridLayout->addWidget(ui.charNamePreview, 0, 1);
    gridLayout->addWidget(ui.renameButton, 0, 2);

    gridLayout->addWidget(new QLabel(tr("Class")), 1, 0, Qt::AlignRight);
    gridLayout->addWidget(ui.classLineEdit, 1, 1);
    gridLayout->addWidget(new QLabel(tr("Level")), 1, 2, Qt::AlignCenter);

    gridLayout->addWidget(new QLabel(tr("Title", "Character title - Slayer/Champion/etc.")), 2, 0, Qt::AlignRight);
    gridLayout->addWidget(ui.titleLineEdit, 2, 1);
    gridLayout->addWidget(ui.levelSpinBox, 2, 2);

    //QHBoxLayout *hbl1 = new QHBoxLayout;
    //hbl1->addWidget(new QLabel(tr("Difficulty")), 0, Qt::AlignRight);
    //hbl1->addWidget(ui.currentDifficultyComboBox);
    //hbl1->addWidget(new QLabel(tr("Act")), 0, Qt::AlignRight);
    //hbl1->addWidget(ui.currentActSpinBox);

    QHBoxLayout *hbl2 = new QHBoxLayout(ui.hardcoreGroupBox);
    hbl2->addWidget(ui.convertToSoftcoreCheckBox);
    hbl2->addStretch();
    hbl2->addWidget(ui.resurrectButton);

    QVBoxLayout *vbl = new QVBoxLayout(ui.characterGroupBox);
    vbl->addLayout(gridLayout);
    //vbl->addLayout(hbl1);
    vbl->addWidget(ui.hardcoreGroupBox);
}

void MedianXLOfflineTools::createMercGroupBoxLayout()
{
    QHBoxLayout *hbl = new QHBoxLayout(ui.mercGroupBox);
    hbl->addWidget(new QLabel(tr("Type")));
    hbl->addWidget(ui.mercTypeComboBox);
    hbl->addWidget(new QLabel(tr("Level")));
    hbl->addWidget(ui.mercLevelLineEdit);
    hbl->addWidget(new QLabel(tr("Name")));
    hbl->addWidget(ui.mercNameComboBox);
}

void MedianXLOfflineTools::createStatsGroupBoxLayout()
{
    QGridLayout *gridLayout = new QGridLayout(ui.statsGroupBox);
    gridLayout->addWidget(new QLabel(tr("Inventory Gold")), 0, 0, Qt::AlignRight);
    gridLayout->addWidget(ui.inventoryGoldLineEdit, 0, 1);
    gridLayout->addWidget(ui.activateWaypointsCheckBox, 0, 2);
    gridLayout->addWidget(new QLabel(tr("Stash Gold")), 0, 3, Qt::AlignRight);
    gridLayout->addWidget(ui.stashGoldLineEdit, 0, 4);

    gridLayout->addWidget(new QLabel(tr("Free Skills")), 1, 0, Qt::AlignRight);
    gridLayout->addWidget(ui.freeSkillPointsLineEdit, 1, 1);
    gridLayout->addWidget(ui.respecSkillsCheckBox, 1, 2);
    gridLayout->addWidget(new QLabel(tr("Signets of Skill")), 1, 3, Qt::AlignRight);
    gridLayout->addWidget(ui.signetsOfSkillEatenLineEdit, 1, 4);

    gridLayout->addWidget(ui.statsTableWidget, 2, 2, 4, 3, Qt::AlignCenter);
    gridLayout->addWidget(new QLabel(tr("Strength")), 2, 0, Qt::AlignRight);
    gridLayout->addWidget(new QLabel(tr("Dexterity")), 3, 0, Qt::AlignRight);
    gridLayout->addWidget(new QLabel(tr("Vitality")), 4, 0, Qt::AlignRight);
    gridLayout->addWidget(new QLabel(tr("Energy")), 5, 0, Qt::AlignRight);
    gridLayout->addWidget(ui.strengthSpinBox, 2, 1);
    gridLayout->addWidget(ui.dexteritySpinBox, 3, 1);
    gridLayout->addWidget(ui.vitalitySpinBox, 4, 1);
    gridLayout->addWidget(ui.energySpinBox, 5, 1);

    gridLayout->addWidget(new QLabel(tr("Free Stats")), 6, 0, Qt::AlignRight);
    gridLayout->addWidget(ui.freeStatPointsLineEdit, 6, 1);
    gridLayout->addWidget(ui.respecStatsButton, 6, 2);
    gridLayout->addWidget(new QLabel(tr("Signets of Learning")), 6, 3, Qt::AlignRight);
    gridLayout->addWidget(ui.signetsOfLearningEatenLineEdit, 6, 4);
}

void MedianXLOfflineTools::loadSettings()
{
    QSettings settings;
    if (settings.contains("origin"))
        move(settings.value("origin").toPoint());

    settings.beginGroup("recentItems");
    _recentFilesList = settings.value("recentFiles").toStringList();
    updateRecentFilesActions();
    settings.endGroup();

    settings.beginGroup("options");
    ui.actionLoadLastUsedCharacter->setChecked(settings.value("loadLastCharacter", true).toBool());
    ui.actionWarnWhenColoredName->setChecked(settings.value("warnWhenColoredName", true).toBool());
    ui.actionBackup->setChecked(settings.value("makeBackups", true).toBool());
    ui.actionOpenItemsAutomatically->setChecked(settings.value("openItemsAutomatically").toBool());
    ui.actionReloadSharedStashes->setChecked(settings.value("reloadSharedStashes").toBool());

    settings.beginGroup("autoOpenSharedStashes");
    ui.actionAutoOpenPersonalStash->setChecked(settings.value("personal", true).toBool());
    ui.actionAutoOpenSharedStash->setChecked(settings.value("shared", true).toBool());
    ui.actionAutoOpenHCShared->setChecked(settings.value("hcShared", true).toBool());
    settings.endGroup();

    settings.endGroup();
}

void MedianXLOfflineTools::saveSettings() const
{
    QSettings settings;
    settings.setValue(LanguageManager::instance().languageKey, LanguageManager::instance().currentLocale);
    settings.setValue("origin", pos());

    settings.beginGroup("recentItems");
    settings.setValue("recentFiles", _recentFilesList);
    settings.endGroup();
    
    settings.beginGroup("options");
    settings.setValue("loadLastCharacter", ui.actionLoadLastUsedCharacter->isChecked());
    settings.setValue("warnWhenColoredName", ui.actionWarnWhenColoredName->isChecked());
    settings.setValue("makeBackups", ui.actionBackup->isChecked());
    settings.setValue("openItemsAutomatically", ui.actionOpenItemsAutomatically->isChecked());
    settings.setValue("reloadSharedStashes", ui.actionReloadSharedStashes->isChecked());

    settings.beginGroup("autoOpenSharedStashes");
    settings.setValue("personal", ui.actionAutoOpenPersonalStash->isChecked());
    settings.setValue("shared", ui.actionAutoOpenSharedStash->isChecked());
    settings.setValue("hcShared", ui.actionAutoOpenHCShared->isChecked());
    settings.endGroup();

    settings.endGroup();

    if (_findItemsDialog)
        _findItemsDialog->saveSettings();
    if (_itemsDialog)
        _itemsDialog->saveSettings();
}

void MedianXLOfflineTools::fillMaps()
{
    _spinBoxesStatsMap[Enums::CharacterStats::Strength] = ui.strengthSpinBox;
    _spinBoxesStatsMap[Enums::CharacterStats::Dexterity] = ui.dexteritySpinBox;
    _spinBoxesStatsMap[Enums::CharacterStats::Vitality] = ui.vitalitySpinBox;
    _spinBoxesStatsMap[Enums::CharacterStats::Energy] = ui.energySpinBox;

    _lineEditsStatsMap[Enums::CharacterStats::FreeStatPoints] = ui.freeStatPointsLineEdit;
    _lineEditsStatsMap[Enums::CharacterStats::FreeSkillPoints] = ui.freeSkillPointsLineEdit;
    _lineEditsStatsMap[Enums::CharacterStats::InventoryGold] = ui.inventoryGoldLineEdit;
    _lineEditsStatsMap[Enums::CharacterStats::StashGold] = ui.stashGoldLineEdit;
    _lineEditsStatsMap[Enums::CharacterStats::SignetsOfLearningEaten] = ui.signetsOfLearningEatenLineEdit;
    _lineEditsStatsMap[Enums::CharacterStats::SignetsOfSkillEaten] = ui.signetsOfSkillEatenLineEdit;

    const QList<SkillInfo> &skills = *ItemDataBase::Skills();
    int n = skills.size();
    for (int classCode = Enums::ClassName::Amazon; classCode <= Enums::ClassName::Assassin; ++classCode)
    {
        QList<int> skillsIndeces;
        for (int i = 0; i < n; ++i)
            if (skills.at(i).classCode == classCode)
                skillsIndeces += i;
        _characterSkillsIndeces[static_cast<Enums::ClassName::ClassNameEnum>(classCode)].first = skillsIndeces;

        for (int i = 0; i < 6; ++i)
        {
            int start = i * 5, end = start + 5;
            for (int j = start; j < end - 1; ++j)
                for (int k = j + 1; k < end; ++k)
                    if (skills[skillsIndeces.at(j)].row > skills[skillsIndeces.at(k)].row)
                        skillsIndeces.swap(j, k);
        }
        _characterSkillsIndeces[static_cast<Enums::ClassName::ClassNameEnum>(classCode)].second = skillsIndeces;
    }
}

void MedianXLOfflineTools::connectSignals()
{
    // files
    connect(ui.actionLoadCharacter, SIGNAL(triggered()), SLOT(loadCharacter()));
    connect(ui.actionReloadCharacter, SIGNAL(triggered()), SLOT(reloadCharacter()));
    connect(ui.actionSaveCharacter, SIGNAL(triggered()), SLOT(saveCharacter()));

    // edit
    connect(ui.actionRename, SIGNAL(triggered()), SLOT(rename()));
    connect(ui.actionFind, SIGNAL(triggered()), SLOT(findItem()));

    // items
    connect(ui.actionShowItems, SIGNAL(triggered()), SLOT(showItems()));
    connect(ui.actionGiveCube, SIGNAL(triggered()), SLOT(giveCube()));

    // export
    connect(ui.actionSkillPlan, SIGNAL(triggered()), SLOT(getSkillPlan()));

    // options
    connect(ui.actionBackup, SIGNAL(triggered(bool)), SLOT(backupSettingTriggered(bool)));

    // about
    connect(ui.actionAbout, SIGNAL(triggered()), SLOT(aboutApp()));
    connect(ui.actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    connect(ui.levelSpinBox, SIGNAL(valueChanged(int)), SLOT(levelChanged(int)));
    foreach (QSpinBox *spinBox, _spinBoxesStatsMap)
        connect(spinBox, SIGNAL(valueChanged(int)), SLOT(statChanged(int)));

    connect(ui.respecStatsButton, SIGNAL(clicked()), SLOT(respecStats()));
    connect(ui.renameButton, SIGNAL(clicked()), SLOT(rename()));
    connect(ui.resurrectButton, SIGNAL(clicked()), SLOT(resurrect()));
    connect(ui.convertToSoftcoreCheckBox, SIGNAL(toggled(bool)), SLOT(convertToSoftcore(bool)));
    connect(ui.respecSkillsCheckBox, SIGNAL(toggled(bool)), SLOT(respecSkills(bool)));
    //connect(ui.currentDifficultyComboBox, SIGNAL(currentIndexChanged(int)), SLOT(currentDifficultyChanged(int)));
}

void MedianXLOfflineTools::updateRecentFilesActions()
{
    ui.menuRecentCharacters->clear();
    ui.menuRecentCharacters->setDisabled(_recentFilesList.isEmpty());

    for (int i = 0; i < _recentFilesList.length(); ++i)
    {
        QString filePath = _recentFilesList.at(i);
        if (QFile::exists(filePath))
            ui.menuRecentCharacters->addAction(createRecentFileAction(filePath, i + 1));
        else
            _recentFilesList.removeAt(i--);
    }
}

void MedianXLOfflineTools::addToRecentFiles(const QString &fileName)
{
    int index = _recentFilesList.indexOf(fileName);
    if (index != -1) // it's already in the list
        _recentFilesList.move(index, 0);
    else
    {
        if (_recentFilesList.length() == maxRecentFiles)
            _recentFilesList.removeLast();
        _recentFilesList.prepend(fileName);
    }
    updateRecentFilesActions();
}

QAction *MedianXLOfflineTools::createRecentFileAction(const QString &fileName, int index)
{
    QAction *recentFileAction = new QAction(QString("&%1 %2").arg(index).arg(QFileInfo(fileName).fileName()), this);
    recentFileAction->setStatusTip(QDir::toNativeSeparators(fileName));
    connect(recentFileAction, SIGNAL(triggered()), SLOT(openRecentFile()));
    return recentFileAction;
}

bool MedianXLOfflineTools::processSaveFile(const QString &charPath)
{
    QFile inputFile(charPath);
    if (!inputFile.open(QIODevice::ReadOnly))
    {
        showErrorMessageBoxForFile(tr("Error opening file '%1'"), inputFile);
        return false;
    }

    _saveFileContents = inputFile.readAll();
    inputFile.close();

    QDataStream inputDataStream(_saveFileContents);
    inputDataStream.setByteOrder(QDataStream::LittleEndian);

    quint32 signature;
    inputDataStream >> signature;
    if (signature != fileSignature)
    {
        ERROR_BOX(tr("Wrong file signature: should be 0x%1, got 0x%2.").arg(fileSignature, 0, 16).arg(signature, 0, 16));
        return false;
    }

    inputDataStream.device()->seek(Enums::Offsets::Checksum);
    quint32 fileChecksum, computedChecksum = checksum(_saveFileContents);
    inputDataStream >> fileChecksum;
    if (fileChecksum != computedChecksum)
    {
        ERROR_BOX(tr("Character checksum doesn't match. Looks like it's corrupted."));
        return false;
    }

    CharacterInfo &charInfo = CharacterInfo::instance();
    charInfo.basicInfo.originalName = _saveFileContents.constData() + Enums::Offsets::Name;
    charInfo.basicInfo.originalName.replace(ansiColorHeader, unicodeColorHeader);
    charInfo.basicInfo.newName = charInfo.basicInfo.originalName;

    inputDataStream.device()->seek(Enums::Offsets::Status);
    quint8 status, progression, classCode, clvl;
    inputDataStream >> status >> progression;
    inputDataStream.device()->seek(Enums::Offsets::Class);
    inputDataStream >> classCode;
    inputDataStream.device()->seek(Enums::Offsets::Level);
    inputDataStream >> clvl;

    if (!(status & Enums::StatusBits::IsExpansion))
    {
        ERROR_BOX(tr("This is not Expansion character."));
        return false;
    }
    charInfo.basicInfo.isHardcore = status & Enums::StatusBits::IsHardcore;
    charInfo.basicInfo.hadDied = status & Enums::StatusBits::HadDied;

    if (classCode > Enums::ClassName::Assassin)
    {
        ERROR_BOX(tr("Wrong class value: got %1").arg(classCode));
        return false;
    }
    charInfo.basicInfo.classCode = static_cast<Enums::ClassName::ClassNameEnum>(classCode);

    if (progression >= Enums::Progression::Completed)
    {
        ERROR_BOX(tr("Wrong progression value: got %1").arg(progression));
        return false;
    }
    charInfo.basicInfo.titleCode = progression;

    if (!clvl || clvl > Enums::CharacterStats::MaxLevel)
    {
        ERROR_BOX(tr("Wrong level: got %1").arg(clvl));
        return false;
    }
    charInfo.basicInfo.level = clvl;

    //inputDataStream.device()->seek(Enums::Offsets::CurrentLocation);
    //for (int i = 0; i < 3; ++i)
    //{
    //    quint8 difficulty;
    //    inputDataStream >> difficulty;
    //    if (difficulty & Enums::DifficultyBits::IsActive)
    //    {
    //        charInfo.basicInfo.currentDifficulty = i;
    //        charInfo.basicInfo.currentAct = (difficulty & Enums::DifficultyBits::CurrentAct) + 1;
    //        break;
    //    }
    //}

    inputDataStream.device()->seek(Enums::Offsets::Mercenary);
    quint32 mercID;
    inputDataStream >> mercID;
    if ((charInfo.mercenary.exists = (mercID != 0)))
    {
        quint16 mercName, mercValue;
        inputDataStream >> mercName >> mercValue;
        if (mercValue > Enums::Mercenary::MaxCode)
        {
            ERROR_BOX(tr("Wrong mercenary code: got %1").arg(mercValue));
            return false;
        }
        charInfo.mercenary.code = Enums::Mercenary::mercCodeFromValue(mercValue);
        charInfo.mercenary.nameIndex = mercName;

        quint32 mercExp;
        inputDataStream >> mercExp;
        charInfo.mercenary.experience = mercExp;
        for (quint8 i = 1; i <= Enums::CharacterStats::MaxLevel; ++i)
            if (mercExp < static_cast<quint32>(i * i * (i + 1)))
            {
                charInfo.mercenary.level = i - 1;
                break;
            }
    }

    // Quests
    if (_saveFileContents.mid(Enums::Offsets::QuestsHeader, 4) != "Woo!")
    {
        ERROR_BOX(tr("Quests data not found!"));
        return false;
    }
    charInfo.questsInfo.clear();
    for (int i = 0; i < difficultiesNumber; ++i)
    {
        int baseOffset = Enums::Offsets::QuestsData + i * Enums::Quests::Size;
        charInfo.questsInfo.denOfEvil += static_cast<bool>(_saveFileContents.at(baseOffset + Enums::Quests::DenOfEvil) & Enums::Quests::IsCompleted);
        charInfo.questsInfo.radament += static_cast<bool>(_saveFileContents.at(baseOffset + Enums::Quests::Radament) & (Enums::Quests::IsCompleted | Enums::Quests::IsTaskDone));
        charInfo.questsInfo.lamEsensTome += static_cast<bool>(_saveFileContents.at(baseOffset + Enums::Quests::LamEsensTome) & Enums::Quests::IsCompleted);
        charInfo.questsInfo.izual += static_cast<bool>(_saveFileContents.at(baseOffset + Enums::Quests::Izual) & Enums::Quests::IsCompleted);
    }

    // WP
    if (_saveFileContents.mid(Enums::Offsets::WaypointsHeader, 2) != "WS")
    {
        ERROR_BOX(tr("Waypoint data not found!"));
        return false;
    }

    //inputDataStream.device->seek(Enums::Offsets::WaypointsData);
    //for (int i = 0; i < 3; ++i)
    //{
    //    inputDataStream.skipRawData(2);

    //    quint64 wpData;
    //    inputDataStream >> wpData;
    //    QString wpString = binaryStringFromNumber(wpData, true, 39);
    //    qDebug() << "wp data difficulty" << i << ":";
    //    int startPos = 0;
    //    for (int j = 0; j < 5; ++j)
    //    {
    //        int endPos = j == 3 ? 3 : 9;
    //        qDebug() << "act" << (j + 1) << ":" << wpString.mid(startPos, endPos);
    //        startPos += endPos;
    //    }

    //    inputDataStream.skipRawData(14);
    //}

    // NPC
    if (_saveFileContents.mid(Enums::Offsets::NPCHeader, 2) != "w4")
    {
        ERROR_BOX(tr("NPC data not found!"));
        return false;
    }

    // stats
    if (_saveFileContents.mid(Enums::Offsets::StatsHeader, 2) != "gf")
    {
        ERROR_BOX(tr("Stats data not found!"));
        return false;
    }
    inputDataStream.device()->seek(Enums::Offsets::StatsData);

    // find "if" header (skills)
    int skillsOffset = _saveFileContents.indexOf("if", Enums::Offsets::StatsData);
    if (skillsOffset == -1)
    {
        ERROR_BOX(tr("Skills data not found!"));
        return false;
    }
    charInfo.skillsOffset = skillsOffset;

    int statsSize = skillsOffset - Enums::Offsets::StatsData;
    QString statsBitData;
    statsBitData.reserve(statsSize * 8);
    for (int i = 0; i < statsSize; ++i)
    {
        quint8 aByte;
        inputDataStream >> aByte;
        statsBitData.prepend(binaryStringFromNumber(aByte));
    }

    // clear dynamic values
    QMetaEnum statisticMetaEnum = Enums::CharacterStats::statisticMetaEnum();
    for (int i = 0; i < statisticMetaEnum.keyCount(); ++i)
        charInfo.basicInfo.statsDynamicData.setProperty(statisticMetaEnum.key(i), QVariant());

    int count = 0; // to prevent infinite loop if something goes wrong
    int totalStats = 0;
    bool wasHackWarningShown = false;
    ReverseBitReader bitReader(statsBitData);
    for (; count < 20; ++count)
    {
        int statCode = bitReader.readNumber(Enums::CharacterStats::StatCodeLength);
        if (statCode == Enums::CharacterStats::End)
            break;

        int statLength = Enums::CharacterStats::statLengthFromValue(statCode);
        if (!statLength)
        {
            ERROR_BOX(tr("Unknown statistic code found: %1. This is not Median XL character.").arg(statCode));
            return false;
        }
        qint64 statValue = bitReader.readNumber(statLength);

        if (statCode == Enums::CharacterStats::Level && statValue != clvl)
        {
            ERROR_BOX(tr("Level in statistics (%1) isn't equal the one in header (%2).").arg(statValue).arg(clvl));
            return false;
        }
        else if (statCode >= Enums::CharacterStats::Life && statCode <= Enums::CharacterStats::BaseStamina)
            statValue >>= 8;
        else if (statCode == Enums::CharacterStats::SignetsOfLearningEaten && statValue > Enums::CharacterStats::SignetsOfLearningMax)
        {
            if (statValue > Enums::CharacterStats::SignetsOfLearningMax + 1 && !wasHackWarningShown)
            {
                WARNING_BOX(hackerDetected);
                wasHackWarningShown = true;
            }
            statValue = Enums::CharacterStats::SignetsOfLearningMax;
        }
        else if (statCode == Enums::CharacterStats::SignetsOfSkillEaten && statValue > Enums::CharacterStats::SignetsOfSkillMax)
        {
            if (statValue > Enums::CharacterStats::SignetsOfSkillMax + 1 && !wasHackWarningShown)
            {
                WARNING_BOX(hackerDetected);
                wasHackWarningShown = true;
            }
            statValue = Enums::CharacterStats::SignetsOfSkillMax;
        }
        else if (statCode >= Enums::CharacterStats::Strength && statCode <= Enums::CharacterStats::Vitality)
        {
            int baseStat = _baseStatsMap[charInfo.basicInfo.classCode].statsAtStart.statFromCode(static_cast<Enums::CharacterStats::StatisticEnum>(statCode));
            totalStats += statValue - baseStat;
        }
        else if (statCode == Enums::CharacterStats::FreeStatPoints)
            totalStats += statValue;

        charInfo.basicInfo.statsDynamicData.setProperty(Enums::CharacterStats::statisticNameFromValue(statCode), statValue);
    }
    if (count == 20)
    {
        ERROR_BOX(tr("Stats data is corrupted!"));
        return false;
    }

    int totalPossibleStats = totalPossibleStatPoints(charInfo.basicInfo.level);
    if (totalStats > totalPossibleStats) // check if stats are hacked
    {
        QMetaEnum statisticMetaEnum = Enums::CharacterStats::statisticMetaEnum();
        for (int i = Enums::CharacterStats::Strength; i <= Enums::CharacterStats::Vitality; ++i)
        {
            Enums::CharacterStats::StatisticEnum statCode = static_cast<Enums::CharacterStats::StatisticEnum>(i);
            int baseStat = _baseStatsMap[charInfo.basicInfo.classCode].statsAtStart.statFromCode(statCode);
            charInfo.basicInfo.statsDynamicData.setProperty(statisticMetaEnum.key(i), baseStat);
        }
        charInfo.basicInfo.statsDynamicData.setProperty("FreeStatPoints", totalPossibleStats);
        if (!wasHackWarningShown)
        {
            WARNING_BOX(hackerDetected);
            wasHackWarningShown = true;
        }
    }

    // skills
    quint16 skills = 0, maxPossibleSkills = totalPossibleSkillPoints();
    charInfo.basicInfo.skills.clear();
    charInfo.basicInfo.skills.reserve(skillsNumber);
    for (int i = 0; i < skillsNumber; ++i)
    {
        quint8 skillValue = _saveFileContents.at(skillsOffset + 2 + i);
        skills += skillValue;
        charInfo.basicInfo.skills += skillValue;
        //qDebug() << skillValue << ItemDataBase::Skills()->value(_characterSkillsIndeces[charInfo.basicInfo.classCode].first.at(i)).name;
    }
    skills += charInfo.basicInfo.statsDynamicData.property("FreeSkillPoints").toUInt();
    if (skills > maxPossibleSkills) // check if skills are hacked
    {
        skills = maxPossibleSkills;
        charInfo.basicInfo.statsDynamicData.setProperty("FreeSkillPoints", skills);
        _saveFileContents.replace(charInfo.skillsOffset + 2, skillsNumber, QByteArray(skillsNumber, 0));
        if (!wasHackWarningShown)
        {
            WARNING_BOX(hackerDetected);
            wasHackWarningShown = true;
        }
    }
    charInfo.basicInfo.totalSkillPoints = skills;

    const QPair<QList<int>, QList<int> > &skillsIndeces = _characterSkillsIndeces[charInfo.basicInfo.classCode];
    charInfo.basicInfo.skillsReadable.clear();
    charInfo.basicInfo.skillsReadable.reserve(skillsNumber);
    for (int i = 0; i < skillsNumber; ++i)
    {
        int skillIndex = skillsIndeces.second.at(i);
        charInfo.basicInfo.skillsReadable += charInfo.basicInfo.skills.at(skillsIndeces.first.indexOf(skillIndex));
        //qDebug() << charInfo.basicInfo.skillsReadable.last() << ItemDataBase::Skills()->value(skillIndex).name;
    }

    // items
    inputDataStream.skipRawData(skillsNumber + 2);
    int charItemsOffset = inputDataStream.device()->pos();
    if (_saveFileContents.mid(charItemsOffset, 2) != ItemParser::itemHeader)
    {
        ERROR_BOX(tr("Items data not found!"));
        return false;
    }
    charInfo.itemsOffset = charItemsOffset + 2;
    inputDataStream.skipRawData(2); // pointing to the beginning of item data

    quint16 charItemsTotal;
    inputDataStream >> charItemsTotal;
    quint32 avoidValue = 0;
    QString corruptedItems;
    ItemsList itemsBuffer;
    for (int i = 0; i < charItemsTotal; ++i)
    {
        ItemInfo *item = ItemParser::parseItem(inputDataStream, _saveFileContents);
        itemsBuffer += item;

        if (item->status != ItemInfo::Ok)
            corruptedItems += itemStorageAndCoordinatesString(tr("Corrupted item detected in %1 at (%2,%3)"), item) + "\n";

        int avoidKey = Enums::ItemProperties::Avoid1;
        if (item->location == Enums::ItemLocation::Equipped || (item->storage == Enums::ItemStorage::Inventory && ItemDataBase::isUberCharm(item)))
        {
            avoidValue += item->props.value(avoidKey).value + item->rwProps.value(avoidKey).value;
            foreach (ItemInfo *socketableItem, item->socketablesInfo)
                avoidValue += socketableItem->props.value(avoidKey).value + socketableItem->rwProps.value(avoidKey).value;
        }
    }
    if (avoidValue >= 100)
    {
        QString avoidText = tr("100% avoid is kewl");
        if (avoidValue > 100)
            avoidText += QString(" (%1)").arg(tr("well, you have %1% actually", "avoid").arg(avoidValue));
        WARNING_BOX(avoidText);
    }
    if (!corruptedItems.isEmpty())
        ERROR_BOX(corruptedItems.trimmed());
    charInfo.itemsEndOffset = inputDataStream.device()->pos();

    const int itemListTerminatorSize = 4, maxCorpses = 15;
    char itemListTerminator[itemListTerminatorSize] = {'J', 'M', 0, 0};
    int itemListTerminatorOffset;
    charInfo.items.corpses = 0;
    do
    {
        itemListTerminatorOffset = _saveFileContents.indexOf(QByteArray(itemListTerminator, itemListTerminatorSize), charItemsOffset);
        if (itemListTerminatorOffset != -1)
            break;
        itemListTerminator[2] = ++charInfo.items.corpses;
    } while (charInfo.items.corpses <= maxCorpses);

    if (itemListTerminatorOffset == -1)
    {
        ERROR_BOX(tr("Items list doesn't have a terminator!"));
        return false;
    }
    // TODO: this calculation is wrong
    itemListTerminatorOffset += itemListTerminatorSize + 4 + 12; // JM0100 + unknown

    int mercItemsOffset = _saveFileContents.lastIndexOf("jf");
    if (mercItemsOffset == -1)
    {
        ERROR_BOX(tr("Mercenary items section not found!"));
        return false;
    }
    mercItemsOffset += 2;

    int eof = _saveFileContents.lastIndexOf("kf");
    if (eof != _saveFileContents.size() - 3 || _saveFileContents.at(_saveFileContents.size() - 1) != 0)
    {
        ERROR_BOX(tr("Save file is not terminated correctly!"));
        return false;
    }

    bool isMercItemListBad = charInfo.mercenary.exists ^ (mercItemsOffset != eof);
    if (isMercItemListBad)
    {
        ERROR_BOX(tr("Mercenary items data is corrupted!"));
        return false;
    }

    if (charInfo.items.corpses)
    {
        inputDataStream.device()->seek(itemListTerminatorOffset);
        // process corpse items
    }

    if (charInfo.mercenary.exists)
    {
        // process merc items
        mercItemsOffset += 2;
        quint16 mercItemsTotal;
        inputDataStream.device()->seek(mercItemsOffset);
        inputDataStream >> mercItemsTotal;
    }

    // parse plugy stashes
    QString oldSharedStashPath = _plugyStashesHash[Enums::ItemStorage::SharedStash].path, oldHCStashPath = _plugyStashesHash[Enums::ItemStorage::HCStash].path;

    QFileInfo charPathFileInfo(charPath);
    QString canonicalCharPath = charPathFileInfo.canonicalPath();
    _plugyStashesHash[Enums::ItemStorage::PersonalStash].path = ui.actionAutoOpenPersonalStash->isChecked() ? QString("%1/%2.d2x").arg(canonicalCharPath, charPathFileInfo.baseName()) : QString();
    _plugyStashesHash[Enums::ItemStorage::SharedStash].path = ui.actionAutoOpenSharedStash->isChecked() ? canonicalCharPath + "/_LOD_SharedStashSave.sss" : QString();
    _plugyStashesHash[Enums::ItemStorage::HCStash].path = ui.actionAutoOpenHCShared->isChecked() ? canonicalCharPath + "/_LOD_HC_SharedStashSave.sss" : QString();

    bool sharedStashPathChanged = oldSharedStashPath != _plugyStashesHash[Enums::ItemStorage::SharedStash].path, hcStashPathChanged = oldHCStashPath != _plugyStashesHash[Enums::ItemStorage::HCStash].path;
    if (ui.actionReloadSharedStashes->isChecked())
        sharedStashPathChanged = hcStashPathChanged = true;
    _sharedGold = 0;
    for (QHash<Enums::ItemStorage::ItemStorageEnum, PlugyStashInfo>::iterator iter = _plugyStashesHash.begin(); iter != _plugyStashesHash.end(); ++iter)
    {
        switch (iter.key())
        {
        case Enums::ItemStorage::PersonalStash:
            if (!(_plugyStashesHash[iter.key()].exists = ui.actionAutoOpenPersonalStash->isChecked()))
                continue;
            break;
        case Enums::ItemStorage::SharedStash:
            if (!(_plugyStashesHash[iter.key()].exists = ui.actionAutoOpenSharedStash->isChecked()) || !sharedStashPathChanged)
                continue;
            break;
        case Enums::ItemStorage::HCStash:
            if (!(_plugyStashesHash[iter.key()].exists = ui.actionAutoOpenHCShared->isChecked()) || !hcStashPathChanged)
                continue;
            break;
        default:
            break;
        }
        processPlugyStash(iter, &itemsBuffer);
    }

    clearItems(sharedStashPathChanged, hcStashPathChanged);
    charInfo.items.character += itemsBuffer;

    return true;
}

quint32 MedianXLOfflineTools::checksum(const QByteArray &charByteArray) const
{
    quint32 sum = 0;
    for (int i = 0; i < charByteArray.size(); ++i)
    {
        bool mostSignificantByte = sum & 0x80000000;
        sum <<= 1;
        sum += mostSignificantByte;
        sum &= 0xFFFFFFFF;
        if (i < 12 || i > 15) // bytes 12-15 - file checksum
            sum += static_cast<quint8>(charByteArray.at(i));
    }
    return sum;
}

inline int MedianXLOfflineTools::totalPossibleStatPoints(int level)
{
    return (level - 1) * statPointsPerLevel + 5 * CharacterInfo::instance().questsInfo.lamEsensTomeQuestsCompleted() + CharacterInfo::instance().basicInfo.statsDynamicData.property("SignetsOfLearningEaten").toInt();
}

inline int MedianXLOfflineTools::totalPossibleSkillPoints()
{
    const CharacterInfo &charInfo = CharacterInfo::instance();
    return (charInfo.basicInfo.level - 1) * skillPointsPerLevel + charInfo.questsInfo.denOfEvilQuestsCompleted() + charInfo.questsInfo.radamentQuestsCompleted() + charInfo.questsInfo.izualQuestsCompleted() * 2 +
        CharacterInfo::instance().basicInfo.statsDynamicData.property("SignetsOfSkillEaten").toInt();
}

int MedianXLOfflineTools::investedStatPoints()
{
    quint16 sum = 0;
    for (quint8 i = Enums::CharacterStats::Strength; i <= Enums::CharacterStats::Vitality; ++i)
    {
        Enums::CharacterStats::StatisticEnum ienum = static_cast<Enums::CharacterStats::StatisticEnum>(i);
        sum += _spinBoxesStatsMap[ienum]->value() - _baseStatsMap[CharacterInfo::instance().basicInfo.classCode].statsAtStart.statFromCode(ienum);
    }
    return sum;
}

void MedianXLOfflineTools::recalculateStatPoints()
{
    CharacterInfo::instance().basicInfo.totalStatPoints = investedStatPoints() + ui.freeStatPointsLineEdit->text().toUInt();
}

void MedianXLOfflineTools::processPlugyStash(QHash<Enums::ItemStorage::ItemStorageEnum, PlugyStashInfo>::iterator &iter, ItemsList *items)
{
    PlugyStashInfo &info = iter.value();
    QFile inputFile(info.path);
    if (!(info.exists = inputFile.exists()))
        return;
    if (!(info.exists = inputFile.open(QIODevice::ReadOnly)))
    {
        showErrorMessageBoxForFile(tr("Error opening PlugY stash '%1'"), inputFile);
        return;
    }

    QByteArray bytes = inputFile.readAll();
    inputFile.close();

    QByteArray header;
    const int headerSize = 5;
    if (iter.key() == Enums::ItemStorage::PersonalStash)
        header = QByteArray("CSTM0");
    else
    {
        char sharedStashHeader[] = {'S', 'S', 'S', '\0', '0'};
        header = QByteArray(sharedStashHeader, headerSize);
    }
    if (bytes.left(headerSize) != header)
    {
        ERROR_BOX(tr("PlugY stash '%1' has wrong header").arg(QFileInfo(info.path).fileName()));
        return;
    }
    info.header = header;

    QDataStream inputDataStream(bytes);
    inputDataStream.setByteOrder(QDataStream::LittleEndian);
    inputDataStream.skipRawData(headerSize);

    inputDataStream >> info.version;
    if ((info.hasGold = (bytes.mid(headerSize + 1 + 4, 2) != ItemParser::plugyPageHeader)))
        inputDataStream >> info.gold;
    if (iter.key() == Enums::ItemStorage::SharedStash)
        _sharedGold = info.gold;

    QString corruptedItems;
    inputDataStream >> info.lastPage;
    for (quint32 page = 1; page <= info.lastPage; ++page)
    {
        if (bytes.mid(inputDataStream.device()->pos(), 2) != ItemParser::plugyPageHeader)
        {
            ERROR_BOX(tr("Page %1 of '%2' has wrong PlugY header").arg(page).arg(QFileInfo(info.path).fileName()));
            return;
        }
        inputDataStream.skipRawData(3);
        if (bytes.mid(inputDataStream.device()->pos(), 2) != ItemParser::itemHeader)
        {
            ERROR_BOX(tr("Page %1 of '%2' has wrong item header").arg(page).arg(QFileInfo(info.path).fileName()));
            return;
        }
        inputDataStream.skipRawData(2);

        quint16 itemsOnPage;
        inputDataStream >> itemsOnPage;
        for (int i = 0; i < itemsOnPage; ++i)
        {
            ItemInfo *item = ItemParser::parseItem(inputDataStream, bytes);
            item->storage = iter.key();
            item->plugyPage = page;
            items->append(item);

            if (item->status != ItemInfo::Ok)
                corruptedItems += itemStorageAndCoordinatesString(tr("Corrupted item detected in %1 on page %4 at (%2,%3)"), item) + "\n";
        }
    }
    if (!corruptedItems.isEmpty())
        ERROR_BOX(corruptedItems.trimmed());
}

void MedianXLOfflineTools::clearUI()
{
    _isLoaded = false;
    _resurrectionPenalty = ResurrectPenaltyDialog::Nothing;

    QList<QLineEdit *> lineEdits = QList<QLineEdit *>() << ui.classLineEdit << ui.titleLineEdit << ui.mercLevelLineEdit;
    foreach (QLineEdit *lineEdit, lineEdits)
        lineEdit->clear();
    foreach (QLineEdit *lineEdit, _lineEditsStatsMap)
    {
        lineEdit->clear();
        lineEdit->setStatusTip(QString());
    }
    ui.charNamePreview->clear();

    QList<QComboBox *> comboBoxes = QList<QComboBox *>()/* << ui.currentDifficultyComboBox*/ << ui.mercNameComboBox << ui.mercTypeComboBox;
    foreach (QComboBox *combobx, comboBoxes)
        combobx->clear();

    foreach (QSpinBox *spinBox, _spinBoxesStatsMap)
    {
        spinBox->setMinimum(0);
        spinBox->setValue(0);
        spinBox->setStatusTip(QString());
    }
    ui.levelSpinBox->setValue(1);
    //ui.currentActSpinBox->setValue(1);

    QList<QCheckBox *> checkBoxes = QList<QCheckBox *>() << ui.convertToSoftcoreCheckBox << ui.activateWaypointsCheckBox << ui.respecSkillsCheckBox;
    foreach (QCheckBox *checkbox, checkBoxes)
        checkbox->setChecked(false);
    ui.respecStatsButton->setChecked(false);

    QList<QGroupBox *> groupBoxes = QList<QGroupBox *>() << ui.characterGroupBox << ui.statsGroupBox << ui.mercGroupBox;
    foreach (QGroupBox *groupBox, groupBoxes)
        groupBox->setDisabled(true);

    QList<QAction *> actions = QList<QAction *>() << ui.actionReloadCharacter << ui.actionRespecStats << ui.actionRespecSkills << ui.actionActivateWaypoints << ui.actionSaveCharacter << ui.actionRename
                                                  << ui.actionReloadCharacter << ui.actionFind << ui.actionFindNext << ui.actionFindPrevious << ui.actionShowItems << ui.actionSkillPlan << ui.actionExportCharacterInfo;
    foreach (QAction *action, actions)
        action->setDisabled(true);

    for (int i = 0; i < ui.statsTableWidget->rowCount(); ++i)
        for (int j = 0; j < ui.statsTableWidget->columnCount(); ++j)
        {
            QTableWidgetItem *item = ui.statsTableWidget->item(i, j);
            if (!item)
            {
                item = new QTableWidgetItem;
                item->setTextAlignment(Qt::AlignCenter);
                ui.statsTableWidget->setItem(i, j, item);
            }
            item->setText(QString());
        }
}

void MedianXLOfflineTools::updateUI()
{
    clearUI();

    QList<QAction *> actions = QList<QAction *>() << ui.actionRespecStats << ui.actionRespecSkills << ui.actionActivateWaypoints << ui.actionSaveCharacter
                                                  << ui.actionRename << ui.actionSkillPlan << ui.actionExportCharacterInfo;
    foreach (QAction *action, actions)
        action->setEnabled(true);
    ui.respecSkillsCheckBox->setEnabled(true);

    QList<QGroupBox *> groupBoxes = QList<QGroupBox *>() << ui.characterGroupBox << ui.statsGroupBox;
    foreach (QGroupBox *groupBox, groupBoxes)
        groupBox->setEnabled(true);

    const CharacterInfo &charInfo = CharacterInfo::instance();
    QD2CharRenamer::updateNamePreview(ui.charNamePreview, charInfo.basicInfo.originalName);

    ui.hardcoreGroupBox->setEnabled(charInfo.basicInfo.isHardcore);
    if (charInfo.basicInfo.isHardcore)
        updateHardcoreUIElements();

    ui.classLineEdit->setText(Enums::ClassName::classes().at(charInfo.basicInfo.classCode));
    updateCharacterTitle(charInfo.basicInfo.isHardcore);

    //ui.currentDifficultyComboBox->setEnabled(true);
    //ui.currentDifficultyComboBox->addItems(difficulties.mid(0, titleAndMaxDifficulty.second + 1));
    //ui.currentDifficultyComboBox->setCurrentIndex(charInfo.basicInfo.currentDifficulty);
    //ui.currentActSpinBox->setValue(charInfo.basicInfo.currentAct);

    _oldClvl = charInfo.basicInfo.level;
    ui.levelSpinBox->setMaximum(_oldClvl);
    ui.levelSpinBox->setValue(_oldClvl);

    setStats();
    recalculateStatPoints();

    int stats = charInfo.basicInfo.totalStatPoints, skills = charInfo.basicInfo.totalSkillPoints;
    updateStatusTips(stats, stats - CharacterInfo::instance().basicInfo.statsDynamicData.property("FreeStatPoints").toInt(), skills, skills - CharacterInfo::instance().basicInfo.statsDynamicData.property("FreeSkillPoints").toInt());
    ui.signetsOfLearningEatenLineEdit->setStatusTip(maxValueFormat.arg(Enums::CharacterStats::SignetsOfLearningMax));
    ui.signetsOfSkillEatenLineEdit->setStatusTip(maxValueFormat.arg(Enums::CharacterStats::SignetsOfSkillMax));
    ui.stashGoldLineEdit->setStatusTip(maxValueFormat.arg(QLocale().toString(Enums::CharacterStats::StashGoldMax)));
    if (_sharedGold)
        ui.stashGoldLineEdit->setStatusTip(ui.stashGoldLineEdit->statusTip() + ", " + tr("Shared: %1", "amount of gold in shared stash").arg(QLocale().toString(_sharedGold)));

    if (charInfo.mercenary.exists)
    {
        int mercTypeIndex;
        QPair<int, int> mercArraySlice = Enums::Mercenary::allowedTypesForMercCode(charInfo.mercenary.code, &mercTypeIndex);
        ui.mercTypeComboBox->addItems(Enums::Mercenary::types().mid(mercArraySlice.first, mercArraySlice.second));
        ui.mercTypeComboBox->setCurrentIndex(mercTypeIndex);

        ui.mercNameComboBox->addItems(mercNames.at(Enums::Mercenary::mercNamesIndexFromCode(charInfo.mercenary.code)));
        ui.mercNameComboBox->setCurrentIndex(charInfo.mercenary.nameIndex);

        ui.mercLevelLineEdit->setText(QString::number(charInfo.mercenary.level));

        ui.mercGroupBox->setEnabled(true);
    }

    bool hasItems = !charInfo.items.character.isEmpty();
    ui.actionShowItems->setEnabled(hasItems);
    ui.actionFind->setEnabled(hasItems);
    ui.actionGiveCube->setDisabled(CharacterInfo::instance().items.hasCube());
    updateWindowTitle();

    _isLoaded = true;
}

void MedianXLOfflineTools::updateHardcoreUIElements()
{
    bool hadDied = CharacterInfo::instance().basicInfo.hadDied;
    ui.convertToSoftcoreCheckBox->setDisabled(hadDied);
    ui.actionConvertToSoftcore->setDisabled(hadDied);
    ui.resurrectButton->setEnabled(hadDied);
    ui.actionResurrect->setEnabled(hadDied);
}

void MedianXLOfflineTools::updateCharacterTitle(bool isHardcore)
{
    const CharacterInfo::CharacterInfoBasic &basicInfo = CharacterInfo::instance().basicInfo;
    bool isMale = basicInfo.classCode >= Enums::ClassName::Necromancer && basicInfo.classCode <= Enums::ClassName::Druid;
    quint8 progression = basicInfo.titleCode;
    QPair<QString, quint8> titleAndMaxDifficulty = Enums::Progression::titleNameAndMaxDifficultyFromValue(progression, isMale, isHardcore);
    QString newTitle = titleAndMaxDifficulty.first;
    if (basicInfo.isHardcore && basicInfo.hadDied)
        newTitle += QString(" (%1)").arg(tr("DEAD", "HC character is dead"));
    ui.titleLineEdit->setText(newTitle);
    ui.titleLineEdit->setStyleSheet(QString("%1; color: %2;").arg(readonlyCss, isHardcore ? "red" : "black"));
}

void MedianXLOfflineTools::setStats()
{
    QMetaEnum statisticMetaEnum = Enums::CharacterStats::statisticMetaEnum();
    for (int i = 0; i < statisticMetaEnum.keyCount(); ++i)
    {
        const char *name = statisticMetaEnum.key(i);
        qulonglong value = CharacterInfo::instance().basicInfo.statsDynamicData.property(name).toULongLong();
        Enums::CharacterStats::StatisticEnum statCode = static_cast<Enums::CharacterStats::StatisticEnum>(statisticMetaEnum.value(i));

        if (statCode >= Enums::CharacterStats::Strength && statCode <= Enums::CharacterStats::Vitality)
        {
            _oldStatValues[statCode] = value;

            quint8 baseValue = _baseStatsMap[CharacterInfo::instance().basicInfo.classCode].statsAtStart.statFromCode(statCode);
            QSpinBox *spinBox = _spinBoxesStatsMap[statCode];
            spinBox->setMinimum(baseValue);
            spinBox->setMaximum(value);
            spinBox->setValue(value);
        }
        else if (statCode >= Enums::CharacterStats::Life && statCode <= Enums::CharacterStats::BaseStamina)
        {
            int j = statCode - Enums::CharacterStats::Life, row = j / 2, col = j % 2;
            QTableWidgetItem *item = ui.statsTableWidget->item(row, col);
            if (!item)
            {
                item = new QTableWidgetItem;
                item->setTextAlignment(Qt::AlignCenter);
                ui.statsTableWidget->setItem(row, col, item);
            }
            item->setText(QString::number(value));
        }
        else if (statCode != Enums::CharacterStats::End && statCode != Enums::CharacterStats::Level && statCode != Enums::CharacterStats::Experience)
        {
            _lineEditsStatsMap[statCode]->setText(QString::number(value));
        }
    }

    int freeStats = ui.freeStatPointsLineEdit->text().toInt();
    if (freeStats)
        foreach (QSpinBox *spinBox, _spinBoxesStatsMap)
            spinBox->setMaximum(spinBox->maximum() + freeStats);
}

void MedianXLOfflineTools::updateWindowTitle()
{
    _charPathLabel->setText(QDir::toNativeSeparators(_charPath));
    // making setWindowFilePath() work correctly
#ifdef Q_WS_MACX
    setWindowTitle(_charPath.isEmpty() ? QString() : QString("%1 (%2)").arg(QFileInfo(_charPath).fileName(), SkillplanDialog::modVersionReadable()));
#else
    setWindowTitle(_charPath.isEmpty() ? qApp->applicationName() : QString("%1[*] (%2) %3 %4").arg(QFileInfo(_charPath).fileName(), SkillplanDialog::modVersionReadable(), QChar(0x2014), qApp->applicationName()));
#endif
}

void MedianXLOfflineTools::updateTableStats(const BaseStats::StatsStep &statsPerStep, int diff, QSpinBox *senderSpinBox /*= 0*/)
{
    if (!senderSpinBox || (senderSpinBox && senderSpinBox == ui.vitalitySpinBox))
    {
        updateTableItemStat(ui.statsTableWidget->item(0, 0), diff, statsPerStep.life); // current life
        updateTableItemStat(ui.statsTableWidget->item(0, 1), diff, statsPerStep.life); // base life
        updateTableItemStat(ui.statsTableWidget->item(2, 0), diff, statsPerStep.stamina); // current stamina
        updateTableItemStat(ui.statsTableWidget->item(2, 1), diff, statsPerStep.stamina); // base stamina
    }
    if (!senderSpinBox || (senderSpinBox && senderSpinBox == ui.energySpinBox))
    {
        updateTableItemStat(ui.statsTableWidget->item(1, 0), diff, statsPerStep.mana); // current mana
        updateTableItemStat(ui.statsTableWidget->item(1, 1), diff, statsPerStep.mana); // base mana
    }
}

void MedianXLOfflineTools::updateTableItemStat(QTableWidgetItem *item, int diff, int statPerPoint)
{
    double newValue = item->text().toDouble() + diff * statPerPoint / 4.0;
    if (newValue > 8191)
        newValue = 8191;

    if (newValue > 0)
    {
        double foo;
        item->setText(modf(newValue, &foo) ? QString::number(newValue, 'f', 1) : QString::number(newValue));
    }
    else
        item->setText("0");
}

void MedianXLOfflineTools::updateStatusTips(int newStatPoints, int investedStatPoints, int newSkillPoints, int investedSkillPoints)
{
    updateMaxCompoundStatusTip(ui.freeStatPointsLineEdit, newStatPoints, investedStatPoints);
    updateMaxCompoundStatusTip(ui.freeSkillPointsLineEdit, newSkillPoints, investedSkillPoints);
    ui.inventoryGoldLineEdit->setStatusTip(maxValueFormat.arg(QLocale().toString(_oldClvl * Enums::CharacterStats::InventoryGoldFactor)));
}

void MedianXLOfflineTools::updateCompoundStatusTip(QWidget *widget, const QString &firstString, const QString &secondString)
{
    widget->setStatusTip(compoundFormat.arg(firstString, secondString));
}

void MedianXLOfflineTools::updateMinCompoundStatusTip(QWidget *widget, int minValue, int investedValue)
{
    updateCompoundStatusTip(widget, minValueFormat.arg(minValue), investedValueFormat.arg(investedValue));
}

void MedianXLOfflineTools::updateMaxCompoundStatusTip(QWidget *widget, int maxValue, int investedValue)
{
    updateCompoundStatusTip(widget, maxValueFormat.arg(maxValue), investedValueFormat.arg(investedValue));
}

QByteArray MedianXLOfflineTools::statisticBytes()
{
    QString result;
    QMetaEnum statisticMetaEnum = Enums::CharacterStats::statisticMetaEnum();
    bool isExpAndLevelNotSet = true;
    for (int i = 0; i < statisticMetaEnum.keyCount(); ++i)
    {
        const char *enumKey = statisticMetaEnum.key(i);
        Enums::CharacterStats::StatisticEnum statCode = static_cast<Enums::CharacterStats::StatisticEnum>(statisticMetaEnum.value(i));
        qulonglong value = 0;
        if (statCode >= Enums::CharacterStats::Strength && statCode <= Enums::CharacterStats::Vitality)
        {
            value = _spinBoxesStatsMap[statCode]->value();
        }
        else if (statCode >= Enums::CharacterStats::Life && statCode <= Enums::CharacterStats::BaseStamina)
        {
            int j = statCode - Enums::CharacterStats::Life, row = j / 2, col = j % 2;
            value = static_cast<qulonglong>(ui.statsTableWidget->item(row, col)->text().toDouble()) << 8;
        }
        else if (statCode != Enums::CharacterStats::End && statCode != Enums::CharacterStats::Level && statCode != Enums::CharacterStats::Experience)
        {
            value = _lineEditsStatsMap[statCode]->text().toULongLong();
            // signets should be set to (max + 1)
            if ((statCode == Enums::CharacterStats::SignetsOfLearningEaten && value == Enums::CharacterStats::SignetsOfLearningMax) ||
                (statCode == Enums::CharacterStats::SignetsOfSkillEaten    && value == Enums::CharacterStats::SignetsOfSkillMax))
            {
                ++value;
            }
            else if (statCode == Enums::CharacterStats::FreeStatPoints)
            {
                int totalPossibleFreeStats = totalPossibleStatPoints(ui.levelSpinBox->value()) - investedStatPoints();
                if (value > static_cast<quint32>(totalPossibleFreeStats)) // prevent hacks and shut the compiler up
                {
                    value = totalPossibleFreeStats;
                    WARNING_BOX(hackerDetected);
                }
            }
        }
        else if (statCode != Enums::CharacterStats::End && isExpAndLevelNotSet) // level or exp
        {
            quint8 clvl = CharacterInfo::instance().basicInfo.level, newClvl = ui.levelSpinBox->value();
            if (clvl != newClvl) // set new level and experience explicitly
            {
                addStatisticBits(result, Enums::CharacterStats::Level, Enums::CharacterStats::StatCodeLength);
                addStatisticBits(result, newClvl, Enums::CharacterStats::statLengthFromValue(Enums::CharacterStats::Level));
                CharacterInfo::instance().basicInfo.statsDynamicData.setProperty("Level", newClvl);

                quint32 newExp = experienceTable.at(newClvl - 1);
                if (newExp) // must not be present for level 1 character
                {
                    addStatisticBits(result, Enums::CharacterStats::Experience, Enums::CharacterStats::StatCodeLength);
                    addStatisticBits(result, newExp, Enums::CharacterStats::statLengthFromValue(Enums::CharacterStats::Experience));
                }
                CharacterInfo::instance().basicInfo.statsDynamicData.setProperty("Experience", newExp);

                isExpAndLevelNotSet = false;
                continue;
            }
            else
                value = CharacterInfo::instance().basicInfo.statsDynamicData.property(enumKey).toULongLong();
        }
        else if (statCode == Enums::CharacterStats::End) // byte align
        {
            addStatisticBits(result, statCode, 16 - result.length() % 8);
            break; // not necessary actually
        }
        else
            continue;

        if (value)
        {
            addStatisticBits(result, statCode, Enums::CharacterStats::StatCodeLength);
            addStatisticBits(result, value, Enums::CharacterStats::statLengthFromValue(statCode));
        }

        CharacterInfo::instance().basicInfo.statsDynamicData.setProperty(enumKey, value);
    }

    int bitsCount = result.length();
    if (bitsCount % 8)
    {
        ERROR_BOX(tr("Stats string is not byte aligned!"));
        return QByteArray();
    }

    QByteArray resultBytes;
    resultBytes.reserve(bitsCount / 8);
    for (int startPos = bitsCount - 8; startPos >= 0; startPos -= 8)
    {
        quint8 aByte = result.mid(startPos, 8).toUShort(0, 2);
        resultBytes += aByte;
    }
    return resultBytes;
}

void MedianXLOfflineTools::addStatisticBits(QString &bitsString, quint64 number, int fieldWidth)
{
    bitsString.prepend(binaryStringFromNumber(number, false, fieldWidth));
}

void MedianXLOfflineTools::clearItems(bool sharedStashPathChanged /*= true*/, bool hcStashPathChanged /*= true*/)
{
    QMutableListIterator<ItemInfo *> itemIterator(CharacterInfo::instance().items.character);
    while (itemIterator.hasNext())
    {
        ItemInfo *item = itemIterator.next();
        switch (item->storage)
        {
        case Enums::ItemStorage::SharedStash:
            if ((sharedStashPathChanged || ui.actionAutoOpenSharedStash->isChecked()) && !sharedStashPathChanged)
                continue;
            break;
        case Enums::ItemStorage::HCStash:
            if ((hcStashPathChanged || ui.actionAutoOpenHCShared->isChecked()) && !hcStashPathChanged)
                continue;
            break;
        default:
            break;
        }

        qDeleteAll(item->socketablesInfo);
        delete item;
        itemIterator.remove();
    }
}

void MedianXLOfflineTools::backupFile(QFile &file)
{
    if (ui.actionBackup->isChecked())
    {
        QFile backupFile(file.fileName() + "." + backupExtension);
        if (backupFile.exists() && !backupFile.remove())
            showErrorMessageBoxForFile(tr("Error removing old backup '%1'"), backupFile);
        else
        {
            if (!file.copy(backupFile.fileName()))
                showErrorMessageBoxForFile(tr("Error creating backup of '%1'"), file);
        }
    }
}

QHash<int, bool> MedianXLOfflineTools::getPlugyStashesExistenceHash() const
{
    QHash<int, bool> plugyStashesExistenceHash;
    for (QHash<Enums::ItemStorage::ItemStorageEnum, PlugyStashInfo>::const_iterator iter = _plugyStashesHash.constBegin(); iter != _plugyStashesHash.constEnd(); ++iter)
        plugyStashesExistenceHash[iter.key()] = iter.value().exists;
    return plugyStashesExistenceHash;
}

void MedianXLOfflineTools::showErrorMessageBoxForFile(const QString &message, const QFile &file)
{
    CUSTOM_BOX_OK(critical, message.arg(QDir::toNativeSeparators(file.fileName())) + "\n" + tr("Reason: %1", "error with file").arg(file.errorString()));
}

QString MedianXLOfflineTools::itemStorageAndCoordinatesString(const QString &text, ItemInfo *item)
{
    return text.arg(ItemsViewerDialog::tabNameAtIndex(ItemsViewerDialog::tabIndexFromItemStorage(item->storage))).arg(item->row + 1).arg(item->column + 1).arg(item->plugyPage);
}

bool MedianXLOfflineTools::maybeSave()
{
    if (isWindowModified())
    {
        QMessageBox msgBox(QMessageBox::Warning, qApp->applicationName(), tr("Character has been modified."), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, this);
        msgBox.setInformativeText(tr("Do you want to save your changes?"));
        msgBox.setDefaultButton(QMessageBox::Save);
        msgBox.setWindowModality(Qt::WindowModal);
        switch (msgBox.exec())
        {
        case QMessageBox::Save:
            saveCharacter();
            break;
        case QMessageBox::Cancel:
            return false;
        case QMessageBox::Discard:
            wasModified(false);
            break;
        default:
            break;
        }
    }
    return true;
}
