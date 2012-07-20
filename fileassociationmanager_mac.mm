#include "fileassociationmanager.h"
#include "helpers.h"

#import "machelpers.h"

#include <QString>
#include <QCoreApplication>

#include <ApplicationServices/ApplicationServices.h>


// helpers

QString extensionWithoutDotFromExtension(const QString &extension)
{
    return extension.startsWith('.') ? extension.mid(1) : extension;
}


// FileAssociationManager implementation

bool FileAssociationManager::isApplicationDefaultForExtension(const QString &extension)
{
    bool isDefault;
    FSRef defaultAppRef = {{0}}; // shut clang up
    OSStatus err = LSGetApplicationForInfo(kLSUnknownType, kLSUnknownCreator, (CFStringRef)NSStringFromQString(extensionWithoutDotFromExtension(extension)), kLSRolesAll, &defaultAppRef, NULL);
    if (err == noErr)
    {
        CFURLRef defaultAppUrl = CFURLCreateFromFSRef(kCFAllocatorDefault, &defaultAppRef), bundleUrl = CFBundleCopyBundleURL(CFBundleGetMainBundle());
        isDefault = CFEqual(defaultAppUrl, bundleUrl);
        CFRelease(defaultAppUrl);
        CFRelease(bundleUrl);
    }
    else if (err == kLSApplicationNotFoundErr)
        isDefault = false;
    else
    {
        isDefault = true; // don't try to register in case of error
        ERROR_BOX_NO_PARENT(QString("Error calling LSGetApplicationForInfo(): %1").arg(err));
    }

    return isDefault;
}

void FileAssociationManager::makeApplicationDefaultForExtension(const QString &extension)
{
    CFStringRef bundleIdentifier = CFBundleGetIdentifier(CFBundleGetMainBundle());
    CFArrayRef UTIs = UTTypeCreateAllIdentifiersForTag(kUTTagClassFilenameExtension, (CFStringRef)NSStringFromQString(extensionWithoutDotFromExtension(extension)), nil);
    for (CFIndex i = 0, n = CFArrayGetCount(UTIs); i < n; ++i)
    {
        CFStringRef UTI = (CFStringRef)CFArrayGetValueAtIndex(UTIs, i);
        OSStatus err = LSSetDefaultRoleHandlerForContentType(UTI, kLSRolesAll, bundleIdentifier);
        if (err != noErr)
            ERROR_BOX_NO_PARENT(QString("Error calling LSSetDefaultRoleHandlerForContentType(): %1").arg(err));
    }
    
    CFRelease(UTIs);
}
