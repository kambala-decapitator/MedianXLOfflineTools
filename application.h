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

    bool shouldAllowShowMainWindow() const { return _shouldAllowShowMainWindow; }

public slots:
    void createAndShowMainWindow();
    void activateWindow();

protected:
#ifdef Q_WS_MACX
    bool event(QEvent *ev);
#endif

private slots:
    void setParam(const QString &param) { _param = param; }
    void allowShowMainWindow();

private:
    MedianXLOfflineTools *_mainWindow;
    QString _param;
    bool _shouldAllowShowMainWindow;
#ifdef Q_WS_MACX
    QTimer *_showWindowMacTimer;

    void disableLionWindowRestoration();
#endif
};

#endif // APPLICATION_H
