#ifndef DUPESCANDIALOG_H
#define DUPESCANDIALOG_H

#include <QDialog>
#include <QFutureWatcher>
#include <QTime>
#include "structs.h"

class QLineEdit;
class QTextEdit;
class QPushButton;
class QCheckBox;
class QProgressBar;

typedef QHash<QString, ItemsList> ItemsHash;
typedef ItemsHash::const_iterator ItemsHashIterator;

class DupeScanDialog : public QDialog
{
    Q_OBJECT

public:
    DupeScanDialog(const QString &currentPath = QString(), bool isDumpItemsMode = false, QWidget *parent = 0);
    virtual ~DupeScanDialog();

    void logLoadingError(const QString &error, bool warn) { _loadingMessage = QString("<font color=%1>%2</font>").arg(warn ? "yellow" : "red", error); }

public slots:
    void done(int r);

signals:
    void loadFile(const QString &file);
    void scanFinished();

private slots:
    void selectPath();
    void scan();
    void scanFinished_();
    void save();
    void crossCheckResultReady(int i);

private:
    QString _currentCharPath, _loadingMessage;
    bool _isDumpItemsMode;
    ItemsHash _allItemsHash;
    QFutureWatcher<QString> *_futureWatcher;
    QTime _timeCounter;
    bool _isAutoLaunched, _isVerbose;

    QLineEdit *_pathLineEdit;
    QTextEdit *_logBrowser;
    QPushButton *_saveButton;
    QCheckBox *_skipEmptyCheckBox;
    QProgressBar *_progressBar;

    void appendStringToLog(const QString &s);
    void scanCharactersInDir(const QString &path);
    bool saveLog(const QString &fileName, bool isPlainText = true);
    QString baseDupeScanLogFileName();
    void addItemName(ItemInfo *item, QString &itemsStr);
};

#endif // DUPESCANDIALOG_H
