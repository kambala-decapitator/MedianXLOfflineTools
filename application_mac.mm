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
            maybeDeleteTimer();

            createAndShowMainWindow();
        }
        else
            _mainWindow->loadFile(_param);
        return true;
    }
    return QtSingleApplication::event(ev);
}


void Application::maybeDeleteTimer()
{
    if (_showWindowMacTimer)
    {
        delete _showWindowMacTimer;
        _showWindowMacTimer = 0;
    }
}


#if MAC_OS_X_VERSION_MAX_ALLOWED < 1070
@interface NSWindow (RestorationHackForOldSDKs)
- (void)setRestorationClass:(Class)restorationClass;
- (void)setRestorable:(BOOL)flag;
- (void)invalidateRestorableState;
@end
#endif

void Application::disableLionWindowRestoration()
{
    NSWindow *window = [reinterpret_cast<NSView *>(_mainWindow->winId()) window];
    if ([window respondsToSelector:@selector(setRestorationClass:)] && [window respondsToSelector:@selector(setRestorable:)] && [window respondsToSelector:@selector(invalidateRestorableState)])
    {
        [window setRestorationClass:nil];
        [window setRestorable:NO];
        [window invalidateRestorableState];
    }
}
