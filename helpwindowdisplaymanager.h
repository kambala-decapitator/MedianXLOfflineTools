#ifndef HELPWINDOWDISPLAYMANAGER_H
#define HELPWINDOWDISPLAYMANAGER_H

#include <QObject>

struct HelpWindowDisplayManagerImpl;

class HelpWindowDisplayManager : public QObject
{
    Q_OBJECT

public:
    HelpWindowDisplayManager(QObject *parent, const QString &windowTitle, const QString &helpText) : QObject(parent), _helpWindowTitle(windowTitle), _helpWindowText(helpText), _impl(0) {}
    virtual ~HelpWindowDisplayManager();

public slots:
    void showHelp();
    void closeHelp();

private:
    QString _helpWindowTitle, _helpWindowText;
    HelpWindowDisplayManagerImpl *_impl;
};

#endif // HELPWINDOWDISPLAYMANAGER_H
