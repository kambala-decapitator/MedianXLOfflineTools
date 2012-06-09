#include "medianxlofflinetools.h"

#import "machelpers.h"

#import <AppKit/NSMenu.h>
#import <AppKit/NSMenuItem.h>


void MedianXLOfflineTools::moveUpdateActionToAppleMenu()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    NSMenu *mainMenu = [NSApp mainMenu];
    NSMenuItem *helpMenuItem = [mainMenu itemWithTitle:NSStringFromQString(ui.menuHelp->title())];
    NSMenu *helpMenu = [helpMenuItem submenu];
    NSMenuItem *checkForUpdateMenuItem = [helpMenu itemAtIndex:0];

    [helpMenu removeItem:checkForUpdateMenuItem];
    if ([helpMenu numberOfItems] == 1) // only 'find' left
        [mainMenu removeItem:helpMenuItem];
    [[[mainMenu itemAtIndex:0] submenu] insertItem:checkForUpdateMenuItem atIndex:3];

    [pool release];
}
