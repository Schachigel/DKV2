#include "filehelper.h"
#include <qstring.h>
#include <qlist.h>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <qdebug.h>


bool overwrite_copy(const QString& from, const QString& to)
{
    qDebug() << from << " to " << to << endl;
    if( QFile().exists(to))
        QFile().remove(to);
    return QFile().copy(from, to);
}

bool backupFile(const QString&  fn)
{
    QList<QString> backupnames;
    for(int i = 0; i<10; i++)
    {
        QString nbr;
        QTextStream ts(&nbr); ts.setFieldWidth(2); ts.setPadChar('0');
        ts << i;
        QString backupextension (QString("-"+ nbr + ".bak"));
        QString name = fn + backupextension;
        backupnames.append(name);
    }
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
            qDebug() << "Backup copy failed. File to be copied: " << backupnames[i] << endl;
        }
    }

    if( QFile().exists(fn))
        if( !overwrite_copy(fn, backupnames[0]))
        {
            ret = false;
            qDebug() << "Backup copy failed. File to be copied: " << backupnames[0] << endl;
        }
    return ret;
}
