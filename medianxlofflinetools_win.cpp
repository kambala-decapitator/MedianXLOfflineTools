#include "medianxlofflinetools.h"
#include "fileassociationmanager.h"
#include "windowsincludes.h"

#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QSettings>

#include <sstream>

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

typedef HRESULT (__stdcall *PSCPEAUMID)(PCWSTR); // SetCurrentProcessExplicitAppUserModelID()
#ifdef WIN_7_OR_LATER
typedef HRESULT (__stdcall *PSHCIFPN)(PCWSTR, IBindCtx *, const IID &, void **); // SHCreateItemFromParsingName()
#endif

typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO); // GetNativeSystemInfo()
typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD); // GetProductInfo()


PCWSTR MedianXLOfflineTools::appUserModelID()
{
    static const QString progId = FileAssociationManager::progIdForExtension(kCharacterExtensionWithDot);
    return WINAPI_STRING_FROM_QSTRING(progId);
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

    IApplicationAssociationRegistrationUI *pAARUI;
    HRESULT hr = ::CoCreateInstance(CLSID_ApplicationAssociationRegistrationUI, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pAARUI));
    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = pAARUI->LaunchAdvancedAssociationUI(WINAPI_STRING_FROM_QSTRING(qApp->applicationName()))))
            ERROR_BOX(QString("Error calling LaunchAdvancedAssociationUI: %1").arg(HRESULT_CODE(hr)));
        pAARUI->Release();
    }
    else
        ERROR_BOX(QString("Error calling CoCreateInstance(CLSID_ApplicationAssociationRegistrationUI): %1").arg(HRESULT_CODE(hr)));
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
                                recentFilesForTaskbar.removeOne(QString::fromUtf16(reinterpret_cast<const ushort *>(path)));

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
                                if (!wcscmp(path, WINAPI_STRING_FROM_QSTRING(QDir::toNativeSeparators(filePath))))
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
        if (SUCCEEDED(pSHCreateItemFromParsingName(WINAPI_STRING_FROM_QSTRING(nativeFilePath), NULL, IID_PPV_ARGS(&pShellItem))))
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
        ::SHAddToRecentDocs(SHARD_PATHW, nativeFilePath.utf16());
    }
}


