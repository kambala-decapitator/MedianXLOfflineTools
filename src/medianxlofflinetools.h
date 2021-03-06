#ifndef MEDIANXLOFFLINETOOLS_H
#define MEDIANXLOFFLINETOOLS_H

#include "enums.h"
#include "structs.h"
#include "resurrectpenaltydialog.h"

#include <QMainWindow>

#include <QMultiHash>
#include <QPointer>


class ItemsViewerDialog;
class FindItemsDialog;
class ExperienceIndicatorGroupBox;
class DupeScanDialog;

namespace Ui { class MedianXLOfflineToolsClass; }
class QSpinBox;
class QLineEdit;
class QLabel;
class QCheckBox;
class QGroupBox;
class QActionGroup;
class QTableWidgetItem;

class QFile;
class QFileSystemWatcher;
class QTimer;

class QNetworkAccessManager;
class QNetworkReply;

enum LaunchMode
{
    LaunchModeNormal,
    LaunchModeDupeScan,
    LaunchModeDumpItems
};

class MedianXLOfflineTools : public QMainWindow
{
    Q_OBJECT

public:
    static const QString kCompoundFormat, kCharacterExtension, kCharacterExtensionWithDot;
    static const quint32 kFileSignature;
    static const int kSkillsNumber, kDifficultiesNumber, kMaxRecentFiles;
    static const int kStatPointsPerLevel, kSkillPointsPerLevel, kStatPointsPerLamEsensTome;

    MedianXLOfflineTools(const QString &cmdPath, LaunchMode launchMode, QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~MedianXLOfflineTools();

#ifdef DUPE_CHECK
    bool shouldShowWindow;
#endif

public slots:
    bool loadFile(const QString &charPath, bool shouldCheckExtension = true, bool shouldOpenItemsWindow = true);
    void loadFileSkipExtensionCheck(const QString &charPath);

protected:
    virtual void closeEvent(QCloseEvent *e);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);

private slots:
    void switchLanguage(QAction *languageAction);
    void setModified(bool modified);
    void modify() { setModified(true); }

    void eatSignetsOfLearning(int signetsEaten);
    void updateFindResults();
    void dupeScanFinished();

    void networkReplyCheckForUpdateFinished(QNetworkReply *reply);

    void fileContentsChanged();
    void fileChangeTimerFired();

#ifdef Q_OS_MAC
    void moveUpdateActionToAppleMenu();
#endif

    // file
    void loadCharacter();
    void openRecentFile();
    void reloadCharacter(bool shouldNotify = true);
    void saveCharacter();
#ifdef DUPE_CHECK
    void showDupeCheck();
#endif

    // edit
    void statChanged(int newValue);
    void respecStats();
    void respecSkills(bool shouldRespec);
    void rename();
    void levelChanged(int newClvl);
    void resurrect();
    void convertToSoftcore(bool isSoftcore);
    void findItem();
    void showFoundItem(ItemInfo *item);

    // items
    void showItems(bool activate = true);
    void itemStorageTabChanged(int tabIndex);
    void giveCube();

    // export
    void showAllStats();

    // options
    void backupSettingTriggered(bool checked);
    void associateFiles();
#ifdef Q_OS_WIN32
    void showFileAssocaitionUI();
#endif

    // help
    void checkForUpdate();
    void aboutApp();

    // toolbar actions
    void showSkillTree();

private:
    // UI
    Ui::MedianXLOfflineToolsClass *ui;
    QMap<Enums::CharacterStats::StatisticEnum, QSpinBox  *> _spinBoxesStatsMap;
    QMap<Enums::CharacterStats::StatisticEnum, QLineEdit *> _lineEditsStatsMap;
    QLabel *_charPathLabel;
    QStringList _recentFilesList;
    QPointer<ItemsViewerDialog> _itemsDialog;
    FindItemsDialog *_findItemsDialog;
    ExperienceIndicatorGroupBox *_mercExpGroupBox, *_expGroupBox;
#if !defined(QT_NO_DEBUG_OUTPUT) && !defined(DUPE_CHECK)
    QCheckBox *_makeNonLadderCheckbox;
#endif
    QGroupBox *_questsGroupBox;
    QHash<int, QList<QCheckBox *> > _checkboxesQuestsHash;
    QActionGroup *_backupLimitsGroup, *_showDisenchantPreviewGroup;
    QPointer<DupeScanDialog> _dupeScanDialog;

