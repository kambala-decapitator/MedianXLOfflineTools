#include "medianxlofflinetools.h"

#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QSettings>

#include <Shlobj.h>
#include <Shobjidl.h>


bool isDefaultBeforeVista()
{
    return true;
}

void MedianXLOfflineTools::checkFileAssociations()
{
    QString appPath = QDir::toNativeSeparators(qApp->applicationFilePath()), binaryName = QFileInfo(appPath).fileName(), defaultValueFormat("%1/."), cmdOpenFileFormat("%1/shell/open/command/.");
    QString registryAppPath = QString("Applications/%1").arg(binaryName), binaryPathWithParam = QString("\"%1\" \"%2\"").arg(appPath, "%1"); // "path/to/exe" "%1"
    QString appName = qApp->applicationName(), progId = QString("%1.%2").arg(QString(appName).remove(' '), characterExtension), defaultApplicationRegistryPath = cmdOpenFileFormat.arg(progId);

    QSettings hklmSoftware("HKEY_LOCAL_MACHINE\\Software", QSettings::NativeFormat);
    if (QSysInfo::windowsVersion() < QSysInfo::WV_VISTA)
    {
        // check key "Application" in HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.d2s\ (if present, contains some binary name)
        hklmSoftware.beginGroup("Classes");
        if (!hklmSoftware.value(defaultApplicationRegistryPath).toString().startsWith(appPath))
        {
            // TODO: same code in Vista+ section
            hklmSoftware.setValue(defaultValueFormat.arg(characterExtensionWithDot), progId);
            hklmSoftware.setValue(defaultValueFormat.arg(progId), "Diablo 2 character save file");
            hklmSoftware.setValue(defaultApplicationRegistryPath, binaryPathWithParam);

            hklmSoftware.setValue(cmdOpenFileFormat.arg(registryAppPath), binaryPathWithParam);
            hklmSoftware.setValue(QString("%1/SupportedTypes/%2").arg(registryAppPath, characterExtensionWithDot), QString(""));
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
    else
    {
        IApplicationAssociationRegistration *pAAR;
        HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistration, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pAAR));
        if (SUCCEEDED(hr))
        {
            QStdWString appNameStdWstr = appName.toStdWString(), extensionWithDotStdWstr = characterExtensionWithDot.toStdWString();
            LPCWSTR appNameWstr = appNameStdWstr.c_str(), extensionWithDotWstr = extensionWithDotStdWstr.c_str();

            BOOL isDefault;
            hr = pAAR->QueryAppIsDefault(extensionWithDotWstr, AT_FILEEXTENSION, AL_EFFECTIVE, appNameWstr, &isDefault); // returns HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) if app doesn't exist in registry
            if (HRESULT_CODE(hr) == ERROR_FILE_NOT_FOUND)
            {
                qDebug("app is not in the registry, adding it");

                // add info about our app to the registry

                // TODO: it's ugly copypaste from above
                hklmSoftware.beginGroup("Classes");
                hklmSoftware.setValue(defaultValueFormat.arg(characterExtensionWithDot), progId);

                hklmSoftware.setValue(defaultValueFormat.arg(progId), "Diablo 2 character save file");
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
                hklmSoftware.setValue("FileAssociations/" + characterExtensionWithDot, progId);
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
                LPWSTR defaultAppNameWstr;
                hr = pAAR->QueryCurrentDefault(extensionWithDotWstr, AT_FILEEXTENSION, AL_EFFECTIVE, &defaultAppNameWstr); // returns HRESULT_FROM_WIN32(ERROR_NO_ASSOCIATION) if no app is associated
                if (SUCCEEDED(hr))
                {
                    OutputDebugString(defaultAppNameWstr);
                    qDebug(" - default app");
                }
                else
                    qDebug("QueryCurrentDefault() failed with result: %ld", HRESULT_CODE(hr));

                hr = pAAR->SetAppAsDefault(appNameWstr, extensionWithDotWstr, AT_FILEEXTENSION); // may also return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) if extension isn't registered
                if (SUCCEEDED(hr))
                    qDebug("app is default now");
                else
                    qDebug("SetAppAsDefault() failed with result: %ld", HRESULT_CODE(hr));
            }

            pAAR->Release();
        }
        else
            ERROR_BOX(tr("Error calling CoCreateInstance(): %1").arg(HRESULT_CODE(hr)));
    }
}
