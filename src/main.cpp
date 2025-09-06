#include "application.h"
#include "languagemanager.hpp"

#include <QSettings>
#include <QLocale>
#include <QTranslator>

static QString dataPath(const QApplication &app)
{
#ifdef DATA_PATH
    return DATA_PATH;
#else
# ifdef Q_OS_MAC
    const QLatin1String resourcesDir("../Resources");
# else
    const QLatin1String resourcesDir("resources");
# endif
    return QString("%1/%2").arg(app.applicationDirPath(), resourcesDir);
#endif
}

int main(int argc, char *argv[])
{
    Application app(argc, argv);
#if HAS_QTSINGLEAPPLICATION
    if (app.isRunning())
        return 0;
#endif

    LanguageManager &langManager = LanguageManager::instance();
    langManager.currentLocale = QSettings().value(langManager.languageKey, QLocale::system().name().left(2)).toString();
    langManager.setResourcesPath(dataPath(app));

    QTranslator myappTranslator;
    if (!myappTranslator.load(app.applicationName().remove(' ').toLower() + "_" + langManager.currentLocale, langManager.translationsPath))
        langManager.currentLocale = langManager.defaultLocale;
    app.installTranslator(&myappTranslator);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + langManager.currentLocale, langManager.translationsPath);
    app.installTranslator(&qtTranslator);

    app.init();
    return app.exec();
}
