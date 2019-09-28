#include "filehelper.h"
#include <qstring.h>
#include <qlist.h>
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <qdebug.h>


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

QString logFilePath()
{
    static QString logFilePath(QDir::toNativeSeparators(QDir::tempPath()) + QDir::separator() + "dkv2.log");
    return logFilePath;
}