QByteArray MedianXLOfflineTools::getOsInfo()
{
    // credits: http://msdn.microsoft.com/en-us/library/windows/desktop/ms724429(v=vs.85).aspx (Getting the System Version)
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    BOOL bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *)&osvi);
    if (!bOsVersionInfoEx || VER_PLATFORM_WIN32_NT != osvi.dwPlatformId || osvi.dwMajorVersion < 5)
        return QByteArray();

    // Call GetNativeSystemInfo if supported or GetSystemInfo otherwise
    SYSTEM_INFO si;
    ZeroMemory(&si, sizeof(SYSTEM_INFO));
    HMODULE kernel32Handle = GetModuleHandle(L"kernel32.dll");
    PGNSI pGNSI = (PGNSI)GetProcAddress(kernel32Handle, "GetNativeSystemInfo");
    if (pGNSI)
        pGNSI(&si);
    else
        GetSystemInfo(&si);

    std::stringstream os;
    // Test for the specific product
    switch (osvi.dwMajorVersion)
    {
    case 6:
    {
        switch (osvi.dwMinorVersion)
        {
        case 2:
            os << (osvi.wProductType == VER_NT_WORKSTATION ? "Windows 8" : "Windows Server 2012");
            break;
        case 1:
            os << (osvi.wProductType == VER_NT_WORKSTATION ? "Windows 7" : "Windows Server 2008 R2");
            break;
        case 0:
            os << (osvi.wProductType == VER_NT_WORKSTATION ? "Windows Vista" : "Windows Server 2008");
            break;
        default:
            os << "Windows NEW: minor " << osvi.dwMinorVersion << ", product " << osvi.wProductType;
            break;
        }
        os << " ";

        DWORD dwType;
        PGPI pGPI = (PGPI)GetProcAddress(kernel32Handle, "GetProductInfo");
        pGPI(osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);
        switch(dwType)
        {
        case PRODUCT_ULTIMATE:
            os << "Ultimate Edition";
            break;
        case PRODUCT_PROFESSIONAL:
            os << "Professional";
            break;
        case PRODUCT_PROFESSIONAL_N:
            os << "Professional N";
            break;
        case PRODUCT_HOME_PREMIUM:
            os << "Home Premium Edition";
            break;
        case PRODUCT_HOME_BASIC:
            os << "Home Basic Edition";
            break;
        case PRODUCT_ENTERPRISE:
            os << "Enterprise Edition";
            break;
        case PRODUCT_BUSINESS:
            os << "Business Edition";
            break;
        case PRODUCT_STARTER:
            os << "Starter Edition";
            break;
        case PRODUCT_STARTER_N:
            os << "Starter N Edition";
            break;
        case PRODUCT_CLUSTER_SERVER:
            os << "Cluster Server Edition";
            break;
        case PRODUCT_DATACENTER_SERVER:
            os << "Datacenter Edition";
            break;
        case PRODUCT_DATACENTER_SERVER_CORE:
            os << "Datacenter Edition (core installation)";
            break;
        case PRODUCT_ENTERPRISE_SERVER:
            os << "Enterprise Edition";
            break;
        case PRODUCT_ENTERPRISE_SERVER_CORE:
            os << "Enterprise Edition (core installation)";
            break;
        case PRODUCT_ENTERPRISE_SERVER_IA64:
            os << "Enterprise Edition for Itanium-based Systems";
            break;
        case PRODUCT_SMALLBUSINESS_SERVER:
            os << "Small Business Server";
            break;
        case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
            os << "Small Business Server Premium Edition";
            break;
        case PRODUCT_STANDARD_SERVER:
            os << "Standard Edition";
            break;
        case PRODUCT_STANDARD_SERVER_CORE:
            os << "Standard Edition (core installation)";
            break;
        case PRODUCT_WEB_SERVER:
            os << "Web Server Edition";
            break;
        case PRODUCT_PROFESSIONAL_WMC:
            os << "Professional with Media Center";
            break;
        case PRODUCT_CORE_N: // Windows 8 N
            os << "N";
            break;
        case PRODUCT_CORE_COUNTRYSPECIFIC:
            os << "China";
            break;
        default:
            os << "dwType " << dwType;
            break;
        }
    }
        break;
    case 5:
        switch (osvi.dwMinorVersion)
        {
        case 2:
            if (GetSystemMetrics(SM_SERVERR2))
                os << "Windows Server 2003 R2, ";
            else if (osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER)
                os << "Windows Storage Server 2003";
            else if (osvi.wSuiteMask & VER_SUITE_WH_SERVER)
                os << "Windows Home Server";
            else if (osvi.wProductType == VER_NT_WORKSTATION && si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
                os << "Windows XP Professional x64 Edition";
            else
                os << "Windows Server 2003, ";

            // Test for the server type.
            if (osvi.wProductType != VER_NT_WORKSTATION)
            {
                switch (si.wProcessorArchitecture)
                {
                case PROCESSOR_ARCHITECTURE_IA64:
                    if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
                        os << "Datacenter Edition for Itanium-based Systems";
                    else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
                        os << "Enterprise Edition for Itanium-based Systems";
                    break;
                case PROCESSOR_ARCHITECTURE_AMD64:
                    if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
                        os << "Datacenter x64 Edition";
                    else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
                        os << "Enterprise x64 Edition";
                    else
                        os << "Standard x64 Edition";
                    break;
                default:
                    if (osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER)
                        os << "Compute Cluster Edition";
                    else if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
                        os << "Datacenter Edition";
                    else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
                        os << "Enterprise Edition";
                    else if (osvi.wSuiteMask & VER_SUITE_BLADE)
                        os << "Web Edition";
                    else
                        os << "Standard Edition";
                    break;
                }
            }
            break;
        case 1:
            os << "Windows XP " << (osvi.wSuiteMask & VER_SUITE_PERSONAL ? "Home Edition" : "Professional");
            break;
        case 0:
            os << "Windows 2000 ";
            if (osvi.wProductType == VER_NT_WORKSTATION)
                os << "Professional";
            else
            {
                if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
                    os << "Datacenter Server";
                else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
                    os << "Advanced Server";
                else
                    os << "Server";
            }
            break;
        }
        break;
    default:
        os << "Windows SUPERNEW: major " << osvi.dwMajorVersion << ", minor " << osvi.dwMinorVersion << ", product " << osvi.wProductType;
        break;
    }

    // Include service pack (if any) and build number
//    if (wcslen(osvi.szCSDVersion) > 0)
//        os << " " << osvi.szCSDVersion;
//    os << " (build " << osvi.dwBuildNumber << ")";

    if (osvi.dwMajorVersion >= 6)
    {
        if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
            os << ", 64-bit";
        else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
            os << ", 32-bit";
    }

    return os.str().c_str();
}
