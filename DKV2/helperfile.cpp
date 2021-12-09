
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QProcess>
#include <QTextDocument>
// 4 html pdf generation
#include <QPainter>
#include <QPdfWriter>

#include "helper.h"
#include "helperfile.h"

QString getUniqueTempFilename(const QString &templateFileName)
{
    QTemporaryFile temp {templateFileName +qsl(".preconversion")};
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
{   LOG_CALL_W(fn);
    QString backupname{fn};
    QFileInfo fi{fn};
    QString suffix = fi.completeSuffix();
    QString path = fi.path();
    if( subfolder.size())
    {
        QDir d(path); d.mkdir(subfolder);
        backupname =d.path() + qsl("/") + subfolder + qsl("/") + fi.fileName();
    }
    backupname.chop(suffix.size()+1/*dot*/);
    backupname += "_" + QDateTime::currentDateTime().toString(qsl("yyyyMMdd_hhmmss")) + qsl(".") + suffix;
    // copy the file
    if( not QFile().copy(fn, backupname))
    {
        qDebug() << "Backup copy failed. File to be copied: " << backupname;
        return false;
    }
    QString names(fi.baseName() + qsl("_????????_??????.") + suffix);
    QDir backups(fi.absolutePath(), names, QDir::Name | QDir::Reversed, QDir::Files);
    for (uint i = 30; i < backups.count(); i++) {
        QFile().remove(backups[i]);
    }
    return true;
}

void showFileInFolder(const QString &fullPath)
{   LOG_CALL_W(fullPath);
#ifdef _WIN32    //Code for Windows
    QString explorerW_selectedFile {qsl(" /select,\"%1\"")};
    explorerW_selectedFile =explorerW_selectedFile.arg(QDir::toNativeSeparators(fullPath));
    QProcess p;
    p.setNativeArguments(explorerW_selectedFile);
    p.setProgram(qsl("explorer.exe"));
    qint64 pid;
    p.startDetached(&pid);
    if( not p.waitForStarted(1000))
        qDebug() << "failed to start explorer. Arg was: " << explorerW_selectedFile;

#elif defined(__APPLE__)    //Code for Mac
    QProcess::execute("/usr/bin/osascript", {"-e", "tell application \"Finder\" to reveal POSIX file \"" + fullPath + "\""});
    QProcess::execute("/usr/bin/osascript", {"-e", "tell application \"Finder\" to activate"});
#endif
}

void printHtmlToPdf( const QString &html, const QString &fn)
{   LOG_CALL;
    QTextDocument td;
    td.setHtml(html);

    QPdfWriter pdfw(fn);
    pdfw.setCreator(qsl("Esperanza Franklin GmbH 4 MHS"));
    //pdfw.setPageSize(QPagedPaintDevice::A4);
    pdfw.setPageSize(QPageSize(QPageSize::A4));
    pdfw.setPageOrientation(QPageLayout::Portrait);
    pdfw.setPdfVersion(QPagedPaintDevice::PdfVersion_1_6);
    pdfw.setResolution(120 );

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
}

QString absoluteCanonicalPath(const QString &path)
{
    QString newpath = QFileInfo(path).canonicalFilePath();
    return newpath.isEmpty() ? path : newpath;
}

#if defined(Q_OS_WIN)
HANDLE openDbSignalnFile =INVALID_HANDLE_VALUE;
#else
// QTemporaryFile* openDbIndicationFile =nullptr;
#endif
namespace {
QString indicatorfilenameExtension {qsl(".is_opened_By_Dkv2")};
}

void createSignalFile(const QString filename)
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
        Sleep(1500);
        openDbSignalnFile=CreateFile(indicatorfilename.toStdWString ().c_str (),
                                     GENERIC_READ|GENERIC_WRITE,
                                     FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,
                                     NULL /* security descriptor */,
                                     CREATE_ALWAYS,
                                     FILE_ATTRIBUTE_NORMAL|FILE_FLAG_DELETE_ON_CLOSE,
                                     NULL);
    }
#else
//    if( openDbIndicationFile) delete openDbIndicationFile;
//    openDbIndicationFile =new QTemporaryFile(indicatorfilename);
//    openDbIndicationFile->open ();
#endif
}
void deleteSignalFile()
{
#if defined(Q_OS_WIN)
    CloseHandle(openDbSignalnFile);
#else
//    if( openDbIndicationFile) delete openDbIndicationFile;
#endif
}
bool checkSignalFile(const QString filename) {
#if defined(Q_OS_WIN)
    {
        QString indicatorfilename {filename +indicatorfilenameExtension};
        return QFile::exists (indicatorfilename);
    }
#else
#endif
}
