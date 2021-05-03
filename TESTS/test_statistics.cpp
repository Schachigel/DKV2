#include <QtTest>

#include "../DKV2/dkdbviews.h"
#include "../DKV2/dkdbhelper.h"

#include "test_statistics.h"

stats getStatsFromSql(QString sql, QDate date)
{   LOG_CALL;
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
        qDebug() << rec;
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

stats getStatsActiveContracts(QDate date)
{   LOG_CALL_W(date.toString(Qt::ISODate));
    return getStatsFromSql(sqlStat_activeContracts_byIMode_toDate, date);
}
stats getStatsInactiveContracts(QDate date)
{   LOG_CALL_W(date.toString(Qt::ISODate));
    return getStatsFromSql(sqlStat_inactiveContracts_byIMode_toDate, date);
}
stats getStatsAllContracts(QDate date)
{   LOG_CALL_W(date.toString(Qt::ISODate));
    return getStatsFromSql(sqlStat_allContracts_byIMode_toDate, date);
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
    stats data =getStatsInactiveContracts( QDate::currentDate());
    QVERIFY(data.value(interestModel::payout) == statSet(0, 0, 0., 0., 0.));
    QVERIFY(data.value(interestModel::reinvest) == statSet(0, 0, 0., 0., 0.));
    QVERIFY(data.value(interestModel::fixed) == statSet(0, 0, 0., 0., 0.));
    QVERIFY(data.value(interestModel::zero) == statSet(0, 0, 0., 0., 0.));
    QVERIFY(data.value(interestModel::allIModels) == statSet(0, 0, 0., 0., 0.));

    data =getStatsActiveContracts( QDate::currentDate());
    QVERIFY(data.value(interestModel::payout) == statSet(0, 0, 0., 0., 0.));
    QVERIFY(data.value(interestModel::reinvest) == statSet(0, 0, 0., 0., 0.));
    QVERIFY(data.value(interestModel::fixed) == statSet(0, 0, 0., 0., 0.));
    QVERIFY(data.value(interestModel::zero) == statSet(0, 0, 0., 0., 0.));
    QVERIFY(data.value(interestModel::allIModels) == statSet(0, 0, 0., 0., 0.));

    data =getStatsAllContracts( QDate::currentDate());
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
    // activation date can be ahead of today
    QDate somefuturedate =QDate::currentDate().addYears(1);
    stats active_data =getStatsActiveContracts( somefuturedate);
    QCOMPARE(active_data.value(interestModel::allIModels).nbrContracts, 4);

    stats inactive_data =getStatsInactiveContracts( somefuturedate);
    QCOMPARE(inactive_data.value(interestModel::allIModels).nbrContracts, 4);

    stats all_data =getStatsAllContracts( somefuturedate);
    QCOMPARE(all_data.value(interestModel::allIModels).nbrContracts, 8);
}

void test_statistics::test_contracts_current_statistics()
{
    QDate date(2000, 5, 15);
    creditor creditor1 {saveRandomCreditor()};
    contract contPayout;
    contPayout.init(creditor1.id());
    contPayout.setInterestModel(interestModel::payout);
    contPayout.setInterestRate(1.);
    contPayout.setPlannedInvest(100.);
    contPayout.setConclusionDate(date);
    contPayout.saveNewContract();

    /*
     * one creditor, first contract, wPayout
     */
    stats active_data =getStatsActiveContracts( date);
    for( int i=0; i<=toInt(interestModel::allIModels); i++)
        QCOMPARE(active_data.value(fromInt(i)), statSet(0, 0, 0., 0., 0.));

    stats inactive_data =getStatsInactiveContracts( QDate::currentDate());
    QVERIFY2(inactive_data.value(interestModel::allIModels) == statSet(1, 1, 100., 1., 1.), "inctive, all interest models");
    QVERIFY2(inactive_data.value(interestModel::payout) ==     statSet(1, 1, 100., 1., 1.), "inctive, with payout");
    QVERIFY2(inactive_data.value(interestModel::reinvest) ==   statSet(0, 0,   0., 0., 0.), "inctive, reinvesting");
    QVERIFY2(inactive_data.value(interestModel::fixed) ==      statSet(0, 0,   0., 0., 0.), "inctive, fixed");
    QVERIFY2(inactive_data.value(interestModel::zero) ==       statSet(0, 0,   0., 0., 0.), "inctive, no interest");


    stats all_data =getStatsAllContracts( QDate::currentDate());
    QVERIFY2(all_data.value(interestModel::allIModels) == statSet(1, 1, 100., 1., 1.), "all, all interest models");
    QVERIFY2(all_data.value(interestModel::payout) ==     statSet(1, 1, 100., 1., 1.), "all, with payout");
    QVERIFY2(all_data.value(interestModel::reinvest) ==   statSet(0, 0,   0., 0., 0.), "all, reinvesting");
    QVERIFY2(all_data.value(interestModel::fixed) ==      statSet(0, 0,   0., 0., 0.), "all, fixed");
    QVERIFY2(all_data.value(interestModel::zero) ==       statSet(0, 0,   0., 0., 0.), "all, no interest");

    /*
     * same creditor, different iMode
     */
    date =date.addDays(15);
    contract contReinvest;
    contPayout.init(creditor1.id());
    contPayout.setInterestModel(interestModel::reinvest);
    contPayout.setInterestRate(2.);
    contPayout.setPlannedInvest(100.);
    contPayout.setConclusionDate(date);
    contPayout.saveNewContract();

    active_data =getStatsActiveContracts( date);
    for( int i=0; i<=toInt(interestModel::allIModels); i++)
        QCOMPARE(active_data.value(fromInt(i)), statSet(0, 0, 0., 0., 0.));

    inactive_data =getStatsInactiveContracts( date);
    QVERIFY2(inactive_data.value(interestModel::allIModels) == statSet(2, 1, 200., 3., 1.5), "inctive, all interest models");
    QVERIFY2(inactive_data.value(interestModel::payout) ==     statSet(1, 1, 100., 1., 1.), "inctive, with payout");
    QVERIFY2(inactive_data.value(interestModel::reinvest) ==   statSet(1, 1, 100., 2., 2.), "inctive, reinvesting");
    QVERIFY2(inactive_data.value(interestModel::fixed) ==      statSet(0, 0,   0., 0., 0.), "inctive, fixed");
    QVERIFY2(inactive_data.value(interestModel::zero) ==       statSet(0, 0,   0., 0., 0.), "inctive, no interest");

    all_data =getStatsAllContracts( date);
    QVERIFY2(all_data.value(interestModel::allIModels) == statSet(2, 1, 200., 3., 1.5), "all, all interest models");
    QVERIFY2(all_data.value(interestModel::payout) ==     statSet(1, 1, 100., 1., 1.),  "all, with payout");
    QVERIFY2(all_data.value(interestModel::reinvest) ==   statSet(1, 1, 100., 2., 2.), "all, reinvesting");
    QVERIFY2(all_data.value(interestModel::fixed) ==      statSet(0, 0,   0., 0., 0.), "all, fixed");
    QVERIFY2(all_data.value(interestModel::zero) ==       statSet(0, 0,   0., 0., 0.), "all, no interest");

    /*
     * other creditor, different iMode
     */
    date =date.addDays(15);
    creditor creditor2 {saveRandomCreditor()};
    contract contFix;
    contPayout.init(creditor2.id());
    contPayout.setInterestModel(interestModel::fixed);
    contPayout.setInterestRate(1.5);
    contPayout.setPlannedInvest(100.);
    contPayout.setConclusionDate(date);
    contPayout.saveNewContract();

    active_data =getStatsActiveContracts( date);
    for( int i=0; i<=toInt(interestModel::allIModels); i++)
        QCOMPARE(active_data.value(fromInt(i)), statSet(0, 0, 0., 0., 0.));

    inactive_data =getStatsInactiveContracts( date);
    QVERIFY2(inactive_data.value(interestModel::allIModels) == statSet(3, 2, 300., 4.5 , 1.5), "inctive, all interest models");
    QVERIFY2(inactive_data.value(interestModel::payout) ==     statSet(1, 1, 100., 1. , 1.),  "inctive, with payout");
    QVERIFY2(inactive_data.value(interestModel::reinvest) ==   statSet(1, 1, 100., 2. , 2.),  "inctive, reinvesting");
    QVERIFY2(inactive_data.value(interestModel::fixed) ==      statSet(1, 1, 100., 1.5, 1.5), "inctive, fixed");
    QVERIFY2(inactive_data.value(interestModel::zero) ==       statSet(0, 0,   0., 0. , 0.),  "inctive, no interest");

    all_data =getStatsAllContracts( date);
    QVERIFY2(all_data.value(interestModel::allIModels) == statSet(3, 2, 300., 4.5, 1.5), "all, all interest models");
    QVERIFY2(all_data.value(interestModel::payout) ==     statSet(1, 1, 100., 1. , 1.),  "all, with payout");
    QVERIFY2(all_data.value(interestModel::reinvest) ==   statSet(1, 1, 100., 2. , 2.),  "all, reinvesting");
    QVERIFY2(all_data.value(interestModel::fixed) ==      statSet(1, 1, 100., 1.5, 1.5), "all, fixed");
    QVERIFY2(all_data.value(interestModel::zero) ==       statSet(0, 0,   0., 0. , 0.),  "all, no interest");

    /*
     * third creditor, - no interest
     */

}
