#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include "helpers.h"

#include <QString>


class LanguageManager
{
public:
	static LanguageManager &instance()
	{
		static LanguageManager obj;
		return obj;
	}

	QString locale() const { return currentLocale == "ru" ? "ru" : defaultLocale; }

	const QString languageKey, defaultLocale, translationsPath;
	QString currentLocale;

private:
	LanguageManager() : languageKey("language"), defaultLocale("en"), translationsPath(resourcesPath + "/translations") {}
};

#endif // LANGUAGEMANAGER_H
