#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>


class MedianXLOfflineTools;

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

private:
    MedianXLOfflineTools *_mainWindow;
};

#endif // APPLICATION_H
