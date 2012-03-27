#include "medianxlofflinetools.h"
#include "fileassociationmanager.h"
#include "windowsincludes.h"

#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QSettings>

#define SHELL32_HANDLE GetModuleHandle(L"shell32.dll")

#ifdef WIN_7_OR_LATER
typedef HRESULT (__stdcall *PSCPEAUMID)(PCWSTR);                                 // SetCurrentProcessExplicitAppUserModelID()
typedef HRESULT (__stdcall *PSHCIFPN)(PCWSTR, IBindCtx *, const IID &, void **); // SHCreateItemFromParsingName()
#endif


// recent files

PCWSTR MedianXLOfflineTools::appUserModelID()
{
    static const QString progId = FileAssociationManager::progIdForExtension(characterExtensionWithDot);
    return progId.utf16();
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
                            
                            ::CoTaskMemFree(path);
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

                            ::CoTaskMemFree(path);
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
#ifdef WIN_7_OR_LATER
    PSHCIFPN pSHCreateItemFromParsingName = (PSHCIFPN)GetProcAddress(SHELL32_HANDLE, "SHCreateItemFromParsingName");
    if (pSHCreateItemFromParsingName)
    {
        IShellItem *pShellItem;
        HRESULT hr = pSHCreateItemFromParsingName(filePath.utf16(), NULL, IID_PPV_ARGS(&pShellItem));
        if (SUCCEEDED(hr))
        {
            SHARDAPPIDINFO info;
            info.psi = pShellItem;
            info.pszAppID = appUserModelID();
            SHAddToRecentDocs(SHARD_APPIDINFO, &info);
        }
        else
            qDebug("Error calling SHCreateItemFromParsingName(): %d", HRESULT_CODE(hr));
    }
    else
#endif
    {
        // just add to recent files on systems before Windows 7
        ::SHAddToRecentDocs(SHARD_PATHW, filePath.utf16());
    }
}
