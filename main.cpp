#include "medianxlofflinetools.h"
#include "languagemanager.hpp"

#include <QApplication>

#include <QSettings>
#include <QTranslator>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setOrganizationName("kambala");
    app.setApplicationName("Median XL Offline Tools");
    app.setApplicationVersion("0.2");

#ifdef Q_WS_MACX
    app.setAttribute(Qt::AA_DontShowIconsInMenus);
#endif


    LanguageManager &langManager = LanguageManager::instance();
    langManager.currentLocale = QSettings().value(langManager.languageKey, QLocale::system().name().left(2)).toString();
    langManager.setResourcesPath(
#ifdef Q_WS_MACX
    app.applicationDirPath() + "/../"
#endif
    "Resources");

    QTranslator myappTranslator;
    if (!myappTranslator.load(app.applicationName().remove(' ').toLower() + "_" + langManager.currentLocale, langManager.translationsPath))
        langManager.currentLocale = langManager.defaultLocale;
    app.installTranslator(&myappTranslator);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + langManager.currentLocale, langManager.translationsPath);
    app.installTranslator(&qtTranslator);


    MedianXLOfflineTools w;
    w.show();

    return app.exec();
}
