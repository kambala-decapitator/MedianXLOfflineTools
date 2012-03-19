#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>


class MedianXLOfflineTools;
class QTimer;

class Application : public QApplication
{
    Q_OBJECT

public:
    explicit Application(int &argc, char **argv);
    virtual ~Application();

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
