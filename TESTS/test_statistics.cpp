#include <QtTest>

#include "../DKV2/dkdbviews.h"
#include "../DKV2/creditor.h"

#include "testhelper.h"
#include "test_statistics.h"

stats getStatsFromSql(QString sql, QDate date)
{   LOG_CALL;
    stats retval;
    QSqlQuery q; q.prepare(sql.replace(qsl(":date"), date.toString(Qt::ISODate)));
    if( not q.exec()) {
        qInfo() << "getStatsFromSql: Query failed: " << q.lastError() << "\n" << q.lastQuery();
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

void test_statistics::init()
{   LOG_CALL;
    initTestDb_InMemory();
}

void test_statistics::cleanup()
{   LOG_CALL;
    cleanupTestDb_InMemory();
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
    QDate somefuturedate =QDate::currentDate().addYears(2);
    stats active_data =getStatsActiveContracts( somefuturedate);
    QCOMPARE(active_data.value(interestModel::allIModels).nbrContracts, 4);

    stats inactive_data =getStatsInactiveContracts( somefuturedate);
    QCOMPARE(inactive_data.value(interestModel::allIModels).nbrContracts, 4);

    stats all_data =getStatsAllContracts( somefuturedate);
    QCOMPARE(all_data.value(interestModel::allIModels).nbrContracts, 8);
}

void test_statistics::test_contracts_current_statistics()
{
    QDate dateFirstContract(2000, 5, 5);
    /*
     * one creditor, first contract, wPayout
     */
    creditor creditor1 {saveRandomCreditor()};
    contract contPayout;
    contPayout.initContractDefaults(creditor1.id());
    contPayout.setInterestModel(interestModel::payout);
    contPayout.setInterestRate(1.);
    contPayout.setPlannedInvest(100.);
    contPayout.setConclusionDate(dateFirstContract);
    contPayout.saveNewContract();

    stats expected_startDate_active {{interestModel::allIModels, statSet(0, 0, 0., 0., 0.)}
                          ,{interestModel::payout,    statSet(0, 0, 0., 0., 0.)}
                          ,{interestModel::reinvest,  statSet(0, 0, 0., 0., 0.)}
                          ,{interestModel::fixed,     statSet(0, 0, 0., 0., 0.)}
                          ,{interestModel::zero,     statSet(0, 0, 0., 0., 0.)}};
    QCOMPARE(getStatsActiveContracts( dateFirstContract), expected_startDate_active);

    stats expected_startDate_inactive {{interestModel::allIModels, statSet(1, 1, 100., 1., 1.)}
                          ,{interestModel::payout,                 statSet(1, 1, 100., 1., 1.)}
                          ,{interestModel::reinvest,               statSet(0, 0,   0., 0., 0.)}
                          ,{interestModel::fixed,                  statSet(0, 0,   0., 0., 0.)}
                          ,{interestModel::zero,                   statSet(0, 0,   0., 0., 0.)}};
    QCOMPARE(getStatsInactiveContracts( dateFirstContract), expected_startDate_inactive);

    stats expected_startDate_all  {{interestModel::allIModels,     statSet(1, 1, 100., 1., 1.)}
                          ,{interestModel::payout,                 statSet(1, 1, 100., 1., 1.)}
                          ,{interestModel::reinvest,               statSet(0, 0,   0., 0., 0.)}
                          ,{interestModel::fixed,                  statSet(0, 0,   0., 0., 0.)}
                          ,{interestModel::zero,                   statSet(0, 0,   0., 0., 0.)}};
    QCOMPARE(getStatsAllContracts( dateFirstContract), expected_startDate_all);

    /*
     * same creditor, different iMode
     */
    QDate dateSecondContract =dateFirstContract.addDays(10);
    contract contReinvest;
    contReinvest.initContractDefaults(creditor1.id());
    contReinvest.setInterestModel(interestModel::reinvest);
    contReinvest.setInterestRate(2.);
    contReinvest.setPlannedInvest(100.);
    contReinvest.setConclusionDate(dateSecondContract);
    contReinvest.saveNewContract();

    stats expected_dateSecondContract_active =expected_startDate_active;
    QCOMPARE(getStatsActiveContracts( dateSecondContract), expected_startDate_active);

    stats expected_dateSecondContract_inactive {{interestModel::allIModels, statSet(2, 1, 200., 3., 1.5)}
                                                ,{interestModel::payout,   statSet(1, 1, 100., 1., 1.)}
                                                ,{interestModel::reinvest, statSet(1, 1, 100., 2., 2.)}
                                                ,{interestModel::fixed,    statSet(0, 0,   0., 0., 0.)}
                                                ,{interestModel::zero,     statSet(0, 0,   0., 0., 0.)}};
    QCOMPARE(getStatsInactiveContracts(dateSecondContract), expected_dateSecondContract_inactive);

    stats expected_dateSecondContract_all{{interestModel::allIModels,     statSet(2, 1, 200., 3., 1.5)}
                                          ,{interestModel::payout,        statSet(1, 1, 100., 1., 1.) }
                                          ,{interestModel::reinvest,      statSet(1, 1, 100., 2., 2.) }
                                          ,{interestModel::fixed,         statSet(0, 0,   0., 0., 0.) }
                                          ,{interestModel::zero,          statSet(0, 0,   0., 0., 0.) }};
    QCOMPARE(getStatsAllContracts( dateSecondContract), expected_dateSecondContract_all);

    /*
     * other creditor, fixed interest payout
     */
    QDate dateThirdContract =dateSecondContract.addDays(10);
    creditor creditor2 {saveRandomCreditor()};
    contract contFix;
    contFix.initContractDefaults(creditor2.id());
    contFix.setInterestModel(interestModel::fixed);
    contFix.setInterestRate(1.5);
    contFix.setPlannedInvest(100.);
    contFix.setConclusionDate(dateThirdContract);
    contFix.saveNewContract();

    stats expected_dateThirdContract_active =expected_dateSecondContract_active;
    QCOMPARE(getStatsActiveContracts( dateThirdContract), expected_dateThirdContract_active);

    stats expected_dateThirdContact_inactive {{interestModel::allIModels,   statSet(3, 2, 300., 4.5 , 1.5)}
                                            ,{interestModel::payout,        statSet(1, 1, 100., 1. , 1.)  }
                                            ,{interestModel::reinvest,      statSet(1, 1, 100., 2. , 2.)  }
                                            ,{interestModel::fixed,         statSet(1, 1, 100., 1.5, 1.5) }
                                            ,{interestModel::zero,          statSet(0, 0,   0., 0. , 0.)  }};
    QCOMPARE(getStatsInactiveContracts( dateThirdContract), expected_dateThirdContact_inactive);

    stats expected_dateThirdContract_all {{interestModel::allIModels,     statSet(3, 2, 300., 4.5, 1.5)}
                                          ,{interestModel::payout,        statSet(1, 1, 100., 1. , 1.)}
                                          ,{interestModel::reinvest,      statSet(1, 1, 100., 2. , 2.)}
                                          ,{interestModel::fixed,         statSet(1, 1, 100., 1.5, 1.5)}
                                          ,{interestModel::zero,          statSet(0, 0,   0., 0. , 0.)}};
    QCOMPARE(getStatsAllContracts( dateThirdContract), expected_dateThirdContract_all);

    /*
     * third creditor, - no interest
     */
    QDate dateForthContract =dateThirdContract.addDays(5);
    creditor creditor3 {saveRandomCreditor()};
    contract contZero;
    contZero.initContractDefaults(creditor3.id());
    contZero.setInterestModel(interestModel::zero);
    //contPayout.setInterestRate(2);
    contZero.setPlannedInvest(200.);
    contZero.setConclusionDate(dateForthContract);
    contZero.saveNewContract();

    stats expected_dateForthContract_active =expected_dateThirdContract_active;
    QCOMPARE(getStatsActiveContracts( dateForthContract), expected_dateForthContract_active);

    stats expected_dateForthContract_inactive {{interestModel::allIModels,     statSet(4, 3, 500., 4.5 , 0.9)}
                                               ,{interestModel::payout,        statSet(1, 1, 100., 1. , 1.)  }
                                               ,{interestModel::reinvest,      statSet(1, 1, 100., 2. , 2.)  }
                                               ,{interestModel::fixed,         statSet(1, 1, 100., 1.5, 1.5) }
                                               ,{interestModel::zero,          statSet(1, 1, 200., 0. , 0.)  }};
    QCOMPARE(getStatsInactiveContracts( dateForthContract),expected_dateForthContract_inactive);

    stats expected_dataForthContract_all  {{interestModel::allIModels,     statSet(4, 3, 500., 4.5 , 0.9)}
                                           ,{interestModel::payout,        statSet(1, 1, 100., 1. , 1.)  }
                                           ,{interestModel::reinvest,      statSet(1, 1, 100., 2. , 2.)  }
                                           ,{interestModel::fixed,         statSet(1, 1, 100., 1.5, 1.5) }
                                           ,{interestModel::zero,          statSet(1, 1, 200., 0. , 0.)  }};
    QCOMPARE(getStatsAllContracts( dateForthContract), expected_dataForthContract_all);

    /*
     * forth creditor, - reinvesting
     */
    QDate dateFifthContract =dateForthContract =QDate(2000, 6, 30);
    contPayout.bookInitialPayment(dateFifthContract, contPayout.plannedInvest());
    contReinvest.bookInitialPayment(dateFifthContract, contReinvest.plannedInvest());
    contFix.bookInitialPayment(dateFifthContract, contFix.plannedInvest());
    contZero.bookInitialPayment(dateFifthContract, contZero.plannedInvest());

    stats expected_dateFifthContract_active{{interestModel::allIModels,     statSet(4, 3, 500., 4.5 , 0.9)}
                                            ,{interestModel::payout,        statSet(1, 1, 100., 1. , 1.)  }
                                            ,{interestModel::reinvest,      statSet(1, 1, 100., 2. , 2.)  }
                                            ,{interestModel::fixed,         statSet(1, 1, 100., 1.5, 1.5) }
                                            ,{interestModel::zero,          statSet(1, 1, 200., 0. , 0.)  }};
    QCOMPARE(getStatsActiveContracts(dateFifthContract), expected_dateFifthContract_active);

    stats expected_dateFifthContract_inactive =expected_dateThirdContract_active;
    QCOMPARE(getStatsInactiveContracts(dateFifthContract), expected_dateFifthContract_inactive);

    stats expceted_dateFifthContract_all{{interestModel::allIModels,     statSet(4, 3, 500., 4.5 , 0.9)}
                                         ,{interestModel::payout,        statSet(1, 1, 100., 1. , 1.)  }
                                         ,{interestModel::reinvest,      statSet(1, 1, 100., 2. , 2.)  }
                                         ,{interestModel::fixed,         statSet(1, 1, 100., 1.5, 1.5) }
                                         ,{interestModel::zero,          statSet(1, 1, 200., 0. , 0.)  }};
    QCOMPARE(getStatsAllContracts(dateFifthContract), expceted_dateFifthContract_all);

    /*
     * Year end: everyone gets interest
     */

    contPayout.annualSettlement(2000);
    contReinvest.annualSettlement(2000);
    contFix.annualSettlement(2000);
    contZero.annualSettlement(2000);
    QDate date1stAnnualSettlement =QDate(2001, 1, 1);


    stats expected_date1stAnnualSettlement_active{{interestModel::allIModels,     statSet(4, 3, 501.75, 4.52 , 0.9)}
                                                  ,{interestModel::payout,        statSet(1, 1, 100., 1. , 1.)     }
                                                  ,{interestModel::reinvest,      statSet(1, 1, 101., 2.02 , 2)    }
                                                  ,{interestModel::fixed,         statSet(1, 1, 100.75, 1.5, 1.5)  }
                                                  ,{interestModel::zero,          statSet(1, 1, 200., 0. , 0.)     }};
    QCOMPARE(getStatsActiveContracts( date1stAnnualSettlement), expected_date1stAnnualSettlement_active);

    stats expected_date1stAnnualSettlement_inactive =expected_dateThirdContract_active;
    QCOMPARE(getStatsInactiveContracts( date1stAnnualSettlement), expected_date1stAnnualSettlement_inactive);

    stats expected_date1stAnnualSettlement_all =expected_date1stAnnualSettlement_active;
    QCOMPARE(getStatsAllContracts( date1stAnnualSettlement), expected_date1stAnnualSettlement_all);

    /*
     *  terminate one contract
     */
    QDate date1stTermination =date1stAnnualSettlement.addMonths(6);
    double interest =0., payout =0.;
    contPayout.finalize(false, date1stTermination, interest, payout);

    stats expected_date1stTermination_active{{interestModel::allIModels,     statSet(3, 3, 401.75, 3.52 , 0.88)}
                                             ,{interestModel::payout,        statSet(0, 0, 0., 0. , 0.)     }
                                             ,{interestModel::reinvest,      statSet(1, 1, 101., 2.02 , 2)    }
                                             ,{interestModel::fixed,         statSet(1, 1, 100.75, 1.5, 1.5)  }
                                             ,{interestModel::zero,          statSet(1, 1, 200., 0. , 0.)     }};
    QCOMPARE(getStatsActiveContracts(date1stTermination), expected_date1stTermination_active);

    stats expected_date1stTermination_inactive =expected_dateThirdContract_active;
    QCOMPARE(getStatsInactiveContracts(date1stTermination), expected_date1stTermination_inactive);

    stats expected_date1stTermination_all =expected_date1stTermination_active;
    QCOMPARE(getStatsAllContracts(date1stTermination), expected_date1stTermination_all);
}
