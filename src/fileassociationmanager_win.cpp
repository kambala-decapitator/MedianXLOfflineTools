#include "fileassociationmanager.h"
#include "windowsincludes.h"
#include "helpers.h"

#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QCoreApplication>

#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
#define HAS_QOPERATINGSYSTEMVERSION 1
#include <QOperatingSystemVersion>
#else
#include <QSysInfo>
#endif


// helpers

static const QString kOpenWithFormat("%1/shell/open/command/.");

QString extensionWithDotFromExtension(const QString &extension)
{
    return extension.startsWith('.') ? extension : '.' + extension;
}

QString executablePath()
{
    return QDir::toNativeSeparators(qApp->applicationFilePath());
}

QString executablePathWithParam()
{
    return QString("\"%1\" \"%2\"").arg(executablePath(), "%1"); // "path\to\exe" "%1"
}

static bool atLeastVista()
{
#if HAS_QOPERATINGSYSTEMVERSION
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion(QOperatingSystemVersion::Windows, 6, 0);
#else
    return QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA;
#endif
}


// editing registry

void registerProgID(const QString &extensionWithDot)
{
    QString progId = FileAssociationManager::progIdForExtension(extensionWithDot);

    QSettings hklmSoftwareClasses("HKEY_LOCAL_MACHINE\\Software\\Classes", QSettings::NativeFormat);
    hklmSoftwareClasses.setValue(QString("%1/.").arg(extensionWithDot), progId);
    hklmSoftwareClasses.setValue(QString("%1/.").arg(progId), "Diablo 2 character save file");
    hklmSoftwareClasses.setValue(kOpenWithFormat.arg(progId), executablePathWithParam());
}

void registerApplication(const QString &extensionWithDot)
{
    QString executableName = QFileInfo(executablePath()).fileName();

    QSettings hklmSoftware("HKEY_LOCAL_MACHINE\\Software", QSettings::NativeFormat);
    hklmSoftware.beginGroup("Microsoft/Windows/CurrentVersion/App Paths");
    hklmSoftware.setValue(QString("%1/.").arg(executableName), executablePath());
    hklmSoftware.setValue(QString("%1/Path").arg(executableName), QDir::toNativeSeparators(qApp->applicationDirPath()));
    hklmSoftware.endGroup();

    QString registryApplications = QString("Applications/%1").arg(executableName);
    hklmSoftware.beginGroup("Classes");
    hklmSoftware.setValue(kOpenWithFormat.arg(registryApplications), executablePathWithParam());
    hklmSoftware.setValue(QString("%1/SupportedTypes/%2").arg(registryApplications, extensionWithDot), QString("")); // empty string is intended
    hklmSoftware.endGroup();

    if (atLeastVista())
    {
        QString capabilitiesPath = QString("%1/%2/Capabilities").arg(qApp->organizationName(), qApp->applicationName());

        hklmSoftware.beginGroup(capabilitiesPath);
        hklmSoftware.setValue("ApplicationName", qApp->applicationName());
        hklmSoftware.setValue("ApplicationDescription", "Offline analogue of grig's Median XL Online Tools");
        hklmSoftware.setValue("FileAssociations/" + extensionWithDot, FileAssociationManager::progIdForExtension(extensionWithDot));
        hklmSoftware.endGroup();

        hklmSoftware.setValue("RegisteredApplications/" + qApp->applicationName(), "Software\\" + QDir::toNativeSeparators(capabilitiesPath));
    }
}

