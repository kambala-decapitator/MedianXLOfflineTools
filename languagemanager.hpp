#ifndef LANGUAGEMANAGER_HPP
#define LANGUAGEMANAGER_HPP

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
    void setResourcesPath(const QString &path) { resourcesPath = path; translationsPath = resourcesPath + "/translations"; }

    const QString languageKey, defaultLocale;
    QString currentLocale, resourcesPath, translationsPath;

private:
    LanguageManager() : languageKey("language"), defaultLocale("en") {}
};

#endif // LANGUAGEMANAGER_HPP