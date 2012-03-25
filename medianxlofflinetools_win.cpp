#include "medianxlofflinetools.h"

#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QSettings>

#if defined(NTDDI_VISTA) || defined(_WIN32_WINNT_VISTA)
#    define WIN_VISTA_OR_LATER
#    if defined(NTDDI_WIN7) || defined(_WIN32_WINNT_WIN7)
#        define WIN_7_OR_LATER
#    endif
#endif

#include <Shlobj.h>
#ifdef WIN_VISTA_OR_LATER
#include <Shobjidl.h>
#endif

#define SHELL32_HANDLE GetModuleHandle(L"shell32.dll")

#ifdef WIN_7_OR_LATER
typedef HRESULT (__stdcall *PSCPEAUMID)(PCWSTR);                                 // SetCurrentProcessExplicitAppUserModelID()
typedef HRESULT (__stdcall *PSHCIFPN)(PCWSTR, IBindCtx *, const IID &, void **); // SHCreateItemFromParsingName()
#endif
typedef void (__stdcall *PSHATRD)(UINT, LPCVOID);                                // SHAddToRecentDocs()


// recent files

const QString &MedianXLOfflineTools::progID()
{
    static const QString progId = QString("%1.%2").arg(qApp->applicationName().remove(' '), characterExtension);
    return progId;
}

LPCWSTR MedianXLOfflineTools::appUserModelID()
{
    return progID().utf16();
}

void MedianXLOfflineTools::setAppUserModelID()
{
#ifdef WIN_7_OR_LATER
    PSCPEAUMID pSetCurrentProcessExplicitAppUserModelID = (PSCPEAUMID)GetProcAddress(SHELL32_HANDLE, "SetCurrentProcessExplicitAppUserModelID");
    if (pSetCurrentProcessExplicitAppUserModelID)
    {
        HRESULT hr = pSetCurrentProcessExplicitAppUserModelID(appUserModelID());
        if (SUCCEEDED(hr))
            qDebug("SetCurrentProcessExplicitAppUserModelID success");
        else
            qDebug("SetCurrentProcessExplicitAppUserModelID error: %d", HRESULT_CODE(hr));
    }
#endif
}

void MedianXLOfflineTools::syncWindowsTaskbarRecentFiles()
{
#ifdef WIN_7_OR_LATER
    qDebug("sync recent");
    IApplicationDocumentLists *pADL;
    HRESULT hr = CoCreateInstance(CLSID_ApplicationDocumentLists, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pADL));
    if (SUCCEEDED(hr))
    {
        if (SUCCEEDED(hr = pADL->SetAppID(appUserModelID())))
        {
            IObjectArray *pRecentItemsArray;
            if (SUCCEEDED(hr = pADL->GetList(ADLT_RECENT, maxRecentFiles, IID_PPV_ARGS(&pRecentItemsArray))))
            {
                UINT n;
                if (SUCCEEDED(hr = pRecentItemsArray->GetCount(&n)))
                {
                    qDebug("got %u recent items", n);
                    QStringList recentFilesForTaskbar(_recentFilesList);
                    for (UINT i = 0; i < n; ++i)
                    {
                        IShellItem *pShellItem;
                        if (SUCCEEDED(hr = pRecentItemsArray->GetAt(i, IID_PPV_ARGS(&pShellItem))))
                        {
                            LPWSTR path = NULL;
                            if (SUCCEEDED(hr = pShellItem->GetDisplayName(SIGDN_FILESYSPATH, &path)))
                                recentFilesForTaskbar.removeOne(QString::fromUtf16(path));
                            else
                                qDebug("Error calling GetPath(): %d", HRESULT_CODE(hr));
                            CoTaskMemFree(path);
                        }
                        else
                            qDebug("Error calling GetAt(): %d", HRESULT_CODE(hr));
                    }

                    foreach (const QString &recentFile, recentFilesForTaskbar)
                        addToWindowsRecentFiles(recentFile);
                }
                else
                    qDebug("Error calling GetCount(): %d", HRESULT_CODE(hr));
            }
            else
                qDebug("Error calling GetList(): %d", HRESULT_CODE(hr));
        }
        else
            qDebug("Error calling SetAppID(): %d", HRESULT_CODE(hr));

        pADL->Release();
    }
    else
        qDebug("Error calling CoCreateInstance(CLSID_ApplicationDocumentLists): %d", HRESULT_CODE(hr));
    qDebug("-----");
#endif
}

