#include "application.h"

#if defined(Q_OS_WIN32)
#include <QTextCodec>
#elif defined(Q_OS_MAC)
#include <QTimer>
#endif

static const QString kAppName("Median XL Offline Tools");


Application::Application(int &argc, char **argv) : QAPP_CLASS(argc, argv), _mainWindow(0), _launchMode(LaunchModeNormal)
{
#ifdef Q_OS_WIN32
    ::AllowSetForegroundWindow(ASFW_ANY);
#endif

    if (argc > 1)
    {
#ifdef DUPE_CHECK
        if (argc >= 3)
        {
            if (!strcmp(argv[1], "-dupeScan"))
                _launchMode = LaunchModeDupeScan;
            else if (!strcmp(argv[1], "-dumpItems"))
                _launchMode = LaunchModeDumpItems;
        }
        else
#endif
#ifdef Q_OS_WIN32
        _param = QTextCodec::codecForLocale()->toUnicode(argv[1]); // stupid Windows
#else
        _param = argv[1];
#endif
    }
#if HAS_QTSINGLEAPPLICATION
	if (sendMessage(_param))
		return;
#endif

    setOrganizationName("kambala");
    setApplicationName(kAppName);
    setApplicationVersion(NVER_STRING);
#ifdef Q_OS_MAC
    setAttribute(Qt::AA_DontShowIconsInMenus);
#endif
}

Application::~Application()
{
    delete _mainWindow;
}


void Application::init()
{
#ifdef Q_OS_MAC
    _showWindowMacTimer = 0;
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

void Application::createAndShowMainWindow()
{
    _mainWindow = new MedianXLOfflineTools(_param, _launchMode);
#ifdef Q_OS_MAC
    disableLionWindowRestoration();
    delete _showWindowMacTimer;
#endif
#ifdef DUPE_CHECK
    if (_mainWindow->shouldShowWindow)
#endif
    _mainWindow->show();

#if HAS_QTSINGLEAPPLICATION
    setActivationWindow(_mainWindow);
    connect(this, SIGNAL(messageReceived(const QString &)), SLOT(setParam(const QString &)));
#endif
}

#if HAS_QTSINGLEAPPLICATION
void Application::activateWindow()
{
    QtSingleApplication::activateWindow();
    _mainWindow->loadFile(_param);
}
#endif
