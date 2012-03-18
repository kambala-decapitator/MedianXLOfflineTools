#include "medianxlofflinetools.h"

#include <ApplicationServices/ApplicationServices.h>


void MedianXLOfflineTools::checkFileAssociations()
{
    FSRef defaultAppRef = {{0}}; // shut clang up
    CFStringRef characterExtensionCF = CFStringCreateWithCharacters(kCFAllocatorDefault, (const UniChar *)characterExtension.unicode(), characterExtension.length());
    OSStatus err;
    if ((err = LSGetApplicationForInfo(kLSUnknownType, kLSUnknownCreator, characterExtensionCF, kLSRolesAll, &defaultAppRef, NULL)) != noErr)
    {
        CFRelease(characterExtensionCF);
        qDebug("error getting default app: %d", err);
        return;
    }

    CFURLRef defaultAppUrl = CFURLCreateFromFSRef(kCFAllocatorDefault, &defaultAppRef);
    CFStringRef defaultAppPath = CFURLCopyFileSystemPath(defaultAppUrl, kCFURLPOSIXPathStyle);
    CFRelease(defaultAppUrl);

    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef bundleUrl = CFBundleCopyBundleURL(mainBundle);
    CFStringRef bundlePath = CFURLCopyFileSystemPath(bundleUrl, kCFURLPOSIXPathStyle);
    CFRelease(bundleUrl);

    bool isDefault = CFStringCompare(bundlePath, defaultAppPath, 0) == kCFCompareEqualTo;
    CFRelease(defaultAppPath);
    CFRelease(bundlePath);
    if (!isDefault)
    {
        CFArrayRef UTIs = UTTypeCreateAllIdentifiersForTag(kUTTagClassFilenameExtension, characterExtensionCF, nil);
        CFStringRef bundleIdentifier = CFBundleGetIdentifier(mainBundle);
        for (CFIndex i = 0, n = CFArrayGetCount(UTIs); i < n; ++i)
        {
            CFStringRef UTI = (CFStringRef)CFArrayGetValueAtIndex(UTIs, i);
            CFShow(UTI);
            if ((err = LSSetDefaultRoleHandlerForContentType(UTI, kLSRolesAll, bundleIdentifier)) == noErr)
                qDebug("app registered as default");
            else
                qDebug("error registering app as default: %d", err);
        }
        CFRelease(UTIs);
    }
    else
        qDebug("app is default already");

    CFRelease(characterExtensionCF);
}
