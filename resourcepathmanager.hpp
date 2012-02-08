#ifndef RESOURCEPATHMANAGER_HPP
#define RESOURCEPATHMANAGER_HPP

#include "languagemanager.hpp"


class ResourcePathManager
{
public:
    static QString dataPathForFileName(const QString &fileName) { return QString("%1/data/%2").arg(LanguageManager::instance().resourcesPath).arg(fileName); }
    static QString localizedPathForFileName(const QString &fileName) { return dataPathForFileName(QString("%1/%2.txt").arg(LanguageManager::instance().locale()).arg(fileName)); }
    static QString pathForImageName(const QString &imageName) { return dataPathForFileName(QString("images/%1.png").arg(imageName)); }
};

#endif // RESOURCEPATHMANAGER_HPP
