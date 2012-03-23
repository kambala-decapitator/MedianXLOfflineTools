#include "application.h"
#include "medianxlofflinetools.h"

#include <QFileOpenEvent>
#include <QTimer>


bool Application::event(QEvent *ev)
{
    if (ev->type() == QEvent::FileOpen)
    {
        _param = static_cast<QFileOpenEvent *>(ev)->file();
        if (!_mainWindow)
        {
            _showWindowMacTimer->stop();
            delete _showWindowMacTimer;
            _showWindowMacTimer = 0;

            createAndShowMainWindow();
        }
        else
            _mainWindow->loadFile(_param);
        return true;
    }
    return QApplication::event(ev);
}


void Application::disableLionWindowRestoration()
{
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
    NSWindow *window = [reinterpret_cast<NSView *>(_mainWindow->winId()) window];
    [window setRestorationClass:nil];
    [window setRestorable:NO];
    [window invalidateRestorableState];
#endif
}
