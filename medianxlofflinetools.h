#ifndef MEDIANXLOFFLINETOOLS_H
#define MEDIANXLOFFLINETOOLS_H

#include "ui_medianxlofflinetools.h"

#include "enums.h"
#include "resurrectpenaltydialog.h"
#include "reversebitreader.h"
#include "structs.h"

#include <QMainWindow>

#include <QMultiHash>
#include <QPointer>


class QCloseEvent;
class ItemsViewerDialog;
class FindItemsDialog;

class MedianXLOfflineTools : public QMainWindow
{
	Q_OBJECT

public:
	//static QString translationsPath, currentLocale;

	static const QString compoundFormat/*, languageKey, defaultLocale*/;
	static const quint32 fileSignature;
	static const int skillsNumber, difficultiesNumber, maxRecentFiles;
    static const int statPointsPerLevel, skillPointsPerLevel;

	MedianXLOfflineTools(QWidget *parent = 0, Qt::WFlags flags = 0);
    virtual ~MedianXLOfflineTools() { clearItems(); }

protected:
    void closeEvent(QCloseEvent *e);

private slots:
	void switchLanguage(QAction *languageAction);

	void loadCharacter();
	void openRecentFile();
	void reloadCharacter();
	void saveCharacter();

	void statChanged(int newValue);
	void respecStats();
	void respecSkills(bool shouldRespec);
	void rename();
	void levelChanged(int newClvl);
	void resurrect();
	void convertToSoftcore(bool isSoftcore);
	//void currentDifficultyChanged(int newDifficulty);
	void findItem();
	//void findNext();
	//void findPrevious();
	void showFoundItem(ItemInfo *item);

    void showItems(bool activate = true);
	void giveCube();

    void aboutApp();

private:
	// UI
	Ui::MedianXLOfflineToolsClass ui;
	//QLabel *_pathLabel;
    QMap<Enums::CharacterStats::StatisticEnum, QSpinBox *> _spinBoxesStatsMap;
    QMap<Enums::CharacterStats::StatisticEnum, QLineEdit *> _lineEditsStatsMap;
    QStringList _recentFilesList;
	QAction *_exitSeparator;
	QPointer<ItemsViewerDialog> _itemsDialog;
	FindItemsDialog *_findItemsDialog;

	// data
	QString _charPath;
	QByteArray _saveFileContents;
	CharacterInfo _editableCharInfo;
	QObject _statsDynamicData;
	int _oldStatValues[4];
	QMap<Enums::ClassName::ClassNameEnum, BaseStats> _baseStats;
	int _oldClvl;
    quint32 _sharedGold;
    QHash<Enums::ItemStorage::ItemStorageEnum, PlugyStashInfo> _plugyStashesHash;

	// consts
	QList<quint32> mercExperience;
    QVector<QStringList> mercNames;
	//const QStringList difficulties;
	const QString hackerDetected, maxValueFormat, minValueFormat, investedValueFormat;

	bool _isLoaded;
	ResurrectPenaltyDialog::ResurrectionPenalty _resurrectionPenalty;

	void createLanguageMenu();

	void loadData();
    void loadExpTable();
	void loadMercNames();

	void createLayout();
	void createCharacterGroupBoxLayout();
	void createMercGroupBoxLayout();
	void createStatsGroupBoxLayout();

	void loadSettings();
	void saveSettings() const;

	void fillMaps();
	void connectSignals();

	void updateRecentFilesActions();
	void addToRecentFiles(const QString &fileName);
	QAction *createRecentFileAction(const QString &fileName, int index);

    bool loadFile(const QString &charPath);
	bool processSaveFile(const QString &charPath);
    quint32 checksum(const QByteArray &charByteArray) const;
	inline int totalPossibleStatPoints(int level, int lamEsen, int solsEaten);
	inline int totalPossibleSkillPoints(int level, int doe, int radament, int izual, int sosEaten);

	void clearUI();
	void updateUI();
	inline void updateHardcoreUIElements();
	void updateCharacterTitle(bool isHardcore);
	void setStats();
	int investedStatPoints();
	inline void recalculateStatPoints();
	void updateStatusTips(int newStatPoints, int investedStatPoints, int newSkillPoints, int investedSkillPoints);
	inline void updateWindowTitle();
	void updateTableStats(QTableWidgetItem *item, int diff, int statPerPoint);
	inline void updateCompoundStatusTip(QWidget *widget, const QString &firstString, const QString &secondString);
	inline void updateMinCompoundStatusTip(QWidget *widget, int minValue, int investedValue);
	inline void updateMaxCompoundStatusTip(QWidget *widget, int maxValue, int investedValue);

	QByteArray statisticBytes();
	inline void addStatisticBits(QString &bitsString, quint64 number, int fieldWidth);

    bool processPlugyStash(QHash<Enums::ItemStorage::ItemStorageEnum, PlugyStashInfo>::iterator &iter, ItemsList *items);
    ItemInfo *parseItem(QDataStream &inputDataStream, const QByteArray &bytes);
    QMap<int, ItemProperty> parseItemProperties(ReverseBitReader &bitReader, bool *ok);
	void writeItems(const ItemsList &items, QDataStream &ds);
    void clearItems(bool sharedStashPathChanged = true, bool hcStashPathChanged = true);

	bool storeItemIn(Enums::ItemStorage::ItemStorageEnum storage, quint8 rows, quint8 cols, ItemInfo *item);
	bool canStoreItemAt(quint8 row, quint8 col, const QByteArray &itemType, const ItemsList &items) const;
};

#endif // MEDIANXLOFFLINETOOLS_H
