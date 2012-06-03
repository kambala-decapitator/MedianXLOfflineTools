#ifndef MEDIANXLOFFLINETOOLS_H
#define MEDIANXLOFFLINETOOLS_H

#include "ui_medianxlofflinetools.h"

#include "enums.h"
#include "resurrectpenaltydialog.h"
#include "structs.h"

#include <QMainWindow>

#include <QMultiHash>
#include <QPointer>


class QCloseEvent;
class QDragEnterEvent;
class QDropEvent;
class QFile;

class ItemsViewerDialog;
class FindItemsDialog;

class MedianXLOfflineTools : public QMainWindow
{
    Q_OBJECT

public:
    static const QString kCompoundFormat, kCharacterExtension, kCharacterExtensionWithDot;
    static const quint32 kFileSignature;
    static const int kSkillsNumber, kDifficultiesNumber, kMaxRecentFiles;
    static const int kStatPointsPerLevel, kSkillPointsPerLevel;

    MedianXLOfflineTools(const QString &cmdPath = QString(), QWidget *parent = 0, Qt::WFlags flags = 0);
    virtual ~MedianXLOfflineTools() { clearItems(); }

public slots:
    bool loadFile(const QString &charPath);

protected:
    void closeEvent(QCloseEvent *e);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);

private slots:
    void switchLanguage(QAction *languageAction);
    void setModified(bool modified);
    void modify() { setModified(true); }

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
    void getSkillPlan();

    // options
    void backupSettingTriggered(bool checked);
    void associateFiles();

    // about
    void aboutApp();

private:
    // UI
    Ui::MedianXLOfflineToolsClass ui;
    QMap<Enums::CharacterStats::StatisticEnum, QSpinBox  *> _spinBoxesStatsMap;
    QMap<Enums::CharacterStats::StatisticEnum, QLineEdit *> _lineEditsStatsMap;
    QLabel *_charPathLabel;
    QStringList _recentFilesList;
    QPointer<ItemsViewerDialog> _itemsDialog;
    FindItemsDialog *_findItemsDialog;
    QGroupBox *_questsGroupBox;
    QHash<int, QList<QCheckBox *> > _checkboxesQuestsHash;

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
    const QString hackerDetected, maxValueFormat, minValueFormat, investedValueFormat;

    bool _isLoaded;
    ResurrectPenaltyDialog::ResurrectionPenalty _resurrectionPenalty;

    // the following group of methods is Windows 7 specific
#ifdef Q_WS_WIN32
    static LPCWSTR appUserModelID();
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

    inline int totalPossibleStatPoints(int level);
    inline int totalPossibleSkillPoints();
    int investedStatPoints();
    inline void recalculateStatPoints();

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

    QByteArray statisticBytes();
    inline void addStatisticBits(QString &bitsString, quint64 number, int fieldWidth);

    void processPlugyStash(QHash<Enums::ItemStorage::ItemStorageEnum, PlugyStashInfo>::iterator &iter, ItemsList *items);
    QHash<int, bool> getPlugyStashesExistenceHash() const;
    void clearItems(bool sharedStashPathChanged = true, bool hcStashPathChanged = true);

    void backupFile(QFile &file);
    void showErrorMessageBoxForFile(const QString &message, const QFile &file);
    QString itemStorageAndCoordinatesString(const QString &text, ItemInfo *item);
    bool maybeSave();
};

#endif // MEDIANXLOFFLINETOOLS_H
