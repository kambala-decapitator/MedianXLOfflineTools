#include "medianxlofflinetools.h"

#import <AppKit/NSMenu.h>
#import <AppKit/NSMenuItem.h>


void MedianXLOfflineTools::moveUpdateAction()
{
    NSMenu *firstMenu = [[[NSApp mainMenu] itemAtIndex:0] submenu];
    NSLog(@"%@", [NSApp mainMenu]);
}
