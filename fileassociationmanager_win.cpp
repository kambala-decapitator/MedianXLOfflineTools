#include "fileassociationmanager.h"
#include "windowsincludes.h"

#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QCoreApplication>


// helpers

QString extensionWithDotFromExtension(const QString &extension)
{
    return extension.startsWith('.') ? extension : '.' + extension;
}

QString executablePath()
{
    return QDir::toNativeSeparators(qApp->applicationFilePath());
}

QString openWithFormat()
{
    return "%1/shell/open/command/.";
}

QString executablePathWithParam()
{
    return QString("\"%1\" \"%2\"").arg(executablePath(), "%1"); // "path\to\exe" "%1"
}


// editing registry

void registerProgID(const QString &extensionWithDot)
{
    QString progId = FileAssociationManager::progIdForExtension(extensionWithDot);

    QSettings hklmSoftwareClasses("HKEY_LOCAL_MACHINE\\Software\\Classes", QSettings::NativeFormat);
    hklmSoftwareClasses.setValue(QString("%1/.").arg(extensionWithDot), progId);
    hklmSoftwareClasses.setValue(QString("%1/.").arg(progId), "Diablo 2 character save file");
    hklmSoftwareClasses.setValue(openWithFormat().arg(progId), executablePathWithParam());
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
    hklmSoftware.setValue(openWithFormat().arg(registryApplications), executablePathWithParam());
    hklmSoftware.setValue(QString("%1/SupportedTypes/%2").arg(registryApplications, extensionWithDot), QString("")); // empty string is intended
    hklmSoftware.endGroup();

#ifdef WIN_VISTA_OR_LATER
    if (QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA)
    {
        QString capabilitiesPath = QString("kambala/%1/Capabilities").arg(qApp->applicationName());

        hklmSoftware.beginGroup(capabilitiesPath);
        hklmSoftware.setValue("ApplicationName", qApp->applicationName());
        hklmSoftware.setValue("ApplicationDescription", "Offline analogue of grig's Median XL Online Tools");
        hklmSoftware.setValue("FileAssociations/" + extensionWithDot, FileAssociationManager::progIdForExtension(extensionWithDot));
        hklmSoftware.endGroup();

        hklmSoftware.setValue("RegisteredApplications/" + qApp->applicationName(), "Software\\" + QDir::toNativeSeparators(capabilitiesPath));
    }
#endif
}


// FileAssociationManager implementation

bool FileAssociationManager::isApplicationDefaultForExtension(const QString &extension)
{
    bool isDefault;
    QString extensionWithDot = extensionWithDotFromExtension(extension);
    if (QSysInfo::windowsVersion() < QSysInfo::WV_VISTA)
    {
        QSettings hklmSoftwareClasses("HKEY_LOCAL_MACHINE\\Software\\Classes", QSettings::NativeFormat);
        isDefault = hklmSoftwareClasses.value(openWithFormat().arg(FileAssociationManager::progIdForExtension(extensionWithDot))).toString() == executablePathWithParam();
        if (isDefault)
        {
            QSettings hkcuFileExts("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\" + extensionWithDot, QSettings::NativeFormat);
            if (hkcuFileExts.contains("Application") && hkcuFileExts.value("Application").toString() != QFileInfo(executablePath()).fileName())
                isDefault = false;
        }
    }
#ifdef WIN_VISTA_OR_LATER
    else
    {
        isDefault = true; // don't try to register in case of error

        IApplicationAssociationRegistration *pAAR;
        HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistration, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pAAR));
        if (SUCCEEDED(hr))
        {
            BOOL isDefaultBOOL;
            hr = pAAR->QueryAppIsDefault(extensionWithDot.utf16(), AT_FILEEXTENSION, AL_EFFECTIVE, qApp->applicationName().utf16(), &isDefaultBOOL);
            if (HRESULT_CODE(hr) == ERROR_FILE_NOT_FOUND)
            {
                qDebug("app is not in the registry");
                hr = S_OK;
            }
            else if (FAILED(hr))
                qDebug("Error calling QueryAppIsDefault(): %d", HRESULT_CODE(hr));

            if (SUCCEEDED(hr))
                isDefault = static_cast<bool>(isDefaultBOOL);

            pAAR->Release();
        }
        else
            qDebug("Error calling CoCreateInstance(CLSID_ApplicationAssociationRegistration): %d", HRESULT_CODE(hr));
    }
#endif
    return isDefault;
}

void FileAssociationManager::makeApplicationDefaultForExtension(const QString &extension)
{
    QString extensionWithDot = extensionWithDotFromExtension(extension);
    registerProgID(extensionWithDot);
    registerApplication(extensionWithDot);
    ::SHChangeNotify(SHCNE_ASSOCCHANGED, 0, NULL, NULL);

#ifdef WIN_VISTA_OR_LATER
    if (QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA)
    {
        IApplicationAssociationRegistration *pAAR;
        HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistration, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pAAR));
        if (SUCCEEDED(hr))
        {
            hr = pAAR->SetAppAsDefault(qApp->applicationName().utf16(), extensionWithDot.utf16(), AT_FILEEXTENSION);
            if (SUCCEEDED(hr))
                qDebug("app is default now");
            else
                qDebug("SetAppAsDefault() failed with result: %d", HRESULT_CODE(hr));

            pAAR->Release();
        }
        else
            qDebug("Error calling CoCreateInstance(CLSID_ApplicationAssociationRegistration): %d", HRESULT_CODE(hr));
    }
#endif
}

QString FileAssociationManager::progIdForExtension(const QString &extensionWithDot)
{
    return qApp->applicationName().remove(' ') + extensionWithDot;
}
