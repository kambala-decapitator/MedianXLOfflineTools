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

namespace Ui { class MedianXLOfflineToolsClass; }

class QSpinBox;
class QLineEdit;
class QLabel;
class QCheckBox;
class QGroupBox;
class QActionGroup;
class QTableWidgetItem;

class QFile;

class QNetworkAccessManager;
class QNetworkReply;

class MedianXLOfflineTools : public QMainWindow
{
    Q_OBJECT

public:
    static const QString kCompoundFormat, kCharacterExtension, kCharacterExtensionWithDot;
    static const quint32 kFileSignature;
    static const int kSkillsNumber, kDifficultiesNumber, kMaxRecentFiles;
    static const int kStatPointsPerLevel, kSkillPointsPerLevel;

    MedianXLOfflineTools(const QString &cmdPath = QString(), QWidget *parent = 0, Qt::WFlags flags = 0);
    virtual ~MedianXLOfflineTools() { delete ui; }

public slots:
    bool loadFile(const QString &charPath);

protected:
    virtual void closeEvent(QCloseEvent *e);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);

private slots:
    void switchLanguage(QAction *languageAction);
    void setModified(bool modified);
    void modify() { setModified(true); }
    void eatSignetsOfLearning(int signetsEaten);
    void networkReplyFinished(QNetworkReply *reply);

#ifdef Q_WS_MACX
    void moveUpdateActionToAppleMenu();
#endif

    // file
    void loadCharacter();
    void openRecentFile();
    void reloadCharacter(bool notify = true);
    void saveCharacter();

    // edit
    void statChanged(int newValue);
    void respecStats();
    void respecSkills(bool shouldRespec);
    void rename();
    void levelChanged(int newClvl);
    void resurrect();
    void convertToSoftcore(bool isSoftcore);
    //void currentDifficultyChanged(int newDifficulty);
    void findItem();
    void showFoundItem(ItemInfo *item);

    // items
    void showItems(bool activate = true);
    void itemStorageTabChanged(int tabIndex);
    void giveCube();

    // export
    void showSkillPlan();

    // options
    void backupSettingTriggered(bool checked);
    void associateFiles();

    // help
    void checkForUpdate();
    void aboutApp();

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
    QGroupBox *_questsGroupBox;
    QHash<int, QList<QCheckBox *> > _checkboxesQuestsHash;
    QActionGroup *_backupLimitsGroup, *_backupFormatsGroup;

    // data
    QString _charPath;
    QByteArray _saveFileContents;
    int _oldStatValues[4];
    QMap <Enums::ClassName::ClassNameEnum, BaseStats> _baseStatsMap;
    QHash<Enums::ClassName::ClassNameEnum, QPair<QList<int>, QList<int> > > _characterSkillsIndexes; // first is save file order, second - visual and planner order
    int _oldClvl;
    quint32 _sharedGold;
    QHash<Enums::ItemStorage::ItemStorageEnum, PlugyStashInfo> _plugyStashesHash;

    // consts
    QList<quint32> experienceTable;
    QVector<QStringList> mercNames;
    //const QStringList difficulties;
    const QString hackerDetected, maxValueFormat, minValueFormat, investedValueFormat, kForumThreadHtmlLinks;

    bool _isLoaded;
    ResurrectPenaltyDialog::ResurrectionPenalty _resurrectionPenalty;
    QNetworkAccessManager *_qnam;
    bool _isManuallyCheckingForUpdate;

    // the following group of methods is Windows 7 specific
#ifdef Q_WS_WIN32
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
    void createMercGroupBoxLayout();
    void createStatsGroupBoxLayout();
    void createQuestsGroupBoxLayout();

    void loadSettings();
    void saveSettings() const;

    void fillMaps();
    void connectSignals();

    void updateRecentFilesActions();
    void addToRecentFiles();
    QAction *createRecentFileAction(const QString &fileName, int index);

    bool processSaveFile();
    quint32 checksum(const QByteArray &charByteArray) const;

    inline int totalPossibleStatPoints(int level) const;
    inline int totalPossibleSkillPoints() const;
    int investedStatPoints();
    inline void recalculateStatPoints();

    void clearUI();
    void updateUI();
    inline void updateHardcoreUIElements();
    void updateCharacterTitle(bool isHardcore);
    void setStats();
    quint32 mercExperienceForLevel(quint8 level) { return static_cast<quint32>(level * level * (level + 1)); }

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
    void clearItems(bool sharedStashPathChanged = true, bool hcStashPathChanged = true);

    void backupFile(QFile &file);
    void showErrorMessageBoxForFile(const QString &message, const QFile &file);
    bool maybeSave();

    void checkForUpdateFromUrl(const QUrl &url);
};

#endif // MEDIANXLOFFLINETOOLS_H
