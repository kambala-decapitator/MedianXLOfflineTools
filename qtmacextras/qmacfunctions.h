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

#ifndef QMACFUNCTIONS_H
#define QMACFUNCTIONS_H

#if 0
#pragma qt_class(QMacFunctions)
#endif

#include "qmacextrasglobal.h"

typedef struct CGImage *CGImageRef;
typedef struct CGContext *CGContextRef;

#ifdef __OBJC__
@class NSData;
@class NSImage;
@class NSString;
@class NSMenu;
@class NSURL;
#else
typedef struct objc_object NSData;
typedef struct objc_object NSImage;
typedef struct objc_object NSString;
typedef struct objc_object NSMenu;
typedef struct objc_object NSURL;
#endif

QT_BEGIN_NAMESPACE

class QByteArray;
class QMenu;
class QMenuBar;
class QPixmap;
class QString;
class QUrl;
class QWidget;
class QWindow;

namespace QtMacExtras
{
Q_MACEXTRAS_EXPORT NSString* toNSString(const QString &string);
Q_MACEXTRAS_EXPORT QString fromNSString(const NSString *string);

Q_MACEXTRAS_EXPORT NSURL* toNSURL(const QUrl &url);
Q_MACEXTRAS_EXPORT QUrl fromNSURL(const NSURL *url);

Q_MACEXTRAS_EXPORT NSData* toNSData(const QByteArray &data);
Q_MACEXTRAS_EXPORT QByteArray fromNSData(const NSData *data);

Q_MACEXTRAS_EXPORT CGImageRef toCGImageRef(const QPixmap &pixmap);
Q_MACEXTRAS_EXPORT QPixmap fromCGImageRef(CGImageRef image);

Q_MACEXTRAS_EXPORT CGContextRef currentCGContext();

#ifndef Q_OS_IOS
Q_MACEXTRAS_EXPORT NSImage* toNSImage(const QPixmap &pixmap);

Q_MACEXTRAS_EXPORT NSMenu* toNSMenu(QMenu *menu);
Q_MACEXTRAS_EXPORT NSMenu* toNSMenu(QMenuBar *menubar);

Q_MACEXTRAS_EXPORT void setDockMenu(QMenu *menu);

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
Q_MACEXTRAS_EXPORT bool isMainWindow(QWindow *window);
#endif

Q_MACEXTRAS_EXPORT bool isMainWindow(QWidget *widget);
#endif
}

#ifndef Q_OS_IOS
// ### Qt 4 compatibility; remove in Qt 6
inline void qt_mac_set_dock_menu(QMenu *menu) { QtMacExtras::setDockMenu(menu); }
#endif

QT_END_NAMESPACE

#endif // QMACFUNCTIONS_H
