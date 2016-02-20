#ifndef DUPESCANDIALOG_H
#define DUPESCANDIALOG_H

#include <QDialog>


class QLineEdit;
class QTextEdit;
class QPushButton;
class QCheckBox;
class QProgressBar;

class ItemInfo;

class DupeScanDialog : public QDialog
{
    Q_OBJECT

public:
    DupeScanDialog(const QString &currentPath = QString(), QWidget *parent = 0);
    virtual ~DupeScanDialog() {}

    void logLoadingError(const QString &error, bool warn) { _loadingMessage = QString("<font color=%1>%2</font>").arg(warn ? "yellow" : "red", error); }

signals:
    void loadFile(const QString &file);

private slots:
    void selectPath();
    void scan();
    void scanFinished();
    void save();
    void updateProgressbarForCrossCheck(int n);

private:
    QString _currentCharPath, _loadingMessage;
    QLineEdit *_pathLineEdit;
    QTextEdit *_logBrowser;
    QPushButton *_saveButton;
    QCheckBox *_dontWriteCheckBox;
    QProgressBar *_progressBar;

    void logDupedItemsInfo(ItemInfo *item1, ItemInfo *item2);
    void appendStringToLog(const QString &s);
    void scanCharactersInDir(const QString &path);

    bool shouldCheckItem(ItemInfo *item);
    bool areItemsSame(ItemInfo *a, ItemInfo *b);
};

#endif // DUPESCANDIALOG_H
