#ifndef APPLICATION_H
#define APPLICATION_H

#include "qtsingleapplication/qtsingleapplication.h"


class MedianXLOfflineTools;
class QTimer;

class Application : public QtSingleApplication
{
    Q_OBJECT

public:
    explicit Application(int &argc, char **argv);
    virtual ~Application();

    static const QString appName;

public slots:
#ifdef Q_WS_WIN32
    // use WinAPI instead of default implementation to bring window to foreground
    void activateWindow() {}
    void activateWindowWinAPI();
#endif

protected:
#ifdef Q_WS_MACX
    bool event(QEvent *ev);
#endif

private slots:
    void createAndShowMainWindow();

private:
    MedianXLOfflineTools *_mainWindow;
    QString _param;
#ifdef Q_WS_MACX
    QTimer *_showWindowMacTimer;

    void disableLionWindowRestoration();
#endif
};

#endif // APPLICATION_H
