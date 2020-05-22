#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlField>
#include <qdebug.h>
#include <QTest>

#include "../DKV2/dkdbhelper.h"
#include "testhelper.h"


const QString testDbFilename = "..\\data\\testdb.sqlite";

void initTestDb()
{   LOG_CALL;
    QDir().mkdir(QString("..\\data"));
    if (QFile::exists(testDbFilename))
        QFile::remove(testDbFilename);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(testDbFilename);
    QVERIFY(db.open());
    QVERIFY2( QFile::exists(testDbFilename), "create database failed." );
}

void cleanupTestDb()
{   LOG_CALL;
    closeDatabaseConnection();
    if (QFile::exists(testDbFilename))
        QFile::remove(testDbFilename);
    QDir().rmdir("..\\data");
    QVERIFY2( (QFile::exists(testDbFilename) == false), "destroy database failed." );
}

int tableRecordCount( QString tname)
{   // LOG_CALL_W(tname);
    QSqlQuery q;
    if (q.exec("SELECT COUNT(*) FROM " + tname)) {
        q.first();
        qDebug() << "#Datensätze: " << q.record().value(0).toInt();
        return q.record().value(0).toInt();
    } else {
        qCritical() << "tableRecordCount: SELECT failed " << q.lastError() << endl << q.lastQuery() << endl;
        return -1;
    }
}

bool dbHasTable(const QString tname)
{   LOG_CALL_W(tname);
    return QSqlDatabase::database().tables().contains(tname);
}

bool dbTableHasField(const QString tname, const QString fname)
{   LOG_CALL_W(tname +": " +fname);
    QSqlRecord r = QSqlDatabase::database().record(tname);
    if( r.field(fname).isValid())
        return true;
    return false;
}
