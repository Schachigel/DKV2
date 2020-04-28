#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlField>
#include <qdebug.h>

#include "testhelper.h"

const QString testDbFilename = "..\\..\\data\\testdb.sqlite";
const QString testCon = "test_connection";
QSqlDatabase testDb()
{
    return QSqlDatabase::database(testCon);
}


int tableRecordCount(QString tname)
{   LOG_CALL_W(tname);
    QSqlQuery q(testDb());
    if (q.exec("SELECT COUNT(*) FROM " + tname)) {
        q.first();
        qDebug() << "#Datensätze: " << q.record().value(0);
        return q.record().value(0).toInt();
    } else {
        qCritical() << "selecting data failed " << q.lastError() << endl << q.lastQuery() << endl;
        return -1;
    }
}

bool dbHasTable(const QString tname)
{   LOG_CALL_W(tname);
    return testDb().tables().contains(tname);
}

bool dbTableHasField(const QString tname, const QString fname)
{   LOG_CALL_W(tname +": " +fname);
    QSqlRecord r = testDb().record(tname);
    if( r.field(fname).isValid())
        return true;
    return false;
}
