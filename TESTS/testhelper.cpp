#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlField>
#include <qdebug.h>

#include "testhelper.h"
const QString testCon = "test_connection";

int tableRecordCount(QString tname)
{
    QSqlQuery q(QSqlDatabase::database(testCon));
    if (q.exec("SELECT COUNT(*) FROM " + tname)) {
        q.next();
        qDebug() << "#Datensätze: " << q.record().value(0);
        return q.record().value(0).toInt();
    } else {
        qCritical() << "selecting data failed " << q.lastError() << endl << q.lastQuery() << endl;
        return -1;
    }
}

bool dbHasTable(const QString tname)
{
    return QSqlDatabase::database(testCon).tables().contains(tname);
}

bool dbTableHasField(const QString tname, const QString fname)
{
    QSqlRecord r = QSqlDatabase::database(testCon).record(tname);
    if( r.field(fname).isValid())
        return true;
    return false;
}
