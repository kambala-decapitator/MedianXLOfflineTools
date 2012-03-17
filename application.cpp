#include "application.h"
#include "medianxlofflinetools.h"
#include "languagemanager.hpp"

#include <QSettings>
#include <QTranslator>

#ifdef Q_WS_MACX
#include <QFileOpenEvent>
#endif


Application::Application(int &argc, char **argv) : QApplication(argc, argv)
{
    setOrganizationName("kambala");
    setApplicationName("Median XL Offline Tools");
    setApplicationVersion("0.3");
#ifdef Q_WS_MACX
    setAttribute(Qt::AA_DontShowIconsInMenus);
#endif

    LanguageManager &langManager = LanguageManager::instance();
    langManager.currentLocale = QSettings().value(langManager.languageKey, QLocale::system().name().left(2)).toString();
    langManager.setResourcesPath(applicationDirPath() +
#ifdef Q_WS_MACX
    "/.." +
#endif
    "/Resources");

    QTranslator myappTranslator;
    if (!myappTranslator.load(applicationName().remove(' ').toLower() + "_" + langManager.currentLocale, langManager.translationsPath))
        langManager.currentLocale = langManager.defaultLocale;
    installTranslator(&myappTranslator);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + langManager.currentLocale, langManager.translationsPath);
    installTranslator(&qtTranslator);

//    if (argc > 0)
//    {
//        QString params;
//        for (int i = 1; i < argc; ++i)
//            params += QString("%1 %2\n").arg(i).arg(argv[i]);
//        qDebug("params: %s", qPrintable(params));
//        QMessageBox::information(0, "params", params);
//    }

    _mainWindow = new MedianXLOfflineTools/*(argc > 1 ? argv[1] : QString())*/;
    _mainWindow->show();
}

Application::~Application()
{
    delete _mainWindow;
}

#ifdef Q_WS_MACX
bool Application::event(QEvent *ev)
{
    if (ev->type() == QEvent::FileOpen)
    {
        _mainWindow->loadFile(static_cast<QFileOpenEvent *>(ev)->file());
        return true;
    }
    return QApplication::event(ev);
}
#endif
