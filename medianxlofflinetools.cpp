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

#include <QCloseEvent>
#include <QFormLayout>
#include <QFileDialog>
#include <QLabel>

#include <QSettings>
#include <QFile>
#include <QDataStream>
#include <QTranslator>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif

#include <cmath>


static const QString lastSavePathKey("lastSavePath"), releaseDate("17.01.2012");

//#define MAKE_HC


// static const

const QString MedianXLOfflineTools::compoundFormat("%1, %2");
const quint32 MedianXLOfflineTools::fileSignature = 0xAA55AA55;
const int MedianXLOfflineTools::skillsNumber = 30;
const int MedianXLOfflineTools::difficultiesNumber = 3;
const int MedianXLOfflineTools::statPointsPerLevel = 5;
const int MedianXLOfflineTools::skillPointsPerLevel = 1;
const int MedianXLOfflineTools::maxRecentFiles = 5;


// ctor

MedianXLOfflineTools::MedianXLOfflineTools(QWidget *parent, Qt::WFlags flags) : QMainWindow(parent, flags), _findItemsDialog(0), hackerDetected(tr("1337 hacker detected! Please, play legit.")),
    //difficulties(QStringList() << tr("Hatred") << tr("Nightmare") << tr("Destruction")),
    maxValueFormat(tr("Max: %1")), minValueFormat(tr("Min: %1")), investedValueFormat(tr("Invested: %1")), _isLoaded(false)
{
    ui.setupUi(this);

    _exitSeparator = ui.menuFile->insertSeparator(ui.actionExit);
    createLanguageMenu();

    loadData();
    createLayout();
    loadSettings();
    fillMaps();
    connectSignals();
    
    ui.actionFindNext->setShortcut(QKeySequence::FindNext);
    ui.actionFindPrevious->setShortcut(QKeySequence::FindPrevious);

    //ui.signetsOfLearningEatenLineEdit->setReadOnly(false);
    //ui.freeSkillPointsLineEdit->setReadOnly(false);
    //ui.freeStatPointsLineEdit->setReadOnly(false);

    if (ui.actionLoadLastUsedCharacter->isChecked() && _recentFilesList.size())
        loadFile(_recentFilesList.at(0));
    else
        setWindowTitle(qApp->applicationName());
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

void MedianXLOfflineTools::loadCharacter()
{
    QSettings settings;
    settings.beginGroup("recentItems");
    QString lastSavePath = settings.value(lastSavePathKey).toString();

    QString charPath = QFileDialog::getOpenFileName(this, tr("Load Character"), lastSavePath, tr("Diablo 2 Save Files (*.d2s)"));
    if (loadFile(charPath))
        ui.statusBar->showMessage(tr("Character loaded"), 3000);
}

void MedianXLOfflineTools::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    QString fileName = QDir::fromNativeSeparators(action->statusTip());
    loadFile(fileName);
}

void MedianXLOfflineTools::reloadCharacter()
{
    if (ui.actionAskBeforeReload->isChecked())
    {
        QMessageBox box(QMessageBox::Question, qApp->applicationName(), tr("Do you really want to reload character? All unsaved changes will be lost."), QMessageBox::Yes | QMessageBox::No, this);
        if (box.exec() == QMessageBox::No)
            return;
    }
    
    if (loadFile(_charPath))
        ui.statusBar->showMessage(tr("Character reloaded"), 3000);
}

