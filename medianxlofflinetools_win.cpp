#include "medianxlofflinetools.h"
#include "fileassociationmanager.h"
#include "windowsincludes.h"

#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QSettings>

#define SHELL32_HANDLE GetModuleHandle(L"shell32.dll")

// WinXP toolchain doesn't define consts below
#ifndef PRODUCT_CORE_N
#define PRODUCT_CORE_N                              0x00000062
#endif
#ifndef PRODUCT_CORE_COUNTRYSPECIFIC
#define PRODUCT_CORE_COUNTRYSPECIFIC                0x00000063
#endif
#ifndef PRODUCT_PROFESSIONAL_WMC
#define PRODUCT_PROFESSIONAL_WMC                    0x00000067
#endif
#ifndef PRODUCT_CORE
#define PRODUCT_CORE                                0x00000065
#endif
#ifndef PRODUCT_DATACENTER_EVALUATION_SERVER
#define PRODUCT_DATACENTER_EVALUATION_SERVER        0x00000050
#endif
#ifndef PRODUCT_ENTERPRISE_EVALUATION
#define PRODUCT_ENTERPRISE_EVALUATION               0x00000048
#endif
#ifndef PRODUCT_ENTERPRISE_N_EVALUATION
#define PRODUCT_ENTERPRISE_N_EVALUATION             0x00000054
#endif
#ifndef PRODUCT_CORE_SINGLELANGUAGE
#define PRODUCT_CORE_SINGLELANGUAGE                 0x00000064
#endif
#ifndef PRODUCT_EMBEDDED_AUTOMOTIVE
#define PRODUCT_EMBEDDED_AUTOMOTIVE                 0x00000055
#endif
#ifndef PRODUCT_EMBEDDED_INDUSTRY_A
#define PRODUCT_EMBEDDED_INDUSTRY_A                 0x00000056
#endif
#ifndef PRODUCT_EMBEDDED_A
#define PRODUCT_EMBEDDED_A                          0x00000058
#endif
#ifndef PRODUCT_EMBEDDED_INDUSTRY
#define PRODUCT_EMBEDDED_INDUSTRY                   0x00000059
#endif
#ifndef PRODUCT_EMBEDDED_E
#define PRODUCT_EMBEDDED_E                          0x0000005A
#endif
#ifndef PRODUCT_EMBEDDED_INDUSTRY_E
#define PRODUCT_EMBEDDED_INDUSTRY_E                 0x0000005B
#endif
#ifndef PRODUCT_EMBEDDED_INDUSTRY_A_E
#define PRODUCT_EMBEDDED_INDUSTRY_A_E               0x0000005C
#endif

typedef HRESULT (__stdcall *PSCPEAUMID)(PCWSTR); // SetCurrentProcessExplicitAppUserModelID()
#ifdef WIN_7_OR_LATER
typedef HRESULT (__stdcall *PSHCIFPN)(PCWSTR, IBindCtx *, const IID &, void **); // SHCreateItemFromParsingName()
#endif

typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO); // GetNativeSystemInfo()
typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD); // GetProductInfo()


PCWSTR MedianXLOfflineTools::appUserModelID()
{
    static const QString progId = FileAssociationManager::progIdForExtension(kCharacterExtensionWithDot);
    return QSTRING_TO_LPCWSTR(progId);
}

void MedianXLOfflineTools::setAppUserModelID()
{
    PSCPEAUMID pSetCurrentProcessExplicitAppUserModelID = (PSCPEAUMID)GetProcAddress(SHELL32_HANDLE, "SetCurrentProcessExplicitAppUserModelID");
    if (pSetCurrentProcessExplicitAppUserModelID)
        pSetCurrentProcessExplicitAppUserModelID(appUserModelID());
}


void MedianXLOfflineTools::showFileAssocaitionUI()
{
    FileAssociationManager::registerApplicationForExtension(kCharacterExtensionWithDot);

#ifdef WIN_VISTA_OR_LATER
    IApplicationAssociationRegistrationUI *pAARUI;
    HRESULT hr = ::CoCreateInstance(CLSID_ApplicationAssociationRegistrationUI, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pAARUI));
    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = pAARUI->LaunchAdvancedAssociationUI(QSTRING_TO_LPCWSTR(qApp->applicationName()))))
            ERROR_BOX(QString("Error calling LaunchAdvancedAssociationUI: %1").arg(HRESULT_CODE(hr)));
        pAARUI->Release();
    }
    else
        ERROR_BOX(QString("Error calling CoCreateInstance(CLSID_ApplicationAssociationRegistrationUI): %1").arg(HRESULT_CODE(hr)));
