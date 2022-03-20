#ifndef RESOURCEPATHMANAGER_HPP
#define RESOURCEPATHMANAGER_HPP

#include "languagemanager.hpp"
#include "itemdatabase.h"

class ResourcePathManager
{
public:
    static QString dataPathForFileName(const QString &fileName) { return QString("%1/data/%2").arg(LanguageManager::instance().resourcesPath).arg(fileName); }
    static QString localizedPathForFileName(const QString &fileName) { return dataPathForFileName(QString("%1/%2.dat").arg(LanguageManager::instance().modLocalization()).arg(fileName)); }
    static QString pathForSortOrderFileName(const QString &fileName) { return dataPathForFileName(QString("sorting/%1.txt").arg(fileName)); }

    static QString pathForImagePath(const QString &imagePath) { return dataPathForFileName(QString("images/%1").arg(imagePath)); }
    static QString pathForItemImageName(const QString &imageName) { return pathForImagePath(QString("items/%1.png").arg(imageName)); }
    static QString pathForSkillImage(int classCode, int imageId)  { return pathForImagePath(QString("skills/%1/%2.jpg").arg(classCode).arg(imageId)); }

    static QString pathForResourceItem(const QString &itemName) { return QString(":/Items/data/items/%1.d2i").arg(itemName); }
};

#endif // RESOURCEPATHMANAGER_HPP