    // data
    QString _charPath;
    QByteArray _saveFileContents;
    int _oldStatValues[4];
    QMap<Enums::ClassName::ClassNameEnum, BaseStats> _baseStatsMap;
    int _oldClvl;
    quint32 _sharedGold;
    QHash<Enums::ItemStorage::ItemStorageEnum, PlugyStashInfo> _plugyStashesHash;
    ResurrectPenaltyDialog::ResurrectionPenalty _resurrectionPenalty;
    bool _isLoaded;

    // consts
    QList<quint32> experienceTable;
    QVector<QStringList> mercNames;
    const QString kHackerDetected, maxValueFormat, minValueFormat, investedValueFormat, kForumThreadHtmlLinks;

    QNetworkAccessManager *_qnamCheckForUpdate;
    bool _isManuallyCheckingForUpdate;
    QByteArray _modDataVersion;

    QFileSystemWatcher *_fsWatcher;
    QTimer *_fileChangeTimer;
    bool _isFileChangedMessageBoxRunning;

    // the following group of methods is Windows 7 specific
#ifdef Q_OS_WIN32
    PCWSTR  appUserModelID();
    void setAppUserModelID();

    void syncWindowsTaskbarRecentFiles();
    void removeFromWindowsRecentFiles(const QString &filePath);
    void addToWindowsRecentFiles(const QString &filePath); // also used by earlier Windows versions
#endif

    void loadData();
    void loadExpTable();
    void loadMercNames();
    void loadBaseStats();

    void createLanguageMenu();
    void createLayout();
    void createCharacterGroupBoxLayout();
    void createWaypointsGroupBoxLayout();
    void createMercGroupBoxLayout();
    void createStatsGroupBoxLayout();
    void createQuestsGroupBoxLayout();
    void configureQuestLabel(QLabel *l, const QString &reward, const QString &questInfo);

    void loadSettings();
    void saveSettings() const;

    void fillMaps();
    void connectSignals();

    void updateRecentFilesActions();
    void addToRecentFiles();
    QAction *createRecentFileAction(const QString &filePath, int index);

    void loadSaveFile(const QString &filePath, bool shouldNotify, const QString &statusBarMessage);
    void loadSaveFile(const QString &filePath) { loadSaveFile(filePath, true, tr("Character loaded")); }
    bool processSaveFile();
    void showLoadingError(const QString &error, bool warn = false);
    quint32 checksum(const QByteArray &charByteArray) const;

    inline int totalPossibleStatPoints(int level) const;
    int totalPossibleStatPoints(int level, quint8 let) const;
    inline int totalPossibleSkillPoints() const;
    int totalPossibleSkillPoints(int level, quint8 doe, quint8 rad, quint8 iz) const;
    int investedStatPoints();
    inline void recalculateStatPoints();

    quint32 mercExperienceForLevel(quint8 level) const { return static_cast<quint32>(level * level * (level + 1)); }

    void clearUI();
    void updateUI();
    inline void updateHardcoreUIElements();
    void updateCharacterTitle(bool isHardcore);
    void setStats();

    inline void updateWindowTitle();
    void updateTableStats(const BaseStats::StatsStep &statsPerStep, int diff, QSpinBox *senderSpinBox = 0);
    void updateTableItemStat(QTableWidgetItem *item, int diff, int statPerPoint);
    void updateStatusTips(int newStatPoints, int investedStatPoints, int newSkillPoints, int investedSkillPoints);
    inline void updateCompoundStatusTip(QWidget *widget, const QString &firstString, const QString &secondString);
    inline void updateMinCompoundStatusTip(QWidget *widget, int minValue, int investedValue);
    inline void updateMaxCompoundStatusTip(QWidget *widget, int maxValue, int investedValue);
    void updateAssociateAction(bool disable);
    void updateCharacterExperienceProgressbar(quint32 newExperience);

    QByteArray statisticBytes();
    inline void addStatisticBits(QString &bitsString, quint64 number, int fieldWidth);

    void processPlugyStash(QHash<Enums::ItemStorage::ItemStorageEnum, PlugyStashInfo>::iterator &iter, ItemsList *items);
    QHash<int, bool> getPlugyStashesExistenceHash() const;
    void clearItems(bool sharedStashPathChanged1 = true, bool hcStashPathChanged1 = true, bool sharedStashPathChanged2 = true, bool hcStashPathChanged2 = true);

    QString backupFile(QFile &file);
    void showErrorMessageBoxForFile(const QString &message, const QFile &file);
    bool maybeSave();

    void displayInfoAboutServerVersion(const QByteArray &version);
};

#endif // MEDIANXLOFFLINETOOLS_H
