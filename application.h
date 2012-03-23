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

//public slots:
//    void activateWindow();

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

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
    void disableLionWindowRestoration();
#endif
#endif
};

#endif // APPLICATION_H
