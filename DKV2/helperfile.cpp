
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QTextDocument>
// 4 html pdf generation
#include <QPainter>
#include <QPdfWriter>

#include "helper.h"
#include "helperfile.h"

bool backupFile(const QString&  fn, const QString& subfolder)
{   LOG_CALL_W(fn);
    QString backupname{fn};
    QFileInfo fi{fn};
    QString suffix = fi.completeSuffix();
    QString path = fi.path();
    if( subfolder.length()!= 0)
    {
        QDir d(path); d.mkdir(subfolder);
        backupname =d.path() + qsl("/") + subfolder + qsl("/") + fi.fileName();
    }
    backupname.chop(suffix.size()+1/*dot*/);
    backupname += "_" + QDateTime::currentDateTime().toString(qsl("yyyyMMdd_hhmmss")) + qsl(".") + suffix;
    // copy the file
    if(!QFile().copy(fn, backupname))
    {
        qDebug() << "Backup copy failed. File to be copied: " << backupname;
        return false;
    }
    QString names(fi.baseName() + qsl("_????????_??????.") + suffix);
    QDir backups(fi.absolutePath(), names, QDir::Name | QDir::Reversed, QDir::Files);
    for (uint i = 15; i < backups.count(); i++) {
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
    if( !p.waitForStarted(1000))
        qDebug() << "failed to start explorer. Arg was: " << explorerW_selectedFile;

#elif defined(__APPLE__)    //Code for Mac
    QProcess::execute("/usr/bin/osascript", {"-e", "tell application \"Finder\" to reveal POSIX file \"" + fullPath + "\""});
    QProcess::execute("/usr/bin/osascript", {"-e", "tell application \"Finder\" to activate"});
#endif
}

void printHtmlToPdf( QString html, QString fn)
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
