#include "application.h"
#include "medianxlofflinetools.h"

//#import <AppKit/NSWindow.h>


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
