#include "helper.h"
#include "helperfile.h"
#include "appconfig.h"

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

void showInExplorer(const QString &pathOrFilename, bool fileOrFolder)
{   LOG_CALL_W(pathOrFilename);
#ifdef _WIN32    //Code for Windows
    QString fullPath {pathOrFilename};
    QFileInfo fi(fullPath);
    if(fi.isRelative ())
        fullPath = appConfig::Outdir ().append (qsl("/").append (pathOrFilename));

    QString explorerW_selectedFile = QDir::toNativeSeparators(fullPath);
    if( fileOrFolder == showFile)
        explorerW_selectedFile =qsl(" /select,\"%1\"").arg(explorerW_selectedFile);
    if( fileOrFolder == showFolder)
        explorerW_selectedFile =qsl(" \"%1\"").arg(explorerW_selectedFile);

    QProcess p;
    p.setNativeArguments(explorerW_selectedFile);
    p.setProgram(qsl("explorer.exe"));
    qint64 pid;
    p.startDetached(&pid);
//  ?! debugging showed, that .waitForStarted always returns an error (on my system...)
//    if( not p.waitForStarted(-1))
//        qDebug().noquote ()<< "failed to start explorer. Arg was: " << explorerW_selectedFile << " Error was " << p.error ();

#elif defined(__APPLE__)    //Code for Mac
    Q_UNUSED(fileOrFolder);

    QProcess::execute("/usr/bin/osascript", {"-e", "tell application \"Finder\" to reveal POSIX file \"" + pathOrFilename + "\""});
    QProcess::execute("/usr/bin/osascript", {"-e", "tell application \"Finder\" to activate"});
#else
    Q_UNUSED (fileOrFolder);
#endif
}

void printHtmlToPdf( const QString &html, const QString &css, const QString &fn)
{   LOG_CALL;
    QTextDocument td;
    td.setDefaultStyleSheet (css);
    td.setHtml(html);

    QPdfWriter pdfw(fn);
    pdfw.setCreator(qsl("Esperanza Franklin GmbH 4 MHS"));
    //pdfw.setPageSize(QPagedPaintDevice::A4);
    pdfw.setPageSize(QPageSize(QPageSize::A4));
    pdfw.setPageOrientation(QPageLayout::Portrait);
    pdfw.setPdfVersion(QPagedPaintDevice::PdfVersion_1_6);
//    pdfw.setResolution(120 );

    QPageSize ps(QPageSize::A4);
    QMarginsF qmf( 188,235, 141, 235);
    QPageLayout pl(ps, QPageLayout::Portrait, qmf, QPageLayout::Unit::Millimeter);
    pl.setMode(QPageLayout::FullPageMode/*respect margins*/);

    pdfw.setPageLayout(pl);
    QPainter painter(&pdfw);
    td.adjustSize();
    td.drawContents(&painter);

    td.print( &pdfw);
}

QString absoluteCanonicalPath(const QString &path)
{
    QString newpath = QFileInfo(path).canonicalFilePath();
    return newpath.isEmpty() ? path : newpath;
}

QString fileToString( const QString& filename)
{   LOG_CALL_W(filename);
    QFile f(filename);
    if( not f.open(QFile::ReadOnly))
        return qsl("file open error");
    return f.readAll();
}

bool stringToFile( const QString& string, const QString& fullFileName)
{   LOG_CALL;
    QFile file(fullFileName);
    if( not file.open (QFile::WriteOnly)) {
        qCritical() << "opening file failed";
        return false;
    }
    return file.write( string.toUtf8 ()) > 0;
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
