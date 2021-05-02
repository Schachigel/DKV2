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
    int i{0};
    for ( i=0; i<toInt(interestModel::maxId); i++)
        retval.insert(fromInt(i), statSet());
    retval.insert(interestModel::maxId, statSet());

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
    cleanupTestDb();
}

void test_statistics::test_noContractsNoBookings()
{
    stats data =getStatsFromSql(sqlStat_activeContracts_byIMode_toDate, QDate::currentDate());
    QVERIFY(data.value(interestModel::payout) == statSet(0, 0, 0., 0., 0.));
    QVERIFY(data.value(interestModel::reinvest) == statSet(0, 0, 0., 0., 0.));
    QVERIFY(data.value(interestModel::fixed) == statSet(0, 0, 0., 0., 0.));
    QVERIFY(data.value(interestModel::zero) == statSet(0, 0, 0., 0., 0.));
    QVERIFY(data.value(interestModel::allIModels) == statSet(0, 0, 0., 0., 0.));
}

void test_statistics::test_randomContracts_50pActivated()
{
    saveRandomCreditors(10);
    saveRandomContracts(8);
    activateRandomContracts(50/* % */);

    QString activ_sql(sqlStat_activeContracts_byIMode_toDate);
    stats active_data =getStatsFromSql(activ_sql, QDate::currentDate());
    QCOMPARE(active_data.value(interestModel::allIModels).nbrContracts, 4);

    QString inactive_sql(sqlStat_inactiveContracts_byIMode_toDate);
    stats inactive_data =getStatsFromSql(inactive_sql, QDate::currentDate());
    QCOMPARE(inactive_data.value(interestModel::allIModels).nbrContracts, 4);

    QString all_sql(sqlStat_allContracts_byIMode_toDate);
    stats all_data =getStatsFromSql(all_sql, QDate::currentDate());
    QCOMPARE(all_data.value(interestModel::allIModels).nbrContracts, 8);
}

void test_statistics::test_oneContract()
{
    creditor creditor {saveRandomCreditor()};
    contract cont;
    cont.init(creditor.id());
    cont.setInterestModel(interestModel::payout);
    cont.setInterestRate(2.);
    cont.setPlannedInvest(100.);
    cont.saveNewContract();
    QString activ_sql(sqlStat_activeContracts_byIMode_toDate);
    stats active_data =getStatsFromSql(activ_sql, QDate::currentDate());
    for( int i=0; i<=toInt(interestModel::allIModels); i++)
        QCOMPARE(active_data.value(fromInt(i)), statSet(0, 0, 0., 0., 0.));

    QString inactive_sql(sqlStat_inactiveContracts_byIMode_toDate);
    stats inactive_data =getStatsFromSql(inactive_sql, QDate::currentDate());
    QVERIFY2(inactive_data.value(interestModel::allIModels) == statSet(1, 1, 100., 1., 1.), "inctive, all interest models");
    QVERIFY2(inactive_data.value(interestModel::payout) ==     statSet(1, 1, 100., 1., 1.), "inctive, with payout");
    QVERIFY2(inactive_data.value(interestModel::reinvest) ==   statSet(0, 0,   0., 0., 0.), "inctive, reinvesting");
    QVERIFY2(inactive_data.value(interestModel::fixed) ==      statSet(0, 0,   0., 0., 0.), "inctive, fixed");
    QVERIFY2(inactive_data.value(interestModel::zero) ==       statSet(0, 0,   0., 0., 0.), "inctive, no interest");


    QString all_sql(sqlStat_allContracts_byIMode_toDate);
    stats all_data =getStatsFromSql(all_sql, QDate::currentDate());
    QVERIFY2(all_data.value(interestModel::allIModels) == statSet(1, 1, 100., 1., 1.), "all, all interest models");
    QVERIFY2(all_data.value(interestModel::payout) ==     statSet(1, 1, 100., 1., 1.), "all, with payout");
    QVERIFY2(all_data.value(interestModel::reinvest) ==   statSet(0, 0,   0., 0., 0.), "all, reinvesting");
    QVERIFY2(all_data.value(interestModel::fixed) ==      statSet(0, 0,   0., 0., 0.), "all, fixed");
    QVERIFY2(all_data.value(interestModel::zero) ==       statSet(0, 0,   0., 0., 0.), "all, no interest");
}
