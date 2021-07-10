#include <QFile>
#include <QFileInfo>
#include <QDirIterator>

#define CRC_OF_BYTEARRAY(byteArray) qChecksum(byteArray.constData(), byteArray.length())

bool compressFile(const QFileInfo &fi)
{
    QFile in(fi.filePath());
    if (!in.open(QIODevice::ReadOnly))
    {
        qWarning("error opening file '%s'\nreason: %s", qPrintable(fi.canonicalFilePath()), qPrintable(in.errorString()));
        return false;
    }
    QByteArray fileData = in.readAll();
    in.close();

    QFile out(fi.canonicalPath() + '/' + fi.baseName() + ".dat");
    if (!out.open(QIODevice::WriteOnly))
    {
        qWarning("error creating file '%s'\nreason: %s", qPrintable(out.fileName()), qPrintable(out.errorString()));
        return false;
    }
    QByteArray compressedData = qCompress(fileData);
    quint16 crcCompressed = CRC_OF_BYTEARRAY(compressedData), crcOriginal = CRC_OF_BYTEARRAY(fileData);
    const char crcData[4] = {crcCompressed & 255, crcCompressed >> 8, crcOriginal & 255, crcOriginal >> 8};
    out.write(crcData, 4);
    out.write(compressedData);

    return true;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        qDebug("usage: compressfiles paths...");
        return 1;
    }

    for (int i = 1; i < argc; ++i)
    {
        QFileInfo fi(argv[i]);
        if (fi.isDir())
        {
            QDirIterator it(fi.canonicalFilePath(), QStringList("*.csv"), QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
            do
            {
                it.next();
                QFileInfo f = it.fileInfo();
                if (f.isFile())
                    compressFile(f);
            } while (it.hasNext());
        }
        else
            compressFile(fi);
    }
    return 0;
}
