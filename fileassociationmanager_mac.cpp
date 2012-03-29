#include "fileassociationmanager.h"
#include "helpers.h"

#include <QString>

#include <ApplicationServices/ApplicationServices.h>


// helpers

QString extensionWithoutDotFromExtension(const QString &extension)
{
    return extension.startsWith('.') ? extension.mid(1) : extension;
}


// FileAssociationManager implementation

bool FileAssociationManager::isApplicationDefaultForExtension(const QString &extension)
{
    QString extensionWithoutDot = extensionWithoutDotFromExtension(extension);
    CFStringRef extensionWithoutDotCF = CFStringCreateWithCharacters(kCFAllocatorDefault, (const UniChar *)extensionWithoutDot.unicode(), extensionWithoutDot.length());

    bool isDefault;
    FSRef defaultAppRef = {{0}}; // shut clang up
    OSStatus err = LSGetApplicationForInfo(kLSUnknownType, kLSUnknownCreator, extensionWithoutDotCF, kLSRolesAll, &defaultAppRef, NULL);
    if (err == noErr)
    {
        CFURLRef defaultAppUrl = CFURLCreateFromFSRef(kCFAllocatorDefault, &defaultAppRef);
        CFStringRef defaultAppPath = CFURLCopyFileSystemPath(defaultAppUrl, kCFURLPOSIXPathStyle);
        CFRelease(defaultAppUrl);

        CFBundleRef mainBundle = CFBundleGetMainBundle();
        CFURLRef bundleUrl = CFBundleCopyBundleURL(mainBundle);
        CFStringRef bundlePath = CFURLCopyFileSystemPath(bundleUrl, kCFURLPOSIXPathStyle);
        CFRelease(bundleUrl);

        isDefault = CFStringCompare(bundlePath, defaultAppPath, 0) == kCFCompareEqualTo;

        CFRelease(bundlePath);
        CFRelease(defaultAppPath);
    }
    else if (err == kLSApplicationNotFoundErr)
        isDefault = false;
    else
    {
        isDefault = true; // don't try to register in case of error
        ERROR_BOX_NO_PARENT(QString("Error calling LSGetApplicationForInfo(): %1").arg(err));
    }

    CFRelease(extensionWithoutDotCF);

    return isDefault;
}

void FileAssociationManager::makeApplicationDefaultForExtension(const QString &extension)
{
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFStringRef bundleIdentifier = CFBundleGetIdentifier(mainBundle);

    QString extensionWithoutDot = extensionWithoutDotFromExtension(extension);
    CFStringRef extensionWithoutDotCF = CFStringCreateWithCharacters(kCFAllocatorDefault, (const UniChar *)extensionWithoutDot.unicode(), extensionWithoutDot.length());
    CFArrayRef UTIs = UTTypeCreateAllIdentifiersForTag(kUTTagClassFilenameExtension, extensionWithoutDotCF, nil);
    CFRelease(extensionWithoutDotCF);

    for (CFIndex i = 0, n = CFArrayGetCount(UTIs); i < n; ++i)
    {
        CFStringRef UTI = (CFStringRef)CFArrayGetValueAtIndex(UTIs, i);
        OSStatus err = LSSetDefaultRoleHandlerForContentType(UTI, kLSRolesAll, bundleIdentifier);
        if (err == noErr)
            qDebug("app is default now");
        else
            ERROR_BOX_NO_PARENT(QString("Error calling LSSetDefaultRoleHandlerForContentType(): %1").arg(err));
    }
    
    CFRelease(UTIs);
}
