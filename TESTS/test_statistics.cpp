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
    // init
    for (int i=0; i<=toInt(interestModel::maxId); i++)
        retval.insert(fromInt(i), statSet());

    int recordCount =0;
    while( q.next()) {
        QSqlRecord rec =q.record();
        recordCount++;
        interestModel key =rec.value(0).toString() == qsl("all") ? interestModel::maxId : fromInt(rec.value(0).toInt());
        statSet set;
        set.nbrContracts =rec.value(1).toInt();
        set.nbrCreditors =rec.value(2).toInt();
        set.volume       =rec.value(3).toDouble();
        set.interest     =rec.value(4).toDouble();
        set.avgInterest  =rec.value(5).toDouble();
        retval.insert(key, set);
    }
    qInfo().noquote() << "read " << recordCount << qsl(" set(s) from the database");
    return retval;
}

void test_statistics::initTestCase()
{   LOG_CALL;
    createTestDbTemplate();
}

void test_statistics::cleanupTestCase()
{   LOG_CALL;
    cleanupTestDbTemplate();
}

void test_statistics::init()
{   LOG_CALL;
    initTestDbFromTemplate();
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

    QVERIFY(data.value(interestModel::payout) == statSet(0, 0, 0., 0., 0.));
}

void test_statistics::test_oneInactiveContract()
{
    saveRandomCreditors(10);
    saveRandomContracts(8);
    activateRandomContracts(50/* % */);

    QString sql(sqlStat_activeContracts_byIMode_toDate);
    stats data =getStatsFromSql(sql, QDate(2021,3,31));

    QCOMPARE(data.value(interestModel::maxId).nbrContracts, 4);

}
