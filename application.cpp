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

    _mainWindow = new MedianXLOfflineTools(argc > 1 ? argv[1] : QString());
    _mainWindow->show();
}

Application::~Application()
{
    delete _mainWindow;
}

//#include <execinfo.h>
//void print_trace (void)
//{
//    void *array[10];
//    size_t size;
//    char **strings;
//    size_t i;

//    size = backtrace (array, 50);
//    strings = backtrace_symbols (array, size);

//    CFStringRef s = CFStringCreateWithFormat(kCFAllocatorDefault, 0, CFSTR("Obtained %zd stack frames"), size);
//    CFShow(s);
//    CFRelease(s);
//    //       printf ("Obtained %zd stack frames.\n", size);
//    for (i = 0; i < size; i++)
//    {
//        //          printf ("%s\n", strings[i]);
//        s = CFStringCreateWithCString(kCFAllocatorDefault, strings[i], kCFStringEncodingUTF8);
//        CFShow(s);
//        CFRelease(s);
//    }

//    free (strings);
//}

#ifdef Q_WS_MACX
bool Application::event(QEvent *ev)
{
    if (ev->type() == QEvent::FileOpen)
    {
//        print_trace();
        _mainWindow->loadFile(static_cast<QFileOpenEvent *>(ev)->file());
        return true;
    }
    return QApplication::event(ev);
}
#endif
