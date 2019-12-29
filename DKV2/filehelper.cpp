
#include <QString>
#include <QList>
#include <QTextStream>
#include <QTextDocument>

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QSettings>
// 4 html pdf generation
#include <QPainter>
#include <QPdfWriter>

#include "helper.h"
#include "filehelper.h"

bool overwrite_copy(const QString& from, const QString& to)
{
    qDebug() << "overwrite_copy " << from << " to " << to;
    if( QFile().exists(to))
        QFile().remove(to);
    return QFile().copy(from, to);
}

bool backupFile(const QString&  fn)
{
    // generate list of backup filenames
    QList<QString> backupnames;
    for(int i = 0; i<10; i++)
    {
        QString backupname{fn};
        QFileInfo fi{fn}; QString suffix = fi.completeSuffix();
        backupname.chop(suffix.size()+1/*dot*/);
        QString nbr(QString ("%1").arg(i,int(2),10, QLatin1Char('0')));
        backupname +=  QString("-"+ nbr + "." + suffix + ".bak");
        backupnames.append(backupname);
    }
    // copy existing files to filename with next index from top to bottom
    bool ret(true);
    if( QFile().exists(backupnames[9]))
        ret &= QFile().remove(backupnames[9]);
    for(int i = 8; i>=0; i--)
    {
        if( !QFile().exists(backupnames[i]))
            continue;
        if( !overwrite_copy(backupnames[i], backupnames[i+1]))
        {
            ret = false;
            qDebug() << "Backup copy failed. File to be copied: " << backupnames[i];
        }
    }
    // copy the last file: the one w the filename of the file you want to create
    if( QFile().exists(fn))
        if( !overwrite_copy(fn, backupnames[0]))
        {
            ret = false;
            qDebug() << "Backup copy failed. File to be copied: " << backupnames[0];
        }
    return ret;
}

void showFileInFolder(const QString &path)
{
#ifdef _WIN32    //Code for Windows
    QProcess::startDetached("explorer.exe", {"/select,", QDir::toNativeSeparators(path)});
#elif defined(__APPLE__)    //Code for Mac
    QProcess::execute("/usr/bin/osascript", {"-e", "tell application \"Finder\" to reveal POSIX file \"" + path + "\""});
    QProcess::execute("/usr/bin/osascript", {"-e", "tell application \"Finder\" to activate"});
#endif
}

void printHtmlToPdf( QString html, QString fn)
{
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

QString getDbFolder()
{
    QSettings config;
    QFileInfo db(config.value("db/last").toString());
    return db.path();
}