void MedianXLOfflineTools::saveCharacter()
{
    QByteArray statsBytes = statisticBytes();
    if (statsBytes.isEmpty())
        return;

    QByteArray tempFileContents(_saveFileContents);
    tempFileContents.replace(Enums::Offsets::StatsData, _editableCharInfo.skillsOffset - Enums::Offsets::StatsData, statsBytes);
    int diff = Enums::Offsets::StatsData + statsBytes.size() - _editableCharInfo.skillsOffset;
    _editableCharInfo.skillsOffset = Enums::Offsets::StatsData + statsBytes.size();
    _editableCharInfo.itemsOffset += diff;
    _editableCharInfo.itemsEndOffset += diff;

    if (ui.respecSkillsCheckBox->isChecked())
        tempFileContents.replace(_editableCharInfo.skillsOffset + 2, skillsNumber, QByteArray(skillsNumber, 0));

    if (ui.activateWaypointsCheckBox->isChecked())
    {
        QByteArray activatedWaypointsBytes(5, 0xFF); // 40x'1'
        for (int startPos = Enums::Offsets::WaypointsData + 2, i = 0; i < difficultiesNumber; ++i, startPos += 24)
            tempFileContents.replace(startPos, activatedWaypointsBytes.size(), activatedWaypointsBytes);
    }

    if (ui.convertToSoftcoreCheckBox->isChecked())
        _editableCharInfo.basicInfo.isHardcore = false;

#ifdef MAKE_HC
    _editableCharInfo.basicInfo.isHardcore = true;
    _editableCharInfo.basicInfo.hadDied = false;
#endif
    char statusValue = tempFileContents[Enums::Offsets::Status];
    if (_editableCharInfo.basicInfo.hadDied)
        statusValue |= Enums::StatusBits::HadDied;
    else
        statusValue &= ~Enums::StatusBits::HadDied;
    if (_editableCharInfo.basicInfo.isHardcore)
        statusValue |= Enums::StatusBits::IsHardcore;
    else
        statusValue &= ~Enums::StatusBits::IsHardcore;
    tempFileContents[Enums::Offsets::Status] = statusValue;

    QDataStream outputDataStream(&tempFileContents, QIODevice::ReadWrite);
    outputDataStream.setByteOrder(QDataStream::LittleEndian);

    /*quint8 curDiff[difficultiesNumber] = {0, 0, 0};
 curDiff[ui.currentDifficultyComboBox->currentIndex()] = 128 + ui.currentActSpinBox->value() - 1; // 10000xxx
 outputDataStream.skipRawData(Enums::Offsets::CurrentLocation);
 for (int i = 0; i < difficultiesNumber; ++i)
  outputDataStream << curDiff[i];*/

    /*outputDataStream.skipRawData(Enums::Offsets::Progression);
 outputDataStream << quint16(13); // set max act to 4 in Destruction
 outputDataStream.device()->reset();*/
    /*quint16 one = 1;
 outputDataStream.skipRawData(Enums::Offsets::QuestsData + 2*96 + 30); // Destruction A3
 outputDataStream << one;
 outputDataStream.skipRawData(14); // A4
 outputDataStream << one;
 outputDataStream.skipRawData(4); // A4Q3 (A4Q2 is skipRawData(6) for some reason)
 outputDataStream << one;         // is completed
 outputDataStream.skipRawData(2); // A5
 outputDataStream << one;
 outputDataStream.device()->reset();
 outputDataStream.skipRawData(Enums::Offsets::CurrentLocation + 2); // set current diff to Dest and act to 4
 quint8 v = 131; // 10000011
 outputDataStream << v;
 outputDataStream.device()->reset();*/

    QString newName = _editableCharInfo.basicInfo.newName;
    bool hasNameChanged = !newName.isEmpty() && _editableCharInfo.basicInfo.originalName != newName;
    if (hasNameChanged)
    {
        outputDataStream.device()->seek(Enums::Offsets::Name);
#if defined(Q_WS_MACX)
        QByteArray newNameByteArray = macTextCodec()->fromUnicode(newName);
#else
        QByteArray newNameByteArray = newName.toLocal8Bit();
#endif
        newNameByteArray += QByteArray(QD2CharRenamer::maxNameLength + 1 - newName.length(), '\0'); // add trailing nulls
        outputDataStream.writeRawData(newNameByteArray.constData(), newNameByteArray.length());
    }
    else
        newName = _editableCharInfo.basicInfo.originalName;

    quint8 newClvl = ui.levelSpinBox->value();
    if (_editableCharInfo.basicInfo.level != newClvl)
    {
        _editableCharInfo.basicInfo.level = newClvl;
        _editableCharInfo.basicInfo.totalSkillPoints = ui.freeSkillPointsLineEdit->text().toUShort();
        recalculateStatPoints();

        outputDataStream.device()->seek(Enums::Offsets::Level);
        outputDataStream << newClvl;

        ui.levelSpinBox->setMaximum(newClvl);
    }

    if (_editableCharInfo.mercenary.exists)
    {
        quint16 newMercValue = Enums::Mercenary::mercBaseValueFromCode(_editableCharInfo.mercenary.code) + ui.mercTypeComboBox->currentIndex();
        _editableCharInfo.mercenary.code = Enums::Mercenary::mercCodeFromValue(newMercValue);
        _editableCharInfo.mercenary.nameIndex = ui.mercNameComboBox->currentIndex();
        outputDataStream.device()->seek(Enums::Offsets::Mercenary + 4);
        outputDataStream << _editableCharInfo.mercenary.nameIndex << newMercValue;
    }

    int characterItemsSize = 2;
    ItemsList characterItems;
    QHash<Enums::ItemStorage::ItemStorageEnum, ItemsList > plugyItemsHash;
    foreach (ItemInfo *item, _editableCharInfo.items.character)
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
            ERROR_BOX_FILE(tr("Error creating file '%1'\nReason: %2"), inputFile);
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

    tempFileContents.replace(_editableCharInfo.itemsOffset, _editableCharInfo.itemsEndOffset - _editableCharInfo.itemsOffset, QByteArray(characterItemsSize, 0));
    outputDataStream.device()->seek(_editableCharInfo.itemsOffset);
    outputDataStream << static_cast<quint16>(characterItems.size());
    ItemParser::writeItems(characterItems, outputDataStream);

    quint32 fileSize = tempFileContents.size();
    outputDataStream.device()->seek(Enums::Offsets::FileSize);
    outputDataStream << fileSize;
    outputDataStream << checksum(tempFileContents);

    QSettings settings;
    settings.beginGroup("recentItems");
    QString savePath = settings.value(lastSavePathKey).toString(), fileName = savePath + "/" + newName, saveFileName = fileName + ".d2s";

    QFile outputFile(saveFileName);
    backupFile(outputFile);
    if (outputFile.open(QIODevice::WriteOnly))
    {
        int bytesWritten = outputFile.write(tempFileContents);
        if (bytesWritten == fileSize)
        {
            outputFile.flush();
            _saveFileContents = tempFileContents;

            if (hasNameChanged)
            {
                // delete .d2s and rename all other related files like .d2x, .key, .ma0, etc.
                bool isOldNameEmpty = QRegExp(QString("[ %1]+").arg(QChar(QChar::Nbsp))).exactMatch(_editableCharInfo.basicInfo.originalName);
                bool hasNonAsciiChars = false;
                for (int i = 0; i < _editableCharInfo.basicInfo.originalName.length(); ++i)
                    if (_editableCharInfo.basicInfo.originalName.at(i).unicode() > 255)
                    {
                        hasNonAsciiChars = true;
                        break;
                    }

                bool isStrangeName = hasNonAsciiChars || isOldNameEmpty;
                QDir sourceFileDir(savePath, isStrangeName ? "*" : _editableCharInfo.basicInfo.originalName + ".*");
                foreach (const QFileInfo &fileInfo, sourceFileDir.entryInfoList())
                {
                    if (isStrangeName && fileInfo.baseName() != _editableCharInfo.basicInfo.originalName)
                        continue;

                    QString extension = fileInfo.suffix();
                    QFile sourceFile(fileInfo.canonicalFilePath());
                    if (extension == "d2s") // delete
                    {
                        if (!sourceFile.remove())
                            ERROR_BOX_FILE(tr("Error removing file '%1'\nReason: %2"), sourceFile);
                    }
                    else // rename
                    {
                        if (!sourceFile.rename(fileName + "." + extension) && !isOldNameEmpty)
                            ERROR_BOX_FILE(tr("Error renaming file '%1'\nReason: %2"), sourceFile);
                    }
                }

                _editableCharInfo.basicInfo.originalName = newName;
                _editableCharInfo.basicInfo.newName.clear();

                _recentFilesList[0] = saveFileName;
                updateRecentFilesActions();
            }

            _charPath = saveFileName;
            loadFile(_charPath); // update all UI at once by reloading the file

            INFO_BOX(tr("File '%1' successfully saved!").arg(QDir::toNativeSeparators(saveFileName)));
        }
        else
            ERROR_BOX_FILE(tr("Error writing file '%1'\nReason: %2"), outputFile);
    }
    else
        ERROR_BOX_FILE(tr("Error creating file '%1'\nReason: %2"), outputFile);
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

        BaseStats::StatsPerPoint charStatsPerPoint = _baseStatsMap[_editableCharInfo.basicInfo.classCode].statsPerPoint;
        if (senderSpinBox == ui.vitalitySpinBox)
        {
            updateTableStats(ui.statsTableWidget->item(0, 0), diff, charStatsPerPoint.life); // current life
            updateTableStats(ui.statsTableWidget->item(0, 1), diff, charStatsPerPoint.life); // base life
            updateTableStats(ui.statsTableWidget->item(2, 0), diff, charStatsPerPoint.stamina); // current stamina
            updateTableStats(ui.statsTableWidget->item(2, 1), diff, charStatsPerPoint.stamina); // base stamina
        }
        else if (senderSpinBox == ui.energySpinBox)
        {
            updateTableStats(ui.statsTableWidget->item(1, 0), diff, charStatsPerPoint.mana); // current mana
            updateTableStats(ui.statsTableWidget->item(1, 1), diff, charStatsPerPoint.mana); // base mana
        }
    }
}

