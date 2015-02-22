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

        DWORD dwType;
        PGPI pGPI = (PGPI)GetProcAddress(kernel32Handle, "GetProductInfo");
        pGPI(osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);
        if (dwType != PRODUCT_CORE)
            os << " ";
        switch(dwType)
        {
        case PRODUCT_ULTIMATE:
            os << "Ultimate";
            break;
        case PRODUCT_ULTIMATE_N:
            os << "Ultimate N";
            break;
        case PRODUCT_ULTIMATE_E:
            os << "Ultimate E";
            break;
        case PRODUCT_PROFESSIONAL:
            os << "Professional";
            break;
        case PRODUCT_PROFESSIONAL_N:
            os << "Professional N";
            break;
        case PRODUCT_HOME_PREMIUM:
            os << "Home Premium";
            break;
        case PRODUCT_HOME_PREMIUM_N:
            os << "Home Premium N";
            break;
        case PRODUCT_HOME_BASIC:
            os << "Home Basic";
            break;
        case PRODUCT_ENTERPRISE:
            os << "Enterprise";
            break;
        case PRODUCT_BUSINESS:
            os << "Business";
            break;
        case PRODUCT_STARTER:
            os << "Starter";
            break;
        case PRODUCT_STARTER_N:
            os << "Starter N";
            break;
        case PRODUCT_CLUSTER_SERVER:
            os << "Cluster Server";
            break;
        case PRODUCT_DATACENTER_SERVER: case PRODUCT_DATACENTER_SERVER_CORE: case PRODUCT_DATACENTER_EVALUATION_SERVER: case PRODUCT_DATACENTER_SERVER_CORE_V: case PRODUCT_DATACENTER_SERVER_V:
            os << "Datacenter";
            break;
        case PRODUCT_ENTERPRISE_SERVER: case PRODUCT_ENTERPRISE_SERVER_CORE: case PRODUCT_ENTERPRISE_SERVER_IA64: case PRODUCT_ENTERPRISE_EVALUATION: case PRODUCT_ENTERPRISE_SERVER_CORE_V: case PRODUCT_ENTERPRISE_SERVER_V:
            os << "Enterprise";
            break;
        case PRODUCT_ENTERPRISE_N: case PRODUCT_ENTERPRISE_N_EVALUATION:
            os << "Enterprise N";
            break;
        case PRODUCT_SMALLBUSINESS_SERVER:
            os << "Small Business Server";
            break;
        case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
            os << "Small Business Server Premium";
            break;
        case PRODUCT_STANDARD_SERVER: case PRODUCT_STANDARD_SERVER_CORE:
            os << "Standard";
            break;
        case PRODUCT_WEB_SERVER:
            os << "Web Server";
            break;
        case PRODUCT_PROFESSIONAL_WMC:
            os << "Professional with Media Center";
            break;
        case PRODUCT_CORE: // simple Windows 8, don't add anything
            break;
        case PRODUCT_CORE_N: // Windows 8 N
            os << "N";
            break;
        case PRODUCT_CORE_COUNTRYSPECIFIC:
            os << "China";
            break;
        case PRODUCT_CORE_SINGLELANGUAGE:
            os << "Single Language";
            break;
        case PRODUCT_EMBEDDED: case PRODUCT_EMBEDDED_A: case PRODUCT_EMBEDDED_E: case PRODUCT_EMBEDDED_AUTOMOTIVE: case PRODUCT_EMBEDDED_INDUSTRY: case PRODUCT_EMBEDDED_INDUSTRY_A: case PRODUCT_EMBEDDED_INDUSTRY_E: case PRODUCT_EMBEDDED_INDUSTRY_A_E:
            os << "Embedded";
            break;
        case PRODUCT_HYPERV:
            os << "Microsoft Hyper-V Server";
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
                os << "Windows XP Professional x64";
            else
                os << "Windows Server 2003, ";

            // Test for the server type.
            if (osvi.wProductType != VER_NT_WORKSTATION)
            {
                switch (si.wProcessorArchitecture)
                {
                case PROCESSOR_ARCHITECTURE_IA64:
                    if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
                        os << "Datacenter for Itanium-based Systems";
                    else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
                        os << "Enterprise for Itanium-based Systems";
                    break;
                case PROCESSOR_ARCHITECTURE_AMD64:
                    if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
                        os << "Datacenter x64";
                    else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
                        os << "Enterprise x64";
                    else
                        os << "Standard x64";
                    break;
                default:
                    if (osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER)
                        os << "Compute Cluster";
                    else if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
                        os << "Datacenter";
                    else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
                        os << "Enterprise";
                    else if (osvi.wSuiteMask & VER_SUITE_BLADE)
                        os << "Web";
                    else
                        os << "Standard";
                    break;
                }
            }
            break;
        case 1:
            os << "Windows XP " << (osvi.wSuiteMask & VER_SUITE_PERSONAL ? "Home" : "Professional");
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
