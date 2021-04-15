#include <QtTest>

#include "../DKV2/dkdbviews.h"
#include "../DKV2/dkdbhelper.h"

#include "test_statistics.h"

stats getStatsFromSql(QString sql, QDate date) {
    stats retval;
    QSqlQuery q; q.prepare(sql.replace(qsl(":date"), date.toString(Qt::ISODate)));
    if( not q.exec()) {
        qInfo() << "getStatsFromSql: Query failed: " << q.lastError() << Qt::endl << q.lastQuery();
        return retval;
    }
    while( q.next()) {
        QSqlRecord rec =q.record();
        interestModel key =rec.value(0).toString() == qsl("all") ? interestModel::maxId : fromInt(rec.value(0).toInt());
        statSet set;
        set.nbrContracts =rec.value(1).toInt();
        set.nbrCreditors =rec.value(2).toInt();
        set.volume       =rec.value(3).toDouble();
        set.interest     =rec.value(4).toDouble();
        set.avgInterest  =rec.value(5).toDouble();
        retval.insert(key, set);
    }
    if( retval.size() not_eq toInt(interestModel::maxId)) {
        qCritical() << "getStatsFromSql: data not complete. retval size:" << retval.count();
        return stats();
    }
    return retval;
}

void test_statistics::initTestCase()
{   LOG_CALL;
    init_DKDBStruct();
    initTestDb();
    fill_DkDbDefaultContent(QSqlDatabase::database(), true);
    closeAllDatabaseConnections();
    QFile::copy(testDbFilename, testTemplateDb);
}

void test_statistics::cleanupTestCase()
{   LOG_CALL;
    QFile::remove(testDbFilename);
    QFile::remove(testTemplateDb);
}

void test_statistics::init()
{   LOG_CALL;
    QFile::remove(testDbFilename);
    QVERIFY(not QFile::exists(testDbFilename));
    QFile::copy(testTemplateDb, testDbFilename);
    QSqlDatabase db =QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(testDbFilename);
    QVERIFY(db.open());
}

void test_statistics::cleanup()
{   LOG_CALL;
    closeAllDatabaseConnections();
    QFile::remove(testDbFilename);
}

void test_statistics::test_start()
{
    QString sql(sqlStat_activeContracts_byIMode_toDate);
    stats data =getStatsFromSql(sql, QDate(2021,3,31));
    QVERIFY(not data.isEmpty());

    QVERIFY(data.value(interestModel::payout) == statSet(0, 0, 0., 0., 0.));
}