void MedianXLOfflineTools::respecStats()
{
    for (int i = Enums::CharacterStats::Strength; i <= Enums::CharacterStats::Vitality; ++i)
    {
        Enums::CharacterStats::StatisticEnum statCode = static_cast<Enums::CharacterStats::StatisticEnum>(i);
        int baseStat = _baseStatsMap[_editableCharInfo.basicInfo.classCode].statsAtStart.statFromCode(statCode);
        _spinBoxesStatsMap[statCode]->setValue(baseStat);
    }
    ui.statusBar->clearMessage();
}

void MedianXLOfflineTools::respecSkills(bool shouldRespec)
{
    int skills = _editableCharInfo.basicInfo.totalSkillPoints;
    ui.freeSkillPointsLineEdit->setText(QString::number(shouldRespec ? skills : _statsDynamicData.property("FreeSkillPoints").toUInt()));
    updateMaxCompoundStatusTip(ui.freeSkillPointsLineEdit, skills, shouldRespec ? 0 : skills - _statsDynamicData.property("FreeSkillPoints").toUInt());
}

void MedianXLOfflineTools::rename()
{
    QD2CharRenamer renameWidget(this, _editableCharInfo.basicInfo.originalName);
    if (renameWidget.exec())
    {
        _editableCharInfo.basicInfo.newName = renameWidget.name();
        QD2CharRenamer::updateNamePreview(ui.charNamePreview, _editableCharInfo.basicInfo.newName);
    }
}

void MedianXLOfflineTools::levelChanged(int newClvl)
{
    if (_isLoaded && qAbs(_oldClvl - newClvl) < _editableCharInfo.basicInfo.level)
    {
        int statsDiff = (_oldClvl - newClvl) * statPointsPerLevel;
        _oldClvl = newClvl;
        if (ui.freeStatPointsLineEdit->text().toInt() - statsDiff < 0)
            respecStats();
        ui.freeStatPointsLineEdit->setText(QString::number(ui.freeStatPointsLineEdit->text().toInt() - statsDiff));
        foreach (QSpinBox *spinBox, _spinBoxesStatsMap)
            spinBox->setMaximum(spinBox->maximum() - statsDiff);

        bool hasLevelChanged = newClvl != _editableCharInfo.basicInfo.level;
        if (_resurrectionPenalty != ResurrectPenaltyDialog::Skills)
        {
            ui.respecSkillsCheckBox->setChecked(hasLevelChanged);
            ui.respecSkillsCheckBox->setDisabled(hasLevelChanged);
        }

        int newSkillPoints = _editableCharInfo.basicInfo.totalSkillPoints, investedSkillPoints = 0;
        if (hasLevelChanged)
        {
            newSkillPoints -= (_editableCharInfo.basicInfo.level - newClvl) * skillPointsPerLevel;
            ui.freeSkillPointsLineEdit->setText(QString::number(newSkillPoints));
        }
        else if (_resurrectionPenalty == ResurrectPenaltyDialog::Skills)
            respecSkills(true);
        else
            investedSkillPoints = newSkillPoints - _statsDynamicData.property("FreeSkillPoints").toInt();

        int investedStats = investedStatPoints();
        updateStatusTips(ui.freeStatPointsLineEdit->text().toInt() + investedStats, investedStats, newSkillPoints, investedSkillPoints);
    }
}

void MedianXLOfflineTools::resurrect()
{
    ResurrectPenaltyDialog dlg(this);
    if (dlg.exec())
    {
        _editableCharInfo.basicInfo.hadDied = false;
        updateHardcoreUIElements();
        updateCharacterTitle(true);

        ui.levelSpinBox->setMaximum(_editableCharInfo.basicInfo.level);
        ui.levelSpinBox->setValue(_editableCharInfo.basicInfo.level);

        _resurrectionPenalty = dlg.resurrectionPenalty();
        switch (_resurrectionPenalty)
        {
        case ResurrectPenaltyDialog::Levels:
        {
            int newLevel = _editableCharInfo.basicInfo.level - ResurrectPenaltyDialog::levelPenalty;
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
            _editableCharInfo.basicInfo.totalSkillPoints = newSkillPoints;

            ui.freeSkillPointsLineEdit->setText(QString::number(newSkillPoints));
            updateMaxCompoundStatusTip(ui.freeSkillPointsLineEdit, newSkillPoints, 0);

            break;
        }
        case ResurrectPenaltyDialog::Stats:
        {
            respecStats();

            ushort newStatPoints = ui.freeStatPointsLineEdit->text().toUShort(), diff = newStatPoints * ResurrectPenaltyDialog::statPenalty;
            newStatPoints -= diff;
            _editableCharInfo.basicInfo.totalStatPoints = newStatPoints;

            ui.freeStatPointsLineEdit->setText(QString::number(newStatPoints));
            updateMaxCompoundStatusTip(ui.freeStatPointsLineEdit, newStatPoints, 0);

            foreach (QSpinBox *spinBox, _spinBoxesStatsMap)
                spinBox->setMaximum(spinBox->maximum() - diff);

            break;
        }
        default:
            break;
        }
    }
}

void MedianXLOfflineTools::convertToSoftcore(bool isSoftcore)
{
    updateCharacterTitle(!isSoftcore);
}

//void MedianXLOfflineTools::currentDifficultyChanged(int newDifficulty)
//{
//    quint8 progression = _editableCharInfo.basicInfo.titleCode, maxDifficulty = ui.currentDifficultyComboBox->count() - 1, maxAct;
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
    ItemDataBase::clvl = &_editableCharInfo.basicInfo.level;
    ItemDataBase::charClass = &_editableCharInfo.basicInfo.classCode;

    if (_itemsDialog)
    {
        if (activate)
            _itemsDialog->activateWindow();
    }
    else
    {
        _itemsDialog = new ItemsViewerDialog(this);
        _itemsDialog->show();
    }
}

void MedianXLOfflineTools::giveCube()
{
    QString cubePath = ResourcePathManager::dataPathForFileName("items/cube.d2i");
    ItemInfo *cube = ItemParser::loadItemFromFile(cubePath);
    if (!cube)
    {
        ERROR_BOX(tr("Error loading '%1'").arg(cubePath));
        return;
    }
    
    if (!ItemParser::storeItemIn(cube, Enums::ItemStorage::Inventory, 6, 10) && !ItemParser::storeItemIn(cube, Enums::ItemStorage::Stash, 10, 10))
    {
        ERROR_BOX(tr("You have no free space in inventory and stash to store the Cube"));
        delete cube;
        return;
    }

    // predefined position is (0,0) in inventory
    if (cube->column)
        ReverseBitWriter::replaceValueInBitString(cube->bitString, Enums::ItemOffsets::Columns, 4, cube->column);
    if (cube->row)
        ReverseBitWriter::replaceValueInBitString(cube->bitString, Enums::ItemOffsets::Rows, 3, cube->row);
    if (cube->storage != Enums::ItemStorage::Inventory)
    {
        ReverseBitWriter::replaceValueInBitString(cube->bitString, Enums::ItemOffsets::Storage, 3, cube->storage);

        ItemInfo *plugyCube = new ItemInfo(*cube);
        plugyCube->storage = Enums::ItemStorage::PersonalStash;
        plugyCube->plugyPage = 1;
        _editableCharInfo.items.character += plugyCube;
    }
    _editableCharInfo.items.character += cube;

    if (_itemsDialog)
        _itemsDialog->updateItems();

    ui.actionGiveCube->setDisabled(true);
    INFO_BOX(tr("Cube has been stored in %1 at (%2,%3)").arg(ItemsViewerDialog::tabNames.at(ItemsViewerDialog::indexFromItemStorage(cube->storage))).arg(cube->row + 1).arg(cube->column + 1));
}

