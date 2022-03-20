#ifndef FILEASSOCIATIONMANAGER_H
#define FILEASSOCIATIONMANAGER_H

#include <QtGlobal>


class FileAssociationManager
{
public:
    // extension may or may not have preceding dot: 'avi' and '.mp3' are both valid to pass as a parameter
    static bool isApplicationDefaultForExtension(const QString &extension);
    static void makeApplicationDefaultForExtension(const QString &extension);

#ifdef Q_OS_WIN32
    static QString progIdForExtension(const QString &extension);
    static void registerApplicationForExtension(const QString &extension);
#endif
};

#endif // FILEASSOCIATIONMANAGER_H
