#include "application.h"
#include "languagemanager.hpp"

#include <QSettings>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    Application app(argc, argv);
    if (app.isRunning())
        return 0;

    // this dirty hack makes translations work - they can't be applied in QApplication's constructor
    while (!app.shouldAllowShowMainWindow());

    LanguageManager &langManager = LanguageManager::instance();
    langManager.currentLocale = QSettings().value(langManager.languageKey, QLocale::system().name().left(2)).toString();
    langManager.setResourcesPath(app.applicationDirPath() +
#ifdef Q_WS_MACX
        "/.."
#endif
        "/Resources");

    QTranslator myappTranslator;
    if (!myappTranslator.load(app.applicationName().remove(' ').toLower() + "_" + langManager.currentLocale, langManager.translationsPath))
        langManager.currentLocale = langManager.defaultLocale;
    app.installTranslator(&myappTranslator);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + langManager.currentLocale, langManager.translationsPath);
    app.installTranslator(&qtTranslator);

    app.createAndShowMainWindow();
    return app.exec();
}