void MedianXLOfflineTools::backupSettingTriggered(bool checked)
{
    if (!checked)
    {
        if (QMessageBox::question(this, qApp->applicationName(), tr("Are you sure you want to disable automatic backups? Then don't blame me if your character gets corrupted."),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
        {
            ui.actionBackup->setChecked(true);
        }
    }
}

void MedianXLOfflineTools::aboutApp()
{
    QMessageBox aboutBox(this);
    aboutBox.setWindowTitle(tr("About %1").arg(qApp->applicationName()));
    aboutBox.setIconPixmap(windowIcon().pixmap(64));
    aboutBox.setTextFormat(Qt::RichText);
    QString appFullName = qApp->applicationName() + " " + qApp->applicationVersion();
    aboutBox.setText(QString("<b>%1</b><br>").arg(appFullName) + tr("Released: %1").arg(releaseDate));
    QString email("decapitator@ukr.net");
    aboutBox.setInformativeText(tr("<i>Author:</i> Filipenkov Andrey (<b>kambala</b>)") +
                                QString("<br><i>ICQ:</i> 287764961<br><i>E-mail:</i> <a href=\"mailto:%1?subject=%2\">%1</a><br><br>")
                                .arg(email, appFullName) + tr("<a href=\"http://modsbylaz.14.forumer.com/viewtopic.php?t=23147\">Official Median XL Forum thread</a>"
                                                              "<br><a href=\"http://forum.worldofplayers.ru/showthread.php?t=34489\">Official Russian Median XL Forum thread</a>") + "<br><br>" +
                                tr("<b>Credits:</b><ul><li><a href=\"http://modsbylaz.hugelaser.com/\">BrotherLaz</a> for this awesome mod</li>"
                                   "<li><a href=\"http://modsbylaz.14.forumer.com/profile.php?mode=viewprofile&u=33805\">grig</a> for the Perl source of "
                                   "<a href=\"http://grig.vlexofree.com/\">Median XL Online Tools</a> and tips</li></ul>"));
    aboutBox.exec();
}


// overridden protected methods

void MedianXLOfflineTools::closeEvent(QCloseEvent *e)
{
    saveSettings();
    e->accept();
}


// private methods

void MedianXLOfflineTools::createLanguageMenu()
{
    QString appTranslationName = qApp->applicationName().remove(' ').toLower();
    QStringList fileNames = QDir(LanguageManager::instance().translationsPath, QString("%1_*.qm").arg(appTranslationName)).entryList(QDir::Files);
    if (fileNames.size())
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

void MedianXLOfflineTools::loadData()
{
    loadExpTable();
    loadMercNames();
}

void MedianXLOfflineTools::loadExpTable()
{
    QFile f(ResourcePathManager::dataPathForFileName("exptable.txt"));
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ERROR_BOX(tr("Experience table not loaded.\nReason: %1").arg(f.errorString()));
        return;
    }

    QList<QByteArray> expLines = f.readAll().split('\n');
    mercExperience.reserve(Enums::CharacterStats::MaxLevel);
    foreach (const QByteArray &numberString, expLines)
        if (!numberString.isEmpty())
            mercExperience.append(numberString.toUInt());
}

void MedianXLOfflineTools::loadMercNames()
{
    QFile f(ResourcePathManager::localizedPathForFileName("mercs"));
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ERROR_BOX(tr("Mercenary names not loaded.\nReason: %1").arg(f.errorString()));
        //close();
        return;
    }

    QList<QByteArray> mercLines = f.readAll().split('\n');
    QStringList actNames;
    mercNames.reserve(4);
    foreach (const QByteArray &mercName, mercLines)
    {
        if (mercName.isEmpty())
        {
            mercNames += actNames;
            actNames.clear();
        }
        else
            actNames += QString::fromUtf8(mercName);
    }
}

void MedianXLOfflineTools::createLayout()
{
    QList<QWidget *> widgetsToFixSize = QList<QWidget *>() << ui.charNamePreview << ui.classLineEdit << ui.titleLineEdit << ui.freeSkillPointsLineEdit
                                                           << ui.freeStatPointsLineEdit << ui.signetsOfLearningEatenLineEdit << ui.signetsOfSkillEatenLineEdit << ui.strengthSpinBox << ui.dexteritySpinBox
                                                           << ui.vitalitySpinBox << ui.energySpinBox << ui.inventoryGoldLineEdit << ui.stashGoldLineEdit << ui.mercLevelLineEdit;
    foreach (QWidget *w, widgetsToFixSize)
        w->setFixedSize(w->size());

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

    gridLayout->addWidget(new QLabel(tr("Free Skill Points")), 1, 0, Qt::AlignRight);
    gridLayout->addWidget(ui.freeSkillPointsLineEdit, 1, 1);
    gridLayout->addWidget(ui.respecSkillsCheckBox, 1, 2);
    gridLayout->addWidget(new QLabel(tr("Signets of Skill Eaten")), 1, 3, Qt::AlignRight);
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

    gridLayout->addWidget(new QLabel(tr("Free Stat Points")), 6, 0, Qt::AlignRight);
    gridLayout->addWidget(ui.freeStatPointsLineEdit, 6, 1);
    gridLayout->addWidget(ui.respecStatsButton, 6, 2);
    gridLayout->addWidget(new QLabel(tr("Signets of Learning Eaten")), 6, 3, Qt::AlignRight);
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
    ui.actionAskBeforeReload->setChecked(settings.value("askBeforeReload", true).toBool());
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
    settings.setValue("askBeforeReload", ui.actionAskBeforeReload->isChecked());
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

    _baseStatsMap[Enums::ClassName::Amazon] = BaseStats(BaseStats::StatsAtStart(25, 25, 20, 15, 84), BaseStats::StatsPerLevel(100, 40, 60), BaseStats::StatsPerPoint(8, 8, 18));
    _baseStatsMap[Enums::ClassName::Sorceress] = BaseStats(BaseStats::StatsAtStart(10, 25, 15, 35, 74), BaseStats::StatsPerLevel(100, 40, 60), BaseStats::StatsPerPoint(8, 8, 18));
    _baseStatsMap[Enums::ClassName::Necromancer] = BaseStats(BaseStats::StatsAtStart(15, 25, 20, 25, 79), BaseStats::StatsPerLevel(80, 20, 80), BaseStats::StatsPerPoint(4, 8, 24));
    _baseStatsMap[Enums::ClassName::Paladin] = BaseStats(BaseStats::StatsAtStart(25, 20, 25, 15, 89), BaseStats::StatsPerLevel(120, 60, 40), BaseStats::StatsPerPoint(12, 8, 12));
    _baseStatsMap[Enums::ClassName::Barbarian] = BaseStats(BaseStats::StatsAtStart(30, 20, 30, 5, 92), BaseStats::StatsPerLevel(120, 60, 40), BaseStats::StatsPerPoint(12, 8, 12));
    _baseStatsMap[Enums::ClassName::Druid] = BaseStats(BaseStats::StatsAtStart(25, 20, 15, 25, 84), BaseStats::StatsPerLevel(80, 20, 80), BaseStats::StatsPerPoint(4, 8, 24));
    _baseStatsMap[Enums::ClassName::Assassin] = BaseStats(BaseStats::StatsAtStart(20, 35, 15, 15, 95), BaseStats::StatsPerLevel(100, 40, 60), BaseStats::StatsPerPoint(8, 8, 18));
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
    // not the best solution to remove recent files
    foreach (QAction *action, ui.menuFile->actions())
        if (action->statusTip().endsWith(".d2s"))
            ui.menuFile->removeAction(action);

    for (int i = 0; i < _recentFilesList.length(); ++i)
    {
        QString filePath = _recentFilesList.at(i);
        if (QFile::exists(filePath))
            ui.menuFile->insertAction(_exitSeparator, createRecentFileAction(filePath, i + 1));
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

bool MedianXLOfflineTools::loadFile(const QString &charPath)
{
    if (charPath.isEmpty())
        return false;

    if (processSaveFile(charPath))
    {
        _charPath = charPath;
        updateUI();
        addToRecentFiles(charPath);

        ItemDataBase::currentCharacterItems = &_editableCharInfo.items.character;
        if (_itemsDialog)
            _itemsDialog->updateItems();
        if (_itemsDialog || ui.actionOpenItemsAutomatically->isChecked())
            showItems();

        QSettings settings;
        settings.beginGroup("recentItems");
        settings.setValue(lastSavePathKey, QFileInfo(charPath).canonicalPath());

        return true;
    }
    else
    {
        _saveFileContents.clear();
        _charPath.clear();

        clearUI();
        setWindowTitle(qApp->applicationName());

        return false;
    }
}

bool MedianXLOfflineTools::processSaveFile(const QString &charPath)
{
    QFile inputFile(charPath);
    if (!inputFile.open(QIODevice::ReadOnly))
    {
        ERROR_BOX_FILE(tr("Error opening file '%1'\nReason: %2"), inputFile);
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

    CharacterInfo editableCharInfo;
    editableCharInfo.basicInfo.originalName = _saveFileContents.constData() + Enums::Offsets::Name;
    editableCharInfo.basicInfo.originalName.replace(ansiColorHeader, unicodeColorHeader);

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
    editableCharInfo.basicInfo.isHardcore = status & Enums::StatusBits::IsHardcore;
    editableCharInfo.basicInfo.hadDied = status & Enums::StatusBits::HadDied;

    if (classCode > Enums::ClassName::Assassin)
    {
        ERROR_BOX(tr("Wrong class value: got %1").arg(classCode));
        return false;
    }
    editableCharInfo.basicInfo.classCode = static_cast<Enums::ClassName::ClassNameEnum>(classCode);

    if (progression >= Enums::Progression::Completed)
    {
        ERROR_BOX(tr("Wrong progression value: got %1").arg(progression));
        return false;
    }
    editableCharInfo.basicInfo.titleCode = progression;

    if (!clvl || clvl > Enums::CharacterStats::MaxLevel)
    {
        ERROR_BOX(tr("Wrong level: got %1").arg(clvl));
        return false;
    }
    editableCharInfo.basicInfo.level = clvl;

    //inputDataStream.device()->seek(Enums::Offsets::CurrentLocation);
    //for (int i = 0; i < 3; ++i)
    //{
    //    quint8 difficulty;
    //    inputDataStream >> difficulty;
    //    if (difficulty & Enums::DifficultyBits::IsActive)
    //    {
    //        editableCharInfo.basicInfo.currentDifficulty = i;
    //        editableCharInfo.basicInfo.currentAct = (difficulty & Enums::DifficultyBits::CurrentAct) + 1;
    //        break;
    //    }
    //}

    inputDataStream.device()->seek(Enums::Offsets::Mercenary);
    quint32 mercID;
    inputDataStream >> mercID;
    if (editableCharInfo.mercenary.exists = (mercID != 0))
    {
        quint16 mercName, mercValue;
        inputDataStream >> mercName >> mercValue;
        if (mercValue > Enums::Mercenary::MaxCode)
        {
            ERROR_BOX(tr("Wrong mercenary code: got %1").arg(mercValue));
            return false;
        }
        editableCharInfo.mercenary.code = Enums::Mercenary::mercCodeFromValue(mercValue);
        editableCharInfo.mercenary.nameIndex = mercName;

        quint32 mercExp;
        inputDataStream >> mercExp;
        editableCharInfo.mercenary.experience = mercExp;
        for (quint8 i = 1; i <= Enums::CharacterStats::MaxLevel; ++i)
            if (mercExp < i * i * (i + 1))
            {
                editableCharInfo.mercenary.level = i - 1;
                break;
            }
    }

    // Quests
    if (_saveFileContents.mid(Enums::Offsets::QuestsHeader, 4) != "Woo!")
    {
        ERROR_BOX(tr("Quests data not found!"));
        return false;
    }
    editableCharInfo.questsInfo.denOfEvilQuestsCompleted = editableCharInfo.questsInfo.radamentQuestsCompleted = 0;
    editableCharInfo.questsInfo.lamEsensTomeQuestsCompleted = editableCharInfo.questsInfo.izualQuestsCompleted = 0;
    for (int i = 0; i < difficultiesNumber; ++i)
    {
        int baseOffset = Enums::Offsets::QuestsData + i * Enums::Quests::Size;
        editableCharInfo.questsInfo.denOfEvilQuestsCompleted += _saveFileContents.at(baseOffset + Enums::Quests::DenOfEvil) & Enums::Quests::IsCompleted;
        editableCharInfo.questsInfo.radamentQuestsCompleted += _saveFileContents.at(baseOffset + Enums::Quests::Radament) & Enums::Quests::IsCompleted ||
                                                               _saveFileContents.at(baseOffset + Enums::Quests::Radament) & Enums::Quests::IsTaskDone;
        editableCharInfo.questsInfo.lamEsensTomeQuestsCompleted += _saveFileContents.at(baseOffset + Enums::Quests::LamEsensTome) & Enums::Quests::IsCompleted;
        editableCharInfo.questsInfo.izualQuestsCompleted += _saveFileContents.at(baseOffset + Enums::Quests::Izual) & Enums::Quests::IsCompleted;
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
    editableCharInfo.skillsOffset = skillsOffset;

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
        _statsDynamicData.setProperty(statisticMetaEnum.key(i), QVariant());

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
            int baseStat = _baseStatsMap[editableCharInfo.basicInfo.classCode].statsAtStart.statFromCode(static_cast<Enums::CharacterStats::StatisticEnum>(statCode));
            totalStats += statValue - baseStat;
        }
        else if (statCode == Enums::CharacterStats::FreeStatPoints)
            totalStats += statValue;

        _statsDynamicData.setProperty(Enums::CharacterStats::statisticNameFromValue(statCode), statValue);
    }
    if (count == 20)
    {
        ERROR_BOX(tr("Stats data is corrupted!"));
        return false;
    }

    int totalPossibleStats = totalPossibleStatPoints(clvl, editableCharInfo.questsInfo.lamEsensTomeQuestsCompleted, _statsDynamicData.property("SignetsOfLearningEaten").toInt());
    if (totalStats > totalPossibleStats) // check if stats are hacked
    {
        QMetaEnum statisticMetaEnum = Enums::CharacterStats::statisticMetaEnum();
        for (int i = Enums::CharacterStats::Strength; i <= Enums::CharacterStats::Vitality; ++i)
        {
            Enums::CharacterStats::StatisticEnum statCode = static_cast<Enums::CharacterStats::StatisticEnum>(i);
            int baseStat = _baseStatsMap[editableCharInfo.basicInfo.classCode].statsAtStart.statFromCode(statCode);
            _statsDynamicData.setProperty(statisticMetaEnum.key(i), baseStat);
        }
        _statsDynamicData.setProperty("FreeStatPoints", totalPossibleStats);
        if (!wasHackWarningShown)
        {
            WARNING_BOX(hackerDetected);
            wasHackWarningShown = true;
        }
    }

    // skills
    int skills = 0, maxPossibleSkills = totalPossibleSkillPoints(clvl, editableCharInfo.questsInfo.denOfEvilQuestsCompleted, editableCharInfo.questsInfo.radamentQuestsCompleted,
                                                                 editableCharInfo.questsInfo.izualQuestsCompleted, _statsDynamicData.property("SignetsOfSkillEaten").toInt());
    for (int i = 0; i < skillsNumber; ++i)
        skills += _saveFileContents.at(skillsOffset + 2 + i);
    skills += _statsDynamicData.property("FreeSkillPoints").toUInt();
    if (skills > maxPossibleSkills) // check if skills are hacked
    {
        skills = maxPossibleSkills;
        _statsDynamicData.setProperty("FreeSkillPoints", skills);
        _saveFileContents.replace(editableCharInfo.skillsOffset + 2, skillsNumber, QByteArray(skillsNumber, 0));
        if (!wasHackWarningShown)
        {
            WARNING_BOX(hackerDetected);
            wasHackWarningShown = true;
        }
    }
    editableCharInfo.basicInfo.totalSkillPoints = skills;

    // items
    inputDataStream.skipRawData(skillsNumber + 2);
    int charItemsOffset = inputDataStream.device()->pos();
    if (_saveFileContents.mid(charItemsOffset, 2) != ItemParser::itemHeader)
    {
        ERROR_BOX(tr("Items data not found!"));
        return false;
    }
    editableCharInfo.itemsOffset = charItemsOffset + 2;
    inputDataStream.skipRawData(2); // pointing to the beginning of item data

    quint16 charItemsTotal;
    inputDataStream >> charItemsTotal;

    const int itemListTerminatorSize = 4, maxCorpses = 15;
    char itemListTerminator[itemListTerminatorSize] = {'J', 'M', 0, 0};
    int itemListTerminatorOffset;
    editableCharInfo.items.corpses = 0;
    do
    {
        itemListTerminatorOffset = _saveFileContents.indexOf(QByteArray(itemListTerminator, itemListTerminatorSize), charItemsOffset);
        if (itemListTerminatorOffset != -1)
            break;
        itemListTerminator[2] = ++editableCharInfo.items.corpses;
    } while (editableCharInfo.items.corpses <= maxCorpses);

    if (itemListTerminatorOffset == -1)
    {
        ERROR_BOX(tr("Items list doesn't have a terminator!"));
        return false;
    }
    editableCharInfo.itemsEndOffset = itemListTerminatorOffset;
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

    bool isMercItemListBad = editableCharInfo.mercenary.exists ^ mercItemsOffset != eof;
    if (isMercItemListBad)
    {
        ERROR_BOX(tr("Mercenary items data is corrupted!"));
        return false;
    }

    for (int i = 0; i < charItemsTotal; ++i)
    {
        editableCharInfo.items.character += ItemParser::parseItem(inputDataStream, _saveFileContents);
    }

    if (editableCharInfo.items.corpses)
    {
        inputDataStream.device()->seek(itemListTerminatorOffset);
        // process corpse items
    }

    if (editableCharInfo.mercenary.exists)
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
    _plugyStashesHash[Enums::ItemStorage::PersonalStash].path = ui.actionAutoOpenPersonalStash->isChecked() ? QString("%1/%2.d2x").arg(charPathFileInfo.canonicalPath(), charPathFileInfo.baseName()) : QString();
    _plugyStashesHash[Enums::ItemStorage::SharedStash].path = ui.actionAutoOpenSharedStash->isChecked() ? charPathFileInfo.canonicalPath() + "/_LOD_SharedStashSave.sss" : QString();
    _plugyStashesHash[Enums::ItemStorage::HCStash].path = ui.actionAutoOpenHCShared->isChecked() ? charPathFileInfo.canonicalPath() + "/_LOD_HC_SharedStashSave.sss" : QString();

    bool sharedStashPathChanged = oldSharedStashPath != _plugyStashesHash[Enums::ItemStorage::SharedStash].path, hcStashPathChanged = oldHCStashPath != _plugyStashesHash[Enums::ItemStorage::HCStash].path;
    if (ui.actionReloadSharedStashes->isChecked())
        sharedStashPathChanged = hcStashPathChanged = true;
    _sharedGold = 0;
    for (QHash<Enums::ItemStorage::ItemStorageEnum, PlugyStashInfo>::iterator iter = _plugyStashesHash.begin(); iter != _plugyStashesHash.end(); ++iter)
    {
        switch (iter.key())
        {
        case Enums::ItemStorage::PersonalStash:
            if (!ui.actionAutoOpenPersonalStash->isChecked())
                continue;
            break;
        case Enums::ItemStorage::SharedStash:
            if (!sharedStashPathChanged || !ui.actionAutoOpenSharedStash->isChecked())
                continue;
            break;
        case Enums::ItemStorage::HCStash:
            if (!hcStashPathChanged || !ui.actionAutoOpenHCShared->isChecked())
                continue;
            break;
        default:
            break;
        }
        processPlugyStash(iter, &editableCharInfo.items.character);
    }

    // check for duped items
    // TODO: uncomment
    /*QSet<quint32> itemIDs;
    foreach (ItemInfo *item, editableCharInfo.items.character)
        if (item->isExtended)
        {
            if (itemIDs.contains(item->guid))
            {
                WARNING_BOX(tr("Like duping items, eh?"));
                break;
            }
            else
                itemIDs.insert(item->guid);
        }
        */
    clearItems(sharedStashPathChanged, hcStashPathChanged);
    ItemsList savedItems = _editableCharInfo.items.character;
    _editableCharInfo = editableCharInfo;
    _editableCharInfo.items.character += savedItems;

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

inline int MedianXLOfflineTools::totalPossibleStatPoints(int level, int lamEsen, int solsEaten)
{
    return (level - 1) * statPointsPerLevel + 5 * lamEsen + solsEaten;
}

inline int MedianXLOfflineTools::totalPossibleSkillPoints(int level, int doe, int radament, int izual, int sosEaten)
{
    return (level - 1) * skillPointsPerLevel + doe + radament + izual * 2 + sosEaten;
}

bool MedianXLOfflineTools::processPlugyStash(QHash<Enums::ItemStorage::ItemStorageEnum, PlugyStashInfo>::iterator &iter, ItemsList *items)
{
    PlugyStashInfo &info = iter.value();
    QFile inputFile(info.path);
    if (!(info.exists = inputFile.open(QIODevice::ReadOnly)))
        return true;

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
        return false;
    }
    info.header = header;

    QDataStream inputDataStream(bytes);
    inputDataStream.setByteOrder(QDataStream::LittleEndian);
    inputDataStream.skipRawData(headerSize);

    inputDataStream >> info.version;
    if (info.hasGold = (bytes.mid(headerSize + 1 + 4, 2) != ItemParser::plugyPageHeader))
        inputDataStream >> info.gold;
    if (iter.key() == Enums::ItemStorage::SharedStash)
        _sharedGold = info.gold;

    inputDataStream >> info.lastPage;

    for (quint32 page = 1; page <= info.lastPage; ++page)
    {
        if (bytes.mid(inputDataStream.device()->pos(), 2) != ItemParser::plugyPageHeader)
        {
            ERROR_BOX(tr("Page %1 of '%2' has wrong PlugY header").arg(page).arg(QFileInfo(info.path).fileName()));
            return false;
        }
        inputDataStream.skipRawData(3);
        if (bytes.mid(inputDataStream.device()->pos(), 2) != ItemParser::itemHeader)
        {
            ERROR_BOX(tr("Page %1 of '%2' has wrong item header").arg(page).arg(QFileInfo(info.path).fileName()));
            return false;
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
        }
    }

    return true;
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

    QList<QAction *> actions = QList<QAction *>() << ui.actionRespecStats << ui.actionRespecSkills << ui.actionActivateWaypoints << ui.actionSaveCharacter << ui.actionRename << ui.actionReloadCharacter
        << ui.actionFind << ui.actionFindNext << ui.actionFindPrevious << ui.actionShowItems;
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

    QList<QAction *> actions = QList<QAction *>() << ui.actionRespecStats << ui.actionRespecSkills << ui.actionActivateWaypoints
                                                  << ui.actionSaveCharacter << ui.actionRename << ui.actionReloadCharacter;
    foreach (QAction *action, actions)
        action->setEnabled(true);
    ui.respecSkillsCheckBox->setEnabled(true);

    QList<QGroupBox *> groupBoxes = QList<QGroupBox *>() << ui.characterGroupBox << ui.statsGroupBox;
    foreach (QGroupBox *groupBox, groupBoxes)
        groupBox->setEnabled(true);

    QD2CharRenamer::updateNamePreview(ui.charNamePreview, _editableCharInfo.basicInfo.originalName);

    ui.hardcoreGroupBox->setEnabled(_editableCharInfo.basicInfo.isHardcore);
    if (_editableCharInfo.basicInfo.isHardcore)
        updateHardcoreUIElements();

    ui.classLineEdit->setText(Enums::ClassName::classes().at(_editableCharInfo.basicInfo.classCode));
    updateCharacterTitle(_editableCharInfo.basicInfo.isHardcore);

    //ui.currentDifficultyComboBox->setEnabled(true);
    //ui.currentDifficultyComboBox->addItems(difficulties.mid(0, titleAndMaxDifficulty.second + 1));
    //ui.currentDifficultyComboBox->setCurrentIndex(_editableCharInfo.basicInfo.currentDifficulty);
    //ui.currentActSpinBox->setValue(_editableCharInfo.basicInfo.currentAct);

    _oldClvl = _editableCharInfo.basicInfo.level;
    ui.levelSpinBox->setMaximum(_oldClvl);
    ui.levelSpinBox->setValue(_oldClvl);

    setStats();
    recalculateStatPoints();

    int stats = _editableCharInfo.basicInfo.totalStatPoints, skills = _editableCharInfo.basicInfo.totalSkillPoints;
    updateStatusTips(stats, stats - _statsDynamicData.property("FreeStatPoints").toInt(), skills, skills - _statsDynamicData.property("FreeSkillPoints").toInt());
    ui.signetsOfLearningEatenLineEdit->setStatusTip(maxValueFormat.arg(Enums::CharacterStats::SignetsOfLearningMax));
    ui.signetsOfSkillEatenLineEdit->setStatusTip(maxValueFormat.arg(Enums::CharacterStats::SignetsOfSkillMax));
    ui.stashGoldLineEdit->setStatusTip(maxValueFormat.arg(QLocale().toString(Enums::CharacterStats::StashGoldMax)));
    if (_sharedGold)
        ui.stashGoldLineEdit->setStatusTip(ui.stashGoldLineEdit->statusTip() + ", " + tr("Shared: %1").arg(QLocale().toString(_sharedGold)));

    if (_editableCharInfo.mercenary.exists)
    {
        int mercTypeIndex;
        QPair<int, int> mercArraySlice = Enums::Mercenary::allowedTypesForMercCode(_editableCharInfo.mercenary.code, &mercTypeIndex);
        ui.mercTypeComboBox->addItems(Enums::Mercenary::types().mid(mercArraySlice.first, mercArraySlice.second));
        ui.mercTypeComboBox->setCurrentIndex(mercTypeIndex);

        ui.mercNameComboBox->addItems(mercNames.at(Enums::Mercenary::mercNamesIndexFromCode(_editableCharInfo.mercenary.code)));
        ui.mercNameComboBox->setCurrentIndex(_editableCharInfo.mercenary.nameIndex);

        ui.mercLevelLineEdit->setText(QString::number(_editableCharInfo.mercenary.level));

        ui.mercGroupBox->setEnabled(true);
    }

    ui.actionGiveCube->setDisabled(std::find_if(_editableCharInfo.items.character.constBegin(), _editableCharInfo.items.character.constEnd(), isCube) != _editableCharInfo.items.character.constEnd());

    bool hasItems = _editableCharInfo.items.character.size() > 0;
    ui.actionShowItems->setEnabled(hasItems);
    ui.actionFind->setEnabled(hasItems);
    updateWindowTitle();

    _isLoaded = true;
}

void MedianXLOfflineTools::updateHardcoreUIElements()
{
    ui.convertToSoftcoreCheckBox->setDisabled(_editableCharInfo.basicInfo.hadDied);
    ui.actionConvertToSoftcore->setDisabled(_editableCharInfo.basicInfo.hadDied);
    ui.resurrectButton->setEnabled(_editableCharInfo.basicInfo.hadDied);
    ui.actionResurrect->setEnabled(_editableCharInfo.basicInfo.hadDied);
}

void MedianXLOfflineTools::updateCharacterTitle(bool isHardcore)
{
    bool isMale = _editableCharInfo.basicInfo.classCode >= Enums::ClassName::Necromancer && _editableCharInfo.basicInfo.classCode <= Enums::ClassName::Druid;
    quint8 progression = _editableCharInfo.basicInfo.titleCode;
    QPair<QString, quint8> titleAndMaxDifficulty = Enums::Progression::titleNameAndMaxDifficultyFromValue(progression, isMale, isHardcore);
    QString newTitle = titleAndMaxDifficulty.first;
    if (_editableCharInfo.basicInfo.isHardcore && _editableCharInfo.basicInfo.hadDied)
        newTitle += QString(" (%1)").arg(tr("DEAD", "HC character is dead"));
    ui.titleLineEdit->setText(newTitle);
    ui.titleLineEdit->setStyleSheet(QString("color: %1").arg(isHardcore ? "red" : "black"));
}

void MedianXLOfflineTools::setStats()
{
    QMetaEnum statisticMetaEnum = Enums::CharacterStats::statisticMetaEnum();
    for (int i = 0; i < statisticMetaEnum.keyCount(); ++i)
    {
        const char *name = statisticMetaEnum.key(i);
        qulonglong value = _statsDynamicData.property(name).toULongLong();
        Enums::CharacterStats::StatisticEnum statCode = static_cast<Enums::CharacterStats::StatisticEnum>(statisticMetaEnum.value(i));

        if (statCode >= Enums::CharacterStats::Strength && statCode <= Enums::CharacterStats::Vitality)
        {
            _oldStatValues[statCode] = value;

            quint8 baseValue = _baseStatsMap[_editableCharInfo.basicInfo.classCode].statsAtStart.statFromCode(statCode);
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

int MedianXLOfflineTools::investedStatPoints()
{
    quint16 sum = 0;
    for (quint8 i = Enums::CharacterStats::Strength; i <= Enums::CharacterStats::Vitality; ++i)
    {
        Enums::CharacterStats::StatisticEnum ienum = static_cast<Enums::CharacterStats::StatisticEnum>(i);
        sum += _spinBoxesStatsMap[ienum]->value() - _baseStatsMap[_editableCharInfo.basicInfo.classCode].statsAtStart.statFromCode(ienum);
    }
    return sum;
}

void MedianXLOfflineTools::recalculateStatPoints()
{
    _editableCharInfo.basicInfo.totalStatPoints = investedStatPoints() + ui.freeStatPointsLineEdit->text().toUInt();
}

void MedianXLOfflineTools::updateStatusTips(int newStatPoints, int investedStatPoints, int newSkillPoints, int investedSkillPoints)
{
    updateMaxCompoundStatusTip(ui.freeStatPointsLineEdit, newStatPoints, investedStatPoints);
    updateMaxCompoundStatusTip(ui.freeSkillPointsLineEdit, newSkillPoints, investedSkillPoints);
    ui.inventoryGoldLineEdit->setStatusTip(maxValueFormat.arg(QLocale().toString(_oldClvl * Enums::CharacterStats::InventoryGoldFactor)));
}

void MedianXLOfflineTools::updateWindowTitle()
{
    setWindowTitle(QString("%1 - %2").arg(QDir::toNativeSeparators(_charPath), qApp->applicationName()));
}

void MedianXLOfflineTools::updateTableStats(QTableWidgetItem *item, int diff, int statPerPoint)
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
            if (statCode == Enums::CharacterStats::SignetsOfLearningEaten && value == Enums::CharacterStats::SignetsOfLearningMax)
                value++; // SignetsOfLearningEaten should be set to 501
            else if (statCode == Enums::CharacterStats::FreeStatPoints)
            {
                int totalPossibleFreeStats = totalPossibleStatPoints(ui.levelSpinBox->value(), _editableCharInfo.questsInfo.lamEsensTomeQuestsCompleted,
                                                                     _lineEditsStatsMap[Enums::CharacterStats::SignetsOfLearningEaten]->text().toInt()) - investedStatPoints();
                if (value > totalPossibleFreeStats) // prevent hacks
                {
                    value = totalPossibleFreeStats;
                    WARNING_BOX(hackerDetected);
                }
            }
        }
        else if (statCode != Enums::CharacterStats::End && isExpAndLevelNotSet) // level or exp
        {
            quint8 clvl = _editableCharInfo.basicInfo.level, newClvl = ui.levelSpinBox->value();
            if (clvl != newClvl) // set new level and experience explicitly
            {
                addStatisticBits(result, Enums::CharacterStats::Level, Enums::CharacterStats::StatCodeLength);
                addStatisticBits(result, newClvl, Enums::CharacterStats::statLengthFromValue(Enums::CharacterStats::Level));
                _statsDynamicData.setProperty("Level", newClvl);

                quint32 newExp = mercExperience.at(newClvl - 1);
                if (newExp) // must not be present for level 1 character
                {
                    addStatisticBits(result, Enums::CharacterStats::Experience, Enums::CharacterStats::StatCodeLength);
                    addStatisticBits(result, newExp, Enums::CharacterStats::statLengthFromValue(Enums::CharacterStats::Experience));
                }
                _statsDynamicData.setProperty("Experience", newExp);

                isExpAndLevelNotSet = false;
                continue;
            }
            else
                value = _statsDynamicData.property(enumKey).toULongLong();
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

        _statsDynamicData.setProperty(enumKey, value);
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
    QMutableListIterator<ItemInfo *> itemIterator(_editableCharInfo.items.character);
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
        QFile backupFile(file.fileName() + ".bak");
        if (backupFile.exists() && !backupFile.remove())
            ERROR_BOX_FILE(tr("Error removing old backup '%1'\nReason: %2"), backupFile);
        else
        {
            if (!file.copy(backupFile.fileName()))
                ERROR_BOX_FILE(tr("Error creating backup for file '%1'\nReason: %2"), file);
        }
    }
}
