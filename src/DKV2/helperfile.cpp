#include "helperfile.h"
#include "helper_core.h"

QString getUniqueTempFilename(const QString &templateFileName, const QString& purpose)
{
    // create a unique filename that indicates the purpose of the file
    QFileInfo fi(templateFileName);
    QString result =templateFileName.left(templateFileName.size ()-fi.suffix ().size ()-1);
    result = result.append(purpose).append(qsl("_XXXXXX.")).append (fi.suffix ());
    // make sure the file can be used
    QTemporaryFile temp {result};
    [[maybe_unused]] auto x =temp.open(); // creates the file
    return temp.fileName();
    // destruction will delete the file, so the name is available for being created again
}

bool moveToBackup(const QString &fn)
{
    if( not QFile(fn).exists())
        // no file, no backup
        return true;

    backupFile(fn, qsl("db-bak"));
    QFile(fn).remove();
    if( not QFile(fn).exists())
        return true;
    else {
        qCritical() << "File to be replaced can not be deleted";
        return false;
    }
}

bool backupFile(const QString&  fn, const QString& subfolder)
{
    if (not QFile::exists(fn))
        RETURN_OK( true, "No need to backup not existing file: ", fn);

    QString backupname{fn};
    QFileInfo fi{fn};
    QString suffix = fi.completeSuffix();
    QString path = fi.path();
    if( subfolder.size()) {
        QDir d(path); d.mkdir(subfolder);
        backupname =d.path() + qsl("/") + subfolder + qsl("/") + fi.fileName();
    }
    backupname.chop(suffix.size()+1/*dot*/);
    backupname += "_" + QDateTime::currentDateTime().toString(qsl("yyyyMMdd_hhmmss")) + qsl(".") + suffix;
    // copy the file
    if( not QFile().copy(fn, backupname))
        RETURN_ERR(false, qsl("Backup copy failed. File to be copied: "), backupname);

    qInfo() << qsl("Backup succeeded from %1 to %2").arg(fn, backupname);
    QString names(fi.baseName() + qsl("_????????_??????.") + suffix);
    QDir backups(fi.absolutePath(), names, QDir::Name | QDir::Reversed, QDir::Files);
    for (uint i = 30; i < backups.count(); i++) {
        QFile().remove(backups[i]);
        qInfo() << "Removed old backup DB: " << backups[i];
    }
    return true;
}

QString absoluteCanonicalPath(const QString &path)
{
    QString newpath = QFileInfo(path).canonicalFilePath();
    return newpath.isEmpty() ? path : newpath;
}

QString readFileToString( const QString& filepath)
{   LOG_CALL_W(filepath);
    QFile f(filepath);
    if( not f.open(QFile::ReadOnly|QIODevice::Text))
        return qsl("file open error");
    QTextStream ts(&f);
    ts.setEncoding(QStringConverter::Utf8);
    ts.setAutoDetectUnicode(true);
    return f.readAll();
}

QString makeSafeFileName(QString name, int maxSize)
{
    // Windows-compatible superset (safe on all platforms)
    static const QRegularExpression illegalChars(
        R"([<>:"/\\|?*\x00-\x1F])"
        );

    // 1) remove illegal characters
    name.remove(illegalChars);

    // 2) trim trailing spaces and dots (Windows restriction)
    while (name.endsWith(' ') || name.endsWith('.'))
        name.chop(1);

    // 3) avoid empty result
    if (name.isEmpty())
        name = QStringLiteral("_");

#ifdef Q_OS_WIN
    // 4) avoid reserved device names (case-insensitive)
    static const QSet<QString> reserved = {
        "CON","PRN","AUX","NUL",
        "COM1","COM2","COM3","COM4","COM5","COM6","COM7","COM8","COM9",
        "LPT1","LPT2","LPT3","LPT4","LPT5","LPT6","LPT7","LPT8","LPT9"
    };

    const QString upper = name.toUpper();
    if (reserved.contains(upper))
        name.prepend('_');
#endif
    if( maxSize > 0)
        name =name.left(maxSize);
    return name;
}



#if defined(Q_OS_WIN)
HANDLE openDbSignalnFile =INVALID_HANDLE_VALUE;
#else
QFile* openDbIndicationFile =nullptr;
#endif
namespace {
QString indicatorfilenameExtension {qsl(".is_opened_By_Dkv2")}; // clazy:exclude=non-pod-global-static
}

void createSignalFile(const QString& filename)
{
#ifdef WIN32
    {
        QString indicatorfilename {filename + indicatorfilenameExtension};
        if( openDbSignalnFile not_eq INVALID_HANDLE_VALUE)
            deleteSignalFile ();
        openDbSignalnFile=CreateFile(indicatorfilename.toStdWString ().c_str (),
                                     GENERIC_READ|GENERIC_WRITE,
                                     FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,
                                     NULL /* security descriptor */,
                                     CREATE_ALWAYS,
                                     FILE_ATTRIBUTE_NORMAL,
                                     NULL);
        DWORD written =0;
        WriteFile(openDbSignalnFile, (LPCVOID)"DKV2 is running", (DWORD)sizeof("DKV2 is running"), &written, NULL);
        CloseHandle(openDbSignalnFile);
        Sleep(2500);
        openDbSignalnFile=CreateFile(indicatorfilename.toStdWString ().c_str (),
                                     GENERIC_READ|GENERIC_WRITE,
                                     FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,
                                     NULL /* security descriptor */,
                                     CREATE_ALWAYS,
                                     FILE_ATTRIBUTE_NORMAL|FILE_FLAG_DELETE_ON_CLOSE,
                                     NULL);
    }
#else
    QString indicatorfilename{filename + indicatorfilenameExtension};
    if (openDbIndicationFile)
        deleteSignalFile();
    openDbIndicationFile = new QFile(indicatorfilename);
    bool ok = openDbIndicationFile->open(QIODevice::WriteOnly | QIODevice::NewOnly);
    if (ok) {
        openDbIndicationFile->write("DKV2 Database is in use!");
        openDbIndicationFile->close();
    }
#endif
}

void deleteSignalFile()
{
#if defined(Q_OS_WIN)
    CloseHandle(openDbSignalnFile);
#else
    if( openDbIndicationFile) {
        openDbIndicationFile->remove();
        delete openDbIndicationFile;
}
#endif
}

bool checkSignalFile(const QString& filename) {
#if defined(Q_OS_WIN)
    {
        QString indicatorfilename {filename +indicatorfilenameExtension};
        return QFile::exists (indicatorfilename);
    }
#else
    QString indicatorfilename {filename +indicatorfilenameExtension};
    return QFile::exists (indicatorfilename);
#endif
}
