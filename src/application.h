#ifndef APPLICATION_H
#define APPLICATION_H

#include "medianxlofflinetools.h"

#if HAS_QTSINGLEAPPLICATION
#include "QtSingleApplication"
#define QAPP_CLASS QtSingleApplication
#else
#include <QApplication>
#define QAPP_CLASS QApplication
#endif

#ifdef Q_OS_MAC
#include <QPointer>
#endif


class QTimer;

class Application : public QAPP_CLASS
{
    Q_OBJECT

public:
    explicit Application(int &argc, char **argv);
    virtual ~Application();

    void init();

public slots:
#if HAS_QTSINGLEAPPLICATION
    void activateWindow();
#endif

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
