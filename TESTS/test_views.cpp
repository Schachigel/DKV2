#include <QtTest>

#include "../DKV2/creditor.h"
#include "../DKV2/contract.h"
#include "../DKV2/dbstatistics.h"

#include "test_views.h"

void test_views::initTestCase()
{
    init_DKDBStruct();
}
void test_views::cleanupTestCase()
{

}
void test_views::init()
{
    initTestDb();
    QVERIFY(create_DK_TablesAndContent());
}
void test_views::cleanup()
{
    cleanupTestDb();
}

void test_views::test_stat_activateContract_reinvesting()
{
    creditor c(saveRandomCreditor());
    contract cont;
    cont.initRandom(c.id());
    double v =1000.;
    cont.setPlannedInvest(v);
    double ir =3.3;
    cont.setInterestRate(ir);
    cont.setInterestModel(interestModel::reinvest);
    cont.saveNewContract();

    // passive contract
    dbStats expected;
    expected.addContract(v, ir, dbStats::payoutType::thesa, c.id());
    QCOMPARE(expected, getStatistic());
    // active contract
    cont.activate(QDate::currentDate(), cont.plannedInvest());
    // setup expected ...
    expected.activateContract(v, cont.plannedInvest(), ir, dbStats::payoutType::thesa, c.id());
    QCOMPARE(expected, getStatistic());
}

void test_views::test_stat_activateContract_wpayout()
{
    creditor c(saveRandomCreditor());
    contract cont;
    cont.initRandom(c.id());
    double v =1000.;
    cont.setPlannedInvest(v);
    double ir =3.3;
    cont.setInterestRate(ir);
    cont.setInterestModel(interestModel::payout);
    cont.saveNewContract();

    // passive contract
    dbStats expected;
    // setup expected ...
    expected.addContract(v, ir, dbStats::payoutType::pout, c.id());
    QCOMPARE(expected, getStatistic());
    // active contract
    cont.activate(QDate::currentDate(), cont.plannedInvest());
    // setup expected ...
    expected.activateContract(v, cont.plannedInvest(), ir, dbStats::payoutType::pout, c.id());
    QCOMPARE(expected, getStatistic());
}


void test_views::test_stat_create_activate_multipleContract()
{
    // contrat 1
    creditor c(saveRandomCreditor());
    contract first;
    first.initRandom(c.id());
    double v1 =1000.;
    first.setPlannedInvest(v1);
    double ir1 =3.3;
    first.setInterestRate(ir1);
    first.setInterestModel(interestModel::reinvest);
    first.saveNewContract();

    dbStats expected;
    expected.addContract(v1, ir1, dbStats::payoutType::thesa, c.id());
    QCOMPARE(expected, getStatistic());

    // passive contract 2
    contract second;
    second.initRandom(c.id());
    double v2 =2000.;
    second.setPlannedInvest(v2);
    //...todo: setup expected ...
    double ir2 =1.66;
    second.setInterestRate(ir2);
    second.setInterestModel(interestModel::reinvest);
    second.saveNewContract();

    expected.addContract(v2, ir2, dbStats::payoutType::thesa, c.id());
    QCOMPARE(expected, getStatistic());

    // passive contract 3
    contract third;
    third.initRandom(c.id());
    double v3 =1000.;
    third.setPlannedInvest(v3);
    //...todo: setup expected ...
    double ir3 =3.3;
    third.setInterestRate(ir3);
    //...todo: setup expected ...
    third.setInterestModel(interestModel::payout);
    third.saveNewContract();

    expected.addContract(v3, ir3, dbStats::payoutType::pout, c.id());
    QCOMPARE(expected, getStatistic());

    third.activate(third.conclusionDate().addYears(1), third.plannedInvest());
    second.activate(second.conclusionDate().addYears(1), second.plannedInvest());

    expected.activateContract( v3, v3, ir3, dbStats::payoutType::pout, c.id());
    expected.activateContract(v2, v2, ir2, dbStats::payoutType::thesa, c.id());
    QCOMPARE(expected, getStatistic());

    first.activate(first.conclusionDate().addYears(1), first.plannedInvest());

    expected.activateContract(v1, v1, ir1, dbStats::payoutType::thesa, c.id());
    QCOMPARE(expected, getStatistic());
}