void MedianXLOfflineTools::removeFromWindowsRecentFiles(const QString &filePath)
{
#ifdef WIN_7_OR_LATER
    qDebug("remove from recent");
    IApplicationDocumentLists *pADL;
    HRESULT hr = CoCreateInstance(CLSID_ApplicationDocumentLists, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pADL));
    if (SUCCEEDED(hr))
    {
        if (SUCCEEDED(hr = pADL->SetAppID(appUserModelID())))
        {
            IObjectArray *pRecentItemsArray;
            if (SUCCEEDED(hr = pADL->GetList(ADLT_RECENT, maxRecentFiles, IID_PPV_ARGS(&pRecentItemsArray))))
            {
                UINT n;
                if (SUCCEEDED(hr = pRecentItemsArray->GetCount(&n)))
                {
                    qDebug("got %u recent items", n);
                    for (UINT i = 0; i < n; ++i)
                    {
                        IShellItem *pShellItem;
                        if (SUCCEEDED(hr = pRecentItemsArray->GetAt(i, IID_PPV_ARGS(&pShellItem))))
                        {
                            LPWSTR path = NULL;
                            if (SUCCEEDED(hr = pShellItem->GetDisplayName(SIGDN_FILESYSPATH, &path)))
                            {
                                if (!wcscmp(path, QDir::toNativeSeparators(filePath).utf16()))
                                {
                                    IApplicationDestinations *pAD;
                                    HRESULT hr = CoCreateInstance(CLSID_ApplicationDestinations, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pAD));
                                    if (SUCCEEDED(hr))
                                    {
                                        if (SUCCEEDED(hr = pAD->SetAppID(appUserModelID())))
                                        {
                                            if (SUCCEEDED(hr = pAD->RemoveDestination(pShellItem)))
                                                qDebug("successfully removed");
                                            else
                                                qDebug("Error calling RemoveDestination(): %d", HRESULT_CODE(hr));
                                        }
                                        else
                                            qDebug("Error calling SetAppID(): %d", HRESULT_CODE(hr));

                                        pAD->Release();
                                    }
                                    else
                                        qDebug("Error calling CoCreateInstance(CLSID_ApplicationDestinations): %d", HRESULT_CODE(hr));

                                    break;
                                }
                            }
                            else
                                qDebug("Error calling GetDisplayName(): %d", HRESULT_CODE(hr));

                            CoTaskMemFree(path);
                        }
                        else
                            qDebug("Error calling GetAt(): %d", HRESULT_CODE(hr));
                    }
                }
                else
                    qDebug("Error calling GetCount(): %d", HRESULT_CODE(hr));
            }
            else
                qDebug("Error calling GetList(): %d", HRESULT_CODE(hr));
        }
        else
            qDebug("Error calling SetAppID(): %d", HRESULT_CODE(hr));

        pADL->Release();
    }
    else
        qDebug("Error calling CoCreateInstance(CLSID_ApplicationDocumentLists): %d", HRESULT_CODE(hr));
    qDebug("-----");
#endif
    
}

void MedianXLOfflineTools::addToWindowsRecentFiles(const QString &filePath)
{
    qDebug("add '%s' to recent", qPrintable(filePath));
    HMODULE shell32Handle = SHELL32_HANDLE;
    PSHATRD pSHAddToRecentDocs = (PSHATRD)GetProcAddress(shell32Handle, "SHAddToRecentDocs");
#ifdef WIN_7_OR_LATER
    PSHCIFPN pSHCreateItemFromParsingName = (PSHCIFPN)GetProcAddress(shell32Handle, "SHCreateItemFromParsingName");
    if (pSHCreateItemFromParsingName)
    {
        IShellItem *pShellItem;
        HRESULT hr = pSHCreateItemFromParsingName(filePath.utf16(), NULL, IID_PPV_ARGS(&pShellItem));
        if (SUCCEEDED(hr))
        {
            SHARDAPPIDINFO info;
            info.psi = pShellItem;
            info.pszAppID = appUserModelID();
            pSHAddToRecentDocs(SHARD_APPIDINFO, &info);
        }
        else
            qDebug("Error calling SHCreateItemFromParsingName(): %d", HRESULT_CODE(hr));
    }
    else
#endif
    if (pSHAddToRecentDocs)
    {
        // just add to recent files on systems before Windows 7
        pSHAddToRecentDocs(SHARD_PATHW, filePath.utf16());
    }
}


// file associations

bool isDefaultBeforeVista()
{
    return true;
}

