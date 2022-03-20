#ifndef APPLICATION_H
#define APPLICATION_H

#include "qtsingleapplication/QtSingleApplication"
#include "medianxlofflinetools.h"

#ifdef Q_OS_MAC
#include <QPointer>
#endif


class QTimer;

class Application : public QtSingleApplication
{
    Q_OBJECT

public:
    explicit Application(int &argc, char **argv);
    virtual ~Application();

    void init();

public slots:
    void activateWindow();

protected:
#ifdef Q_OS_MAC
    bool event(QEvent *ev);
#endif

private slots:
    void setParam(const QString &param) { _param = param; }
    void createAndShowMainWindow();

private:
    MedianXLOfflineTools *_mainWindow;
    QString _param;
    LaunchMode _launchMode;
#ifdef Q_OS_MAC
    QPointer<QTimer> _showWindowMacTimer;

    void disableLionWindowRestoration();
#endif
};

#endif // APPLICATION_H
