#ifndef LANGUAGEMANAGER_HPP
#define LANGUAGEMANAGER_HPP

#include <QString>
#include <QDir>


class LanguageManager
{
public:
    static LanguageManager &instance()
    {
        static LanguageManager obj;
        return obj;
    }

    const QString &modLocalization() const { static const QString s(QDir(QString("%1/data/%2").arg(resourcesPath, currentLocale)).exists() ? currentLocale : defaultLocale); return s; }
    void setResourcesPath(const QString &path) { resourcesPath = path; translationsPath = resourcesPath + "/translations"; }

    const QString languageKey, defaultLocale;
    QString currentLocale, resourcesPath, translationsPath;

private:
    LanguageManager() : languageKey("language"), defaultLocale("en") {}
};

#endif // LANGUAGEMANAGER_HPP