void test_views::test_stat_annualSettelemnt()
{
    dbStats expected;
    QDate conclusionDate (2000, 1, 1),  activationDate(2000, 6, 30);
    /* have contracts and test statistics over an annual settlement
    *  having 5 contracts:
    *  1 inactive, 2 activ & reinvesting, 2 active & not reinvesting
    * */
    creditor inactiveCred(saveRandomCreditor());
    contract inactive; inactive.initRandom(inactiveCred.id());
    inactive.setPlannedInvest(5000.);
    inactive.saveNewContract();
    qInfo().noquote() << inactive.toString(qsl("Inactive Contract:")) << Qt::endl;
    expected =getStatistic();

    creditor activeCred1(saveRandomCreditor());
    contract activeReInv1; activeReInv1.initRandom(activeCred1.id());
    activeReInv1.setInterestModel(interestModel::reinvest);
    double v =1000, ir =0.5; // %
    activeReInv1.setPlannedInvest(v);
    activeReInv1.setInterestRate(ir);
    activeReInv1.setConclusionDate(conclusionDate);
    activeReInv1.saveNewContract();
    expected.addContract(v, ir, dbStats::payoutType::thesa, activeCred1.id());
    activeReInv1.activate(activationDate, activeReInv1.plannedInvest());
    expected.activateContract(v, v, ir, dbStats::payoutType::thesa,activeCred1.id());

    contract activeReInv2; activeReInv2.initRandom(activeCred1.id());
    activeReInv2.setInterestModel(interestModel::reinvest);
    activeReInv2.setPlannedInvest(2 *v);
    activeReInv2.setInterestRate(2 *ir);
    activeReInv2.setConclusionDate(conclusionDate);
    activeReInv2.saveNewContract();
    expected.addContract(2*v, 2*ir, dbStats::payoutType::thesa, activeCred1.id());
    activeReInv2.activate(activationDate, activeReInv2.plannedInvest());
    expected.activateContract(2*v, 2*v, 2*ir,dbStats::payoutType::thesa, activeCred1.id());

    creditor activeCred2(saveRandomCreditor());
    contract activeNonReIn1;activeNonReIn1.initRandom(activeCred2.id());
    activeNonReIn1.setInterestModel(interestModel::payout);
    activeNonReIn1.setPlannedInvest(3 *v);
    activeNonReIn1.setInterestRate(3 *ir);
    activeNonReIn1.setConclusionDate(conclusionDate);
    activeNonReIn1.saveNewContract();
    expected.addContract(3*v, 3*ir, dbStats::payoutType::pout, activeCred2.id());
    activeNonReIn1.activate(activationDate, activeNonReIn1.plannedInvest());
    expected.activateContract(3*v, 3*v, 3*ir, dbStats::payoutType::pout, activeCred2.id());

    contract activeNonReIn2;activeNonReIn2.initRandom(activeCred2.id());
    activeNonReIn2.setInterestModel(interestModel::payout);
    activeNonReIn2.setPlannedInvest(4 *v);
    activeNonReIn2.setInterestRate(4 *ir);
    activeNonReIn2.setConclusionDate(conclusionDate);
    activeNonReIn2.saveNewContract();
    expected.addContract(4*v, 4*ir, dbStats::payoutType::pout, activeCred2.id());
    activeNonReIn2.activate(activationDate, activeNonReIn2.plannedInvest());
    expected.activateContract(4*v, 4*v, 4*ir, dbStats::payoutType::pout, activeCred2.id());


//    /* values:
//     * v=1000
//    inactive: random
//    active, reinvesting: 1000-0.5%, 2000-1.0%
//    active, not reinv. : 3000-1.5%, 4000-2.0%
//    sum: 10 *v = 10000
//    avg: 1.25%, w avg 1.5%
//    */

    qInfo().noquote() << "pre settlement check. Expected Values: " << Qt::endl << expected.toString();
    QCOMPARE(expected, getStatistic());

    /*
     * run annual settlement and check statistics
     */
    activeReInv1.annualSettlement(activationDate.year());
    activeReInv2.annualSettlement(activationDate.year());
    activeNonReIn1.annualSettlement(activationDate.year());
    activeNonReIn2.annualSettlement(activationDate.year());
    qInfo().noquote() << activeReInv1.toString("Re1") << Qt::endl << activeReInv2.toString("Re2") << Qt::endl <<
        activeNonReIn1.toString("nRe1") << Qt::endl << activeNonReIn2.toString("nRe2");

    // setup expected ...
    expected.reinvest(v, ir, 180);
    expected.reinvest(2* v, 2* ir, 180);
    // no change with inactive contracts

    QCOMPARE(expected, getStatistic());
}

void test_views::test_stat_extend_contract_sameYear()
{
    QDate activationDate(2020, 3, 31);
    creditor c(saveRandomCreditor());
    contract cont;
    cont.initRandom(c.id());
    double v =1000.;
    cont.setPlannedInvest(v);
    double ir =3.3;
    cont.setInterestRate(ir);
    cont.setInterestModel(interestModel::payout);
    cont.setConclusionDate(activationDate.addMonths(1));
    cont.saveNewContract();

    // active contract
    cont.activate(activationDate, cont.plannedInvest());
    dbStats expected( dbStats::calculate);
    cont.deposit(activationDate.addDays(36), v);
    expected.changeContract(v, ir, 36, dbStats::payoutType::pout);

    QCOMPARE(expected, getStatistic());

}

//void test_views::test_stat_mny_contracts()
//{
//    {
//        dbgTimer timer("create many contracts");
//        saveRandomCreditors(44);
//        saveRandomContracts(50);    // contract date: 2 years back or less
//        activateRandomContracts(90);// activation date: > contract date
//        // 500 contracts < 1min
//    }
//    {
//        dbgTimer timer("statistics");
//        dbStats stats(true);
//        qInfo().noquote() << stats.toString();
//        // 500 contracts < 200ms
//    }

//}
