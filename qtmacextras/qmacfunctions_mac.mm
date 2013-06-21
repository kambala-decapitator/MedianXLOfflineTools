/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtMacExtras module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qmacfunctions.h"
#include "qmacfunctions_p.h"
#import <Cocoa/Cocoa.h>

QT_BEGIN_NAMESPACE

#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
#include <QtGui/QWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <qpa/qplatformmenu.h>
#endif

namespace QtMacExtras
{

NSImage* toNSImage(const QPixmap &pixmap)
{
    NSBitmapImageRep *bitmapRep = [[NSBitmapImageRep alloc] initWithCGImage:toCGImageRef(pixmap)];
    NSImage *image = [[NSImage alloc] init];
    [image addRepresentation:bitmapRep];
    [bitmapRep release];
    return image;
}

NSMenu* toNSMenu(QMenu *menu)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    // Get the platform menu, which will be a QCocoaMenu
    QPlatformMenu *platformMenu = menu->platformMenu();

    // Get the qMenuToNSMenu function and call it.
    QPlatformNativeInterface::NativeResourceForIntegrationFunction function = resolvePlatformFunction("qmenutonsmenu");
    if (function) {
        typedef void* (*QMenuToNSMenuFunction)(QPlatformMenu *platformMenu);
        return reinterpret_cast<NSMenu *>(reinterpret_cast<QMenuToNSMenuFunction>(function)(platformMenu));
    }
    return nil;
#else
    return menu->nsMenu();
#endif
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
NSMenu* toNSMenu(QMenuBar *menubar)
{
    // Get the platform menubar, which will be a QCocoaMenuBar
    QPlatformMenuBar *platformMenuBar = menubar->platformMenuBar();

    // Get the qMenuBarToNSMenu function and call it.
    QPlatformNativeInterface::NativeResourceForIntegrationFunction function = resolvePlatformFunction("qmenubartonsmenu");
    if (function) {
        typedef void* (*QMenuBarToNSMenuFunction)(QPlatformMenuBar *platformMenuBar);
        return reinterpret_cast<NSMenu *>(reinterpret_cast<QMenuBarToNSMenuFunction>(function)(platformMenuBar));
    }
    return nil;
}
#endif

void setDockMenu(QMenu *menu)
{
    // Get the platform menu, which will be a QCocoaMenu
    QPlatformMenu *platformMenu = menu->platformMenu();

    // Get the setDockMenu function and call it.
    QPlatformNativeInterface::NativeResourceForIntegrationFunction function = resolvePlatformFunction("setdockmenu");
    if (function) {
        typedef void (*SetDockMenuFunction)(QPlatformMenu *platformMenu);
        reinterpret_cast<SetDockMenuFunction>(function)(platformMenu);
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
bool isMainWindow(QWindow *window)
{
    NSWindow *macWindow = static_cast<NSWindow*>(
        QGuiApplication::platformNativeInterface()->nativeResourceForWindow("nswindow", window));
    if (!macWindow)
        return false;

    return [macWindow isMainWindow];
}
#endif

bool isMainWindow(QWidget *widget)
{
    if (!widget)
        return false;

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    return isMainWindow(widget->windowHandle());
#else
    NSWindow *macWindow =
            reinterpret_cast<NSWindow*>([reinterpret_cast<NSView*>(widget->window()->winId()) window]);
    return [macWindow isMainWindow];
#endif
}

CGContextRef currentCGContext()
{
    return reinterpret_cast<CGContextRef>([[NSGraphicsContext currentContext] graphicsPort]);
}

} // namespace QtMacExtras

QT_END_NAMESPACE
