
#include "helper.h"
#include "helperfile.h"
#include "appconfig.h"


QString tempPathTemplateFromPath (const QString& path, const QString &purpose)
{
    QFileInfo fi(path);
    QString ext =fi.suffix ();
    return path.left(path.size ()-fi.suffix ().size ()-1).append(qsl(".")).append(purpose).append ("_XXXXXX.").append (ext);
}

QString getUniqueTempFilename(const QString &templateFileName, const QString& purpose)
{
    QTemporaryFile temp {tempPathTemplateFromPath(templateFileName, purpose)};
    temp.open();
    return temp.fileName();
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
    if (not QFile::exists(fn)) {
        qDebug() << "No need to backup not existing file: " << fn;
        return true;
    }
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
    if( not QFile().copy(fn, backupname)) {
        qDebug() << "Backup copy failed. File to be copied: " << backupname;
        return false;
    }
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
    qDebug() << td.size();
    td.adjustSize();
    qDebug() << td.size();
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
{
    QFile file(fullFileName);
    file.open (QFile::WriteOnly);
    return file.write( string.toUtf8 ()) > 0;
}

#if defined(Q_OS_WIN)
HANDLE openDbSignalnFile =INVALID_HANDLE_VALUE;
#else
QFile* openDbIndicationFile =nullptr;
#endif
namespace {
QString indicatorfilenameExtension {qsl(".is_opened_By_Dkv2")};
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
        delete openDbIndicationFile;
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