#endif
}


void MedianXLOfflineTools::syncWindowsTaskbarRecentFiles()
{
#ifdef WIN_7_OR_LATER
    IApplicationDocumentLists *pADL;
    HRESULT hr = ::CoCreateInstance(CLSID_ApplicationDocumentLists, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pADL));
    if (SUCCEEDED(hr))
    {
        if (SUCCEEDED(hr = pADL->SetAppID(appUserModelID())))
        {
            IObjectArray *pRecentItemsArray;
            if (SUCCEEDED(hr = pADL->GetList(ADLT_RECENT, kMaxRecentFiles, IID_PPV_ARGS(&pRecentItemsArray))))
            {
                UINT n;
                if (SUCCEEDED(hr = pRecentItemsArray->GetCount(&n)))
                {
                    QStringList recentFilesForTaskbar(_recentFilesList);
                    for (UINT i = 0; i < n; ++i)
                    {
                        IShellItem *pShellItem;
                        if (SUCCEEDED(hr = pRecentItemsArray->GetAt(i, IID_PPV_ARGS(&pShellItem))))
                        {
                            LPWSTR path = NULL;
                            if (SUCCEEDED(hr = pShellItem->GetDisplayName(SIGDN_FILESYSPATH, &path)))
                                recentFilesForTaskbar.removeOne(LPWSTR_TO_QSTRING(path));

                            ::CoTaskMemFree(path);
                        }
                    }

                    foreach (const QString &recentFile, recentFilesForTaskbar)
                        addToWindowsRecentFiles(recentFile);
                }
            }
        }

        pADL->Release();
    }
#endif
}

void MedianXLOfflineTools::removeFromWindowsRecentFiles(const QString &filePath)
{
#ifdef WIN_7_OR_LATER
    IApplicationDocumentLists *pADL;
    HRESULT hr = ::CoCreateInstance(CLSID_ApplicationDocumentLists, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pADL));
    if (SUCCEEDED(hr))
    {
        if (SUCCEEDED(hr = pADL->SetAppID(appUserModelID())))
        {
            IObjectArray *pRecentItemsArray;
            if (SUCCEEDED(hr = pADL->GetList(ADLT_RECENT, kMaxRecentFiles, IID_PPV_ARGS(&pRecentItemsArray))))
            {
                UINT n;
                if (SUCCEEDED(hr = pRecentItemsArray->GetCount(&n)))
                {
                    for (UINT i = 0; i < n; ++i)
                    {
                        IShellItem *pShellItem;
                        if (SUCCEEDED(hr = pRecentItemsArray->GetAt(i, IID_PPV_ARGS(&pShellItem))))
                        {
                            LPWSTR path = NULL;
                            if (SUCCEEDED(hr = pShellItem->GetDisplayName(SIGDN_FILESYSPATH, &path)))
                            {
                                if (!wcscmp(path, QSTRING_TO_LPCWSTR(QDir::toNativeSeparators(filePath))))
                                {
                                    IApplicationDestinations *pAD;
                                    if (SUCCEEDED(hr = CoCreateInstance(CLSID_ApplicationDestinations, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pAD))))
                                    {
                                        if (SUCCEEDED(hr = pAD->SetAppID(appUserModelID())))
                                            pAD->RemoveDestination(pShellItem);

                                        pAD->Release();
                                    }

                                    break;
                                }
                            }

                            ::CoTaskMemFree(path);
                        }
                    }
                }
            }
        }

        pADL->Release();
    }
#else
    Q_UNUSED(filePath);
#endif
}

void MedianXLOfflineTools::addToWindowsRecentFiles(const QString &filePath)
{
    QString nativeFilePath = QDir::toNativeSeparators(filePath);
#ifdef WIN_7_OR_LATER
    PSHCIFPN pSHCreateItemFromParsingName = (PSHCIFPN)GetProcAddress(SHELL32_HANDLE, "SHCreateItemFromParsingName");
    if (pSHCreateItemFromParsingName)
    {
        IShellItem *pShellItem;
        if (SUCCEEDED(pSHCreateItemFromParsingName(QSTRING_TO_LPCWSTR(nativeFilePath), NULL, IID_PPV_ARGS(&pShellItem))))
        {
            SHARDAPPIDINFO info;
            info.psi = pShellItem;
            info.pszAppID = appUserModelID();
            ::SHAddToRecentDocs(SHARD_APPIDINFO, &info);
        }
    }
    else
#endif
    {
        // just add to recent files on systems before Windows 7
        ::SHAddToRecentDocs(SHARD_PATHW, QSTRING_TO_LPCWSTR(nativeFilePath));
    }
}
