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

void test_views::test_statistics_activateContract()
{
    creditor c(saveRandomCreditor());
    contract cont;
    cont.initRandom(c.id());
    double v =1000.;
    cont.setPlannedInvest(v);
    double ir =3.3;
    cont.setInterestRate(ir);
    cont.setReinvesting(true);
    cont.saveNewContract();
    dbStatistics expected;
    expected.nbrCreditors =1;
    expected.nbrContracts =1;
    expected.nbrInactiveContracts =1;
    expected.nbrCreditors_inactiveContracts =1;
    expected.valueContracts =v;
    expected.weightedAvgInterestRate =ir;
    expected.avgInterestRate =ir;
    expected.annualInterest =v*ir/100;
    expected.valueInactiveContracts =v;
    expected.avgInterestInactiveContracts =ir;
    expected.weightedAvgInterestInactiveContracts =ir;
    expected.expectedAnnualInterestInactiveContrasts =v*ir/100;
    // passive contract
    QCOMPARE(expected, getStatistic());
    // active contract
    cont.activate(QDate::currentDate(), cont.plannedInvest());
    expected.nbrCreditors_inactiveContracts =0;
    expected.nbrInactiveContracts =0;
    expected.valueInactiveContracts =0.;
    expected.avgInterestInactiveContracts =0.;
    expected.weightedAvgInterestInactiveContracts =0.;
    expected.expectedAnnualInterestInactiveContrasts =0;
    expected.nbrCreditors_activeContracts =1;
    expected.nbrActiveContracts =1;
    expected.valueActiveContracts =v;
    expected.weightedAvgInterestActiveContracts = ir;
    expected.avgInterestActiveContracts = ir;
    expected.annualInterestActiveContracts = v *ir/100;
    expected.nbrActiveReinvesting =1;
    expected.valueActiveReinvesting =v;
    expected.annualInterestReinvestment = v *ir/100;
    QCOMPARE(expected, getStatistic());
}

void test_views::test_statistics_multipleContract()
{
    // contrat 1
    creditor c(saveRandomCreditor());
    contract cont;
    cont.initRandom(c.id());
    double v1 =1000.;
    cont.setPlannedInvest(v1);
    double ir1 =3.3;
    cont.setInterestRate(ir1);
    cont.setReinvesting(true);
    cont.saveNewContract();
    dbStatistics expected;
    expected.nbrCreditors =1;
    expected.nbrContracts =1;
    expected.nbrInactiveContracts =1;
    expected.nbrCreditors_inactiveContracts =1;
    expected.valueContracts =v1;
    expected.weightedAvgInterestRate =ir1;
    expected.avgInterestRate =ir1;
    expected.annualInterest =v1*ir1/100;
    expected.valueInactiveContracts =v1;
    expected.avgInterestInactiveContracts =ir1;
    expected.weightedAvgInterestInactiveContracts =ir1;
    expected.expectedAnnualInterestInactiveContrasts =v1*ir1/100;
    QCOMPARE(expected, getStatistic());

    // passive contract 2
    contract second;
    second.initRandom(c.id());
    second.setReinvesting(true);
    double v2 =2000.;
    second.setPlannedInvest(v2);
    expected.valueContracts +=v2;
    expected.valueInactiveContracts +=v2;
    double ir2 =1.66;
    second.setInterestRate(ir2);
    expected.avgInterestRate = (ir1 +ir2) /2;
    expected.avgInterestInactiveContracts = expected.avgInterestRate;
    expected.weightedAvgInterestRate = ((v1 *ir1) + (v2 *ir2)) /(v1 +v2);
    expected.weightedAvgInterestInactiveContracts = expected.weightedAvgInterestRate;
    expected.annualInterest = expected.weightedAvgInterestInactiveContracts *expected.valueInactiveContracts /100.;
    expected.expectedAnnualInterestInactiveContrasts = expected.annualInterest;
    second.setReinvesting(false);
    second.saveNewContract();
    expected.nbrContracts +=1;
    expected.nbrInactiveContracts +=1;
    QCOMPARE(expected, getStatistic());

    // passive contract 2
    contract third;
    third.initRandom(c.id());
    third.setReinvesting(false);
    double v3 =1000.;
    third.setPlannedInvest(v3);
    expected.valueContracts +=v3;
    expected.valueInactiveContracts +=v3;
    double ir3 =3.3;
    third.setInterestRate(ir3);
    expected.avgInterestRate = (ir1 +ir2 +ir3) /3;
    expected.avgInterestInactiveContracts = expected.avgInterestRate;
    expected.weightedAvgInterestRate = ((v1 *ir1) + (v2 *ir2) +(v3 *ir3)) /(v1 +v2 +v3);
    expected.weightedAvgInterestInactiveContracts = expected.weightedAvgInterestRate;
    expected.annualInterest = expected.weightedAvgInterestInactiveContracts *expected.valueInactiveContracts /100.;
    expected.expectedAnnualInterestInactiveContrasts = expected.annualInterest;
    third.setReinvesting(false);
    third.saveNewContract();
    expected.nbrContracts +=1;
    expected.nbrInactiveContracts +=1;
    QCOMPARE(expected, getStatistic());

    third.activate(third.conclusionDate().addYears(1), third.plannedInvest());
    second.activate(second.conclusionDate().addYears(1), second.plannedInvest());

    expected.nbrInactiveContracts  -=2;
    expected.nbrActiveContracts    +=2;
    expected.valueInactiveContracts =v1;
    expected.valueActiveContracts   =v2;
    expected.avgInterestActiveContracts =(v2 +v3) /2;
    expected.avgInterestInactiveContracts =ir1;
    expected.weightedAvgInterestActiveContracts =((v2 *ir2)+(v3 *ir3))/(v2 +v3);
    expected.weightedAvgInterestInactiveContracts =(v1 *ir1) /v1;
    expected.annualInterestActiveContracts =(v2 *ir2) + (v3 *ir3);
    expected.expectedAnnualInterestInactiveContrasts =v1*ir1;
    expected.annualInterestPayout =v2 *ir2;
    expected.annualInterestReinvestment =v3 *ir3;

}

void test_views::test_statistics_mny_contracts()
{
    {
        dbgTimer timer("create many contracts");
        saveRandomCreditors(44);
        saveRandomContracts(50);    // contract date: 2 years back or less
        activateRandomContracts(90);// activation date: > contract date
    }
    {
        dbgTimer timer("statistics");
        dbStatistics stats(true);
        qInfo().noquote() << stats.toString();
    }

}
