#include "application.h"
#include "medianxlofflinetools.h"
#include "languagemanager.hpp"

#include <QSettings>
#include <QTranslator>

#ifdef Q_WS_WIN32
#include <QTextCodec>
#endif

#ifdef Q_WS_MACX
#include <QTimer>
#endif

static const QString appName("Median XL Offline Tools");


Application::Application(int &argc, char **argv) : QtSingleApplication(appName, argc, argv), _mainWindow(0)
{
#ifdef Q_WS_WIN32
    ::AllowSetForegroundWindow(ASFW_ANY);
#endif

    if (argc > 1)
    {
#ifdef Q_WS_WIN32
        _param = QTextCodec::codecForName("cp1251")->toUnicode(argv[1]);
#else
        _param = argv[1];
#endif
    }
    if (sendMessage(_param))
        return;

    setOrganizationName("kambala");
    setApplicationName(appName);
    setApplicationVersion("0.2.1");

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
    if (_mainWindow)
        delete _mainWindow;
}


void Application::createAndShowMainWindow()
{
    _mainWindow = new MedianXLOfflineTools(_param);
    _mainWindow->show();

    setActivationWindow(_mainWindow);
    connect(this, SIGNAL(messageReceived(const QString &)), SLOT(setParam(const QString &)));

#ifdef Q_WS_MACX
    disableLionWindowRestoration();

    if (_showWindowMacTimer)
        delete _showWindowMacTimer;
#endif
}

void Application::activateWindow()
{
    QtSingleApplication::activateWindow();
    _mainWindow->loadFile(_param);
}