void MedianXLOfflineTools::checkFileAssociations()
{
    QString appPath = QDir::toNativeSeparators(qApp->applicationFilePath()), binaryName = QFileInfo(appPath).fileName(), defaultValueFormat("%1/."), cmdOpenFileFormat("%1/shell/open/command/.");
    QString registryAppPath = QString("Applications/%1").arg(binaryName), binaryPathWithParam = QString("\"%1\" \"%2\"").arg(appPath, "%1"); // "path/to/exe" "%1"
    QString defaultApplicationRegistryPath = cmdOpenFileFormat.arg(progID());

    QSettings hklmSoftware("HKEY_LOCAL_MACHINE\\Software", QSettings::NativeFormat);
    if (QSysInfo::windowsVersion() < QSysInfo::WV_VISTA)
    {
        // check key "Application" in HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.d2s\ (if present, contains some binary name)
        hklmSoftware.beginGroup("Classes");
        if (hklmSoftware.value(defaultApplicationRegistryPath).toString() != binaryPathWithParam)
        {
            // TODO: same code in Vista+ section
            hklmSoftware.setValue(defaultValueFormat.arg(characterExtensionWithDot), progID());
            hklmSoftware.setValue(defaultValueFormat.arg(progID()), "Diablo 2 character save file");
            hklmSoftware.setValue(defaultApplicationRegistryPath, binaryPathWithParam);

            hklmSoftware.setValue(cmdOpenFileFormat.arg(registryAppPath), binaryPathWithParam);
            hklmSoftware.setValue(QString("%1/SupportedTypes/%2").arg(registryAppPath, characterExtensionWithDot), QString("")); // empty string must be set, but not null string
            hklmSoftware.endGroup();

            hklmSoftware.beginGroup("Microsoft/Windows/CurrentVersion/App Paths");
            hklmSoftware.setValue(defaultValueFormat.arg(binaryName), appPath);
            hklmSoftware.setValue(QString("%1/Path").arg(binaryName), QDir::toNativeSeparators(qApp->applicationDirPath()));
            // end

            SHChangeNotify(SHCNE_ASSOCCHANGED, 0, 0, 0);
            qDebug("file association changed");
        }
        hklmSoftware.endGroup();
    }
#ifdef WIN_VISTA_OR_LATER
    else
    {
        IApplicationAssociationRegistration *pAAR;
        HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistration, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pAAR));
        if (SUCCEEDED(hr))
        {
            QString appName = qApp->applicationName();
            LPCWSTR appNameWstr = appName.utf16(), extensionWithDotWstr = characterExtensionWithDot.utf16();

            BOOL isDefault;
            hr = pAAR->QueryAppIsDefault(extensionWithDotWstr, AT_FILEEXTENSION, AL_EFFECTIVE, appNameWstr, &isDefault);
            if (HRESULT_CODE(hr) == ERROR_FILE_NOT_FOUND)
            {
                qDebug("app is not in the registry, adding it");

                // add info about our app to the registry

                // TODO: it's ugly copypaste from above
                hklmSoftware.beginGroup("Classes");
                hklmSoftware.setValue(defaultValueFormat.arg(characterExtensionWithDot), progID());

                hklmSoftware.setValue(defaultValueFormat.arg(progID()), "Diablo 2 character save file");
                hklmSoftware.setValue(defaultApplicationRegistryPath, binaryPathWithParam);

                hklmSoftware.setValue(cmdOpenFileFormat.arg(registryAppPath), binaryPathWithParam);
                hklmSoftware.setValue(QString("%1/SupportedTypes/%2").arg(registryAppPath, characterExtensionWithDot), QString(""));
                hklmSoftware.endGroup();

                hklmSoftware.beginGroup("Microsoft/Windows/CurrentVersion/App Paths");
                hklmSoftware.setValue(defaultValueFormat.arg(binaryName), appPath);
                hklmSoftware.setValue(QString("%1/Path").arg(binaryName), QDir::toNativeSeparators(qApp->applicationDirPath()));
                hklmSoftware.endGroup();
                // end

                // add to HKLM\Software\%App%\Capabilities and HKLM\Software\RegisteredApplications
                QString appRegistryPath = QString("kambala/%1/Capabilities").arg(appName);
                hklmSoftware.beginGroup(appRegistryPath);
                hklmSoftware.setValue("ApplicationName", appName);
                hklmSoftware.setValue("ApplicationDescription", tr("Offline analogue of grig's Median XL Online Tools"));
                hklmSoftware.setValue("FileAssociations/" + characterExtensionWithDot, progID());
                hklmSoftware.endGroup();

                hklmSoftware.setValue("RegisteredApplications/" + appName, "Software\\" + QDir::toNativeSeparators(appRegistryPath));

                hr = S_OK;
            }
            else if (FAILED(hr))
                ERROR_BOX(tr("Error calling QueryAppIsDefault(): %1").arg(HRESULT_CODE(hr)));
            else if (isDefault)
                qDebug("app is default already");

            if (!isDefault && SUCCEEDED(hr))
            {
                //LPWSTR defaultAppNameWstr;
                // returns HRESULT_FROM_WIN32(ERROR_NO_ASSOCIATION) if no app is associated
                //hr = pAAR->QueryCurrentDefault(extensionWithDotWstr, AT_FILEEXTENSION, AL_EFFECTIVE, &defaultAppNameWstr);
                //if (SUCCEEDED(hr))
                //{
                //    OutputDebugString(defaultAppNameWstr);
                //    qDebug(" - default app");
                //}
                //else
                //    qDebug("QueryCurrentDefault() failed with result: %ld", HRESULT_CODE(hr));

                // returns HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) if extension isn't registered
                hr = pAAR->SetAppAsDefault(appNameWstr, extensionWithDotWstr, AT_FILEEXTENSION);
                if (SUCCEEDED(hr))
                    qDebug("app is default now");
                else
                    qDebug("SetAppAsDefault() failed with result: %ld", HRESULT_CODE(hr));
            }

            pAAR->Release();
        }
        else
            ERROR_BOX(tr("Error calling CoCreateInstance(CLSID_ApplicationAssociationRegistration): %1").arg(HRESULT_CODE(hr)));
    }
#endif
}
