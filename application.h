#ifndef APPLICATION_H
#define APPLICATION_H

#include "qtsingleapplication/QtSingleApplication"


class MedianXLOfflineTools;
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
#ifdef Q_OS_MAC
    QTimer *_showWindowMacTimer;

    void disableLionWindowRestoration();
    void maybeDeleteTimer();
#endif
};

#endif // APPLICATION_H
