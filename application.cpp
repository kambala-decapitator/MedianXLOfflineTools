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


Application::Application(int &argc, char **argv) : QtSingleApplication(appName, argc, argv), _mainWindow(0), _shouldAllowShowMainWindow(false)
{
#ifdef Q_WS_WIN32
    ::AllowSetForegroundWindow(ASFW_ANY);
#endif

    if (argc > 1)
    {
#ifdef Q_WS_WIN32
        _param = QTextCodec::codecForLocale()->toUnicode(argv[1]); // stupid Windows
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

    if (_param.isEmpty())
    {
        _showWindowMacTimer = new QTimer;
        _showWindowMacTimer->setSingleShot(true);
        connect(_showWindowMacTimer, SIGNAL(timeout()), SLOT(allowShowMainWindow()));
        _showWindowMacTimer->start(0);
    }
    else
#endif
        allowShowMainWindow();
}

Application::~Application()
{
    if (_mainWindow)
        delete _mainWindow;
}


void Application::allowShowMainWindow()
{
    _shouldAllowShowMainWindow = true;

#ifdef Q_WS_MACX
    if (_showWindowMacTimer)
        delete _showWindowMacTimer;
#endif
}

void Application::createAndShowMainWindow()
{
    _mainWindow = new MedianXLOfflineTools(_param);
    _mainWindow->show();

    setActivationWindow(_mainWindow);
    connect(this, SIGNAL(messageReceived(const QString &)), SLOT(setParam(const QString &)));

#ifdef Q_WS_MACX
    disableLionWindowRestoration();
#endif
}

void Application::activateWindow()
{
    QtSingleApplication::activateWindow();
    _mainWindow->loadFile(_param);
}
