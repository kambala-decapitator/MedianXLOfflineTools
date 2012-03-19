#include "application.h"
#include "medianxlofflinetools.h"
#include "languagemanager.hpp"

#include <QSettings>
#include <QTranslator>

#ifdef Q_WS_MACX
#include <QFileOpenEvent>
#include <QTimer>
#endif


Application::Application(int &argc, char **argv) : QApplication(argc, argv), _mainWindow(0)
{
    setOrganizationName("kambala");
    setApplicationName("Median XL Offline Tools");
    setApplicationVersion("0.3");
#ifdef Q_WS_MACX
    setAttribute(Qt::AA_DontShowIconsInMenus);

    _showWindowMacTimer = 0;
#endif

    LanguageManager &langManager = LanguageManager::instance();
    langManager.currentLocale = QSettings().value(langManager.languageKey, QLocale::system().name().left(2)).toString();
    langManager.setResourcesPath(applicationDirPath() +
#ifdef Q_WS_MACX
    "/.."
#endif
    "/Resources");

    QTranslator myappTranslator;
    if (!myappTranslator.load(applicationName().remove(' ').toLower() + "_" + langManager.currentLocale, langManager.translationsPath))
        langManager.currentLocale = langManager.defaultLocale;
    installTranslator(&myappTranslator);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + langManager.currentLocale, langManager.translationsPath);
    installTranslator(&qtTranslator);

    if (argc > 1)
        _param = argv[1];
#ifdef Q_WS_MACX
    if (_param.isEmpty())
    {
        _showWindowMacTimer = new QTimer;
        _showWindowMacTimer->setSingleShot(true);
        connect(_showWindowMacTimer, SIGNAL(timeout()), SLOT(createAndShowMainWindow()));
        _showWindowMacTimer->start(0);
    }
    else
#endif
        createAndShowMainWindow();
}

Application::~Application()
{
    delete _mainWindow;
}


void Application::createAndShowMainWindow()
{
    _mainWindow = new MedianXLOfflineTools(_param);
#ifdef Q_WS_MACX
    disableLionWindowRestoration();
#endif
    _mainWindow->show();

#ifdef Q_WS_MACX
    if (_showWindowMacTimer)
        delete _showWindowMacTimer;
#endif
}


#ifdef Q_WS_MACX
bool Application::event(QEvent *ev)
{
    if (ev->type() == QEvent::FileOpen)
    {
        _param = static_cast<QFileOpenEvent *>(ev)->file();
        if (!_mainWindow)
        {
            _showWindowMacTimer->stop();
            delete _showWindowMacTimer;
            _showWindowMacTimer = 0;

            createAndShowMainWindow();
        }
        else
            _mainWindow->loadFile(_param);
        return true;
    }
    return QApplication::event(ev);
}
#endif
