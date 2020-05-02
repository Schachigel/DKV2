
#include <QString>
#include <QList>
#include <QTextStream>
#include <QTextDocument>

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
//#include <QSettings>
// 4 html pdf generation
#include <QPainter>
#include <QPdfWriter>

#include "helper.h"
#include "filehelper.h"

bool backupFile(const QString&  fn, const QString& subfolder)
{   LOG_CALL_W(fn);
    QString backupname{fn};
    QFileInfo fi{fn};
    QString suffix = fi.completeSuffix();
    QString path = fi.path();
    if( subfolder.length()!= 0)
    {
        QDir d(path); d.mkdir(subfolder);
        backupname =d.path() + "/" + subfolder + "/" + fi.fileName();
    }
    backupname.chop(suffix.size()+1/*dot*/);
    backupname += "_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + "." + suffix;
    // copy the file
    if(!QFile().copy(fn, backupname))
    {
        qDebug() << "Backup copy failed. File to be copied: " << backupname;
        return false;
    }
    QString names(fi.baseName() + "_????????_??????." + suffix);
    QDir backups(fi.absolutePath(), names, QDir::Name | QDir::Reversed, QDir::Files);
    for (uint i = 15; i < backups.count(); i++) {
        QFile().remove(backups[i]);
    }
    return true;
}

void showFileInFolder(const QString &path)
{   LOG_CALL_W(path);
#ifdef _WIN32    //Code for Windows
    QProcess::startDetached("explorer.exe", {"/select,", QDir::toNativeSeparators(path)});
#elif defined(__APPLE__)    //Code for Mac
    QProcess::execute("/usr/bin/osascript", {"-e", "tell application \"Finder\" to reveal POSIX file \"" + path + "\""});
    QProcess::execute("/usr/bin/osascript", {"-e", "tell application \"Finder\" to activate"});
#endif
}

void printHtmlToPdf( QString html, QString fn)
{   LOG_CALL;
    QTextDocument td;
    td.setHtml(html);

    QPdfWriter pdfw(fn);
    pdfw.setCreator("Esperanza Franklin GmbH 4 MHS");
    pdfw.setPageSize(QPagedPaintDevice::A4);
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