bool isRegisteredApplicationOverridenInFileExts(const QString &extensionWithDot, bool removeIfExists)
{
    QSettings hkcuFileExts("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\" + extensionWithDot, QSettings::NativeFormat);
    bool result = hkcuFileExts.contains("Application") && hkcuFileExts.value("Application").toString() != QFileInfo(executablePath()).fileName();
    if (result && removeIfExists)
        hkcuFileExts.remove("Application");
    return result;
}


// FileAssociationManager implementation

bool FileAssociationManager::isApplicationDefaultForExtension(const QString &extension)
{
    bool isDefault = true; // don't try to register if something goes wrong
    QString extensionWithDot = extensionWithDotFromExtension(extension);
    if (!atLeastVista())
    {
        QSettings hklmSoftwareClasses("HKEY_LOCAL_MACHINE\\Software\\Classes", QSettings::NativeFormat);
        isDefault = hklmSoftwareClasses.value(kOpenWithFormat.arg(FileAssociationManager::progIdForExtension(extensionWithDot))).toString() == executablePathWithParam();
        if (isDefault)
        {
            if (isRegisteredApplicationOverridenInFileExts(extensionWithDot, false))
                isDefault = false;
        }
    }
#ifdef WIN_VISTA_OR_LATER
    else
    {
        IApplicationAssociationRegistration *pAAR;
        HRESULT hr = ::CoCreateInstance(CLSID_ApplicationAssociationRegistration, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pAAR));
        if (SUCCEEDED(hr))
        {
            BOOL isDefaultBOOL;
            hr = pAAR->QueryAppIsDefault(QSTRING_TO_LPCWSTR(extensionWithDot), AT_FILEEXTENSION, AL_EFFECTIVE, QSTRING_TO_LPCWSTR(qApp->applicationName()), &isDefaultBOOL);
            if (HRESULT_CODE(hr) == ERROR_FILE_NOT_FOUND)
            {
                qDebug("app is not in the registry");
                hr = S_OK;
            }
            else if (FAILED(hr))
                ERROR_BOX_NO_PARENT(QString("Error calling QueryAppIsDefault(): %1").arg(HRESULT_CODE(hr)));

            if (SUCCEEDED(hr))
                isDefault = static_cast<bool>(isDefaultBOOL);

            pAAR->Release();
        }
        else
            ERROR_BOX_NO_PARENT(QString("Error calling CoCreateInstance(CLSID_ApplicationAssociationRegistration): %1").arg(HRESULT_CODE(hr)));
    }
#endif
    return isDefault;
}

void FileAssociationManager::makeApplicationDefaultForExtension(const QString &extension)
{
    QString extensionWithDot = extensionWithDotFromExtension(extension);
    registerApplicationForExtension(extensionWithDot);

    bool hasAssociationChanged = true;
    if (!atLeastVista())
        isRegisteredApplicationOverridenInFileExts(extensionWithDot, true);
#ifdef WIN_VISTA_OR_LATER
    else
    {
        IApplicationAssociationRegistration *pAAR;
        HRESULT hr = ::CoCreateInstance(CLSID_ApplicationAssociationRegistration, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pAAR));
        if ((hasAssociationChanged = SUCCEEDED(hr)))
        {
            hr = pAAR->SetAppAsDefault(QSTRING_TO_LPCWSTR(qApp->applicationName()), QSTRING_TO_LPCWSTR(extensionWithDot), AT_FILEEXTENSION);
            if ((hasAssociationChanged = SUCCEEDED(hr)))
                qDebug("app is default now");
            else
                ERROR_BOX_NO_PARENT(QString("Error calling SetAppAsDefault(): %1").arg(HRESULT_CODE(hr)));

            pAAR->Release();
        }
        else
            ERROR_BOX_NO_PARENT(QString("Error calling CoCreateInstance(CLSID_ApplicationAssociationRegistration): %1").arg(HRESULT_CODE(hr)));
    }
#endif

    if (hasAssociationChanged)
        ::SHChangeNotify(SHCNE_ASSOCCHANGED, 0, NULL, NULL);
}

QString FileAssociationManager::progIdForExtension(const QString &extension)
{
    return qApp->applicationName().remove(' ') + extensionWithDotFromExtension(extension);
}

void FileAssociationManager::registerApplicationForExtension(const QString &extension)
{
    QString extensionWithDot = extensionWithDotFromExtension(extension);
    registerProgID(extensionWithDot);
    registerApplication(extensionWithDot);
}
