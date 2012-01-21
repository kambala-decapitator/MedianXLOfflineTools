#include "medianxlofflinetools.h"
#include "languagemanager.h"

#include <QApplication>

#include <QSettings>
#include <QTranslator>


int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	app.setOrganizationName("kambala");
	app.setApplicationName("Median XL Offline Tools");
	app.setApplicationVersion("0.2");


	QSettings settings;
	LanguageManager::instance().currentLocale = settings.value(LanguageManager::instance().languageKey, QLocale::system().name().left(2)).toString().toLatin1();

	QTranslator myappTranslator;
	if (!myappTranslator.load(app.applicationName().remove(' ').toLower() + "_" + LanguageManager::instance().currentLocale, LanguageManager::instance().translationsPath))
		LanguageManager::instance().currentLocale = LanguageManager::instance().defaultLocale;
	app.installTranslator(&myappTranslator);

	QTranslator qtTranslator;
	qtTranslator.load("qt_" + LanguageManager::instance().currentLocale, LanguageManager::instance().translationsPath);
	app.installTranslator(&qtTranslator);


	MedianXLOfflineTools w;
	w.show();

	return app.exec();
}
