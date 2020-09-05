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
    second.setReinvesting(true);
    second.saveNewContract();
    expected.nbrContracts +=1;
    expected.nbrInactiveContracts +=1;
    QCOMPARE(expected, getStatistic());

    // passive contract 3
    contract third;
    third.initRandom(c.id());
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

    expected.nbrCreditors_activeContracts +=1;
    expected.nbrInactiveContracts  -=2;
    expected.nbrActiveContracts    +=2;
    expected.nbrActiveReinvesting  +=1;
    expected.nbrActiveNotReinvesting +=1;
    expected.valueInactiveContracts =v1;
    expected.valueActiveContracts   =v2 +v3;
    expected.valueActiveReinvesting =v2;
    expected.valueActiveNotReinvesting =v3;
    expected.avgInterestActiveContracts =(ir2 +ir3) /2;
    expected.avgInterestInactiveContracts =ir1;
    expected.weightedAvgInterestActiveContracts =((v2 *ir2) +(v3 *ir3))/(v2 +v3);
    expected.weightedAvgInterestInactiveContracts =(v1 *ir1) /v1;
    expected.annualInterestActiveContracts =((v2 *ir2) +(v3 *ir3)) /100.;
    expected.expectedAnnualInterestInactiveContrasts =v1*ir1 /100.;
    expected.annualInterestPayout =v3 *ir3 /100.;
    expected.annualInterestReinvestment =v2 *ir2 /100.;
    QCOMPARE(expected, getStatistic());

    cont.activate(cont.conclusionDate().addYears(1), cont.plannedInvest());
    expected.nbrCreditors_inactiveContracts -=1;
    expected.nbrInactiveContracts  -=1;
    expected.nbrActiveContracts    +=1;
    expected.nbrActiveReinvesting +=1;
    expected.valueInactiveContracts =0.;
    expected.valueActiveContracts   =v1 +v2 +v3;
    expected.valueActiveReinvesting =v1 +v2;
    expected.avgInterestActiveContracts =(ir1 +ir2 +ir3) /3;
    expected.avgInterestInactiveContracts =0.;
    expected.weightedAvgInterestActiveContracts =((v1 *ir1) +(v2 *ir2) +(v3 *ir3))/(v1 +v2 +v3);
    expected.weightedAvgInterestInactiveContracts =0.;
    expected.annualInterestActiveContracts =((v1 *ir1) +(v2 *ir2) +(v3 *ir3)) /100.;
    expected.expectedAnnualInterestInactiveContrasts =0.;
    expected.annualInterestReinvestment =((v1 *ir1) + (v2 *ir2)) /100.;
    QCOMPARE(expected, getStatistic());

}

void test_views::test_statistics_annualSettelemnt()
{
    dbStatistics expected;
    QDate conclusionDate (2000, 1, 1),  activationDate(2000, 7, 1);
    /* have contracts and test statistics over an annual settlement
    *  having 5 contracts:
    *  1 inactive, 2 activ & reinvesting, 2 active & not reinvesting
    * */
    creditor inactiveCred(saveRandomCreditor());
    contract inactive; inactive.initRandom(inactiveCred.id());
    inactive.saveNewContract();
    qInfo().noquote() << inactive.toString(qsl("inactive")) << Qt::endl;
    expected =getStatistic();

    creditor activeCred1(saveRandomCreditor());
    contract activeReInv1; activeReInv1.initRandom(activeCred1.id());
    activeReInv1.setReinvesting(true);
    double v =1000, ir =0.5; // %
    activeReInv1.setPlannedInvest(v);
    activeReInv1.setInterestRate(ir);
    activeReInv1.setConclusionDate(conclusionDate);
    activeReInv1.saveNewContract();
    activeReInv1.activate(activationDate, activeReInv1.plannedInvest());

    contract activeReInv2; activeReInv2.initRandom(activeCred1.id());
    activeReInv2.setReinvesting(true);
    activeReInv2.setPlannedInvest(2 *v);
    activeReInv2.setInterestRate(2 *ir);
    activeReInv2.setConclusionDate(conclusionDate);
    activeReInv2.saveNewContract();
    activeReInv2.activate(activationDate, activeReInv2.plannedInvest());

    creditor activeCred2(saveRandomCreditor());
    contract activeNonReIn1;activeNonReIn1.initRandom(activeCred2.id());
    activeNonReIn1.setReinvesting(false);
    activeNonReIn1.setPlannedInvest(3 *v);
    activeNonReIn1.setInterestRate(3 *ir);
    activeNonReIn1.setConclusionDate(conclusionDate);
    activeNonReIn1.saveNewContract();
    activeNonReIn1.activate(activationDate, activeNonReIn1.plannedInvest());

    contract activeNonReIn2;activeNonReIn2.initRandom(activeCred2.id());
    activeNonReIn2.setReinvesting(false);
    activeNonReIn2.setPlannedInvest(4 *v);
    activeNonReIn2.setInterestRate(4 *ir);
    activeNonReIn2.setConclusionDate(conclusionDate);
    activeNonReIn2.saveNewContract();
    activeNonReIn2.activate(activationDate, activeNonReIn2.plannedInvest());

    /* values:
     * v=1000
    inactive: random
    active, reinvesting: 1000-0.5%, 2000-1.0%
    active, not reinv. : 3000-1.5%, 4000-2.0%
    sum: 10 *v = 10000
    avg: 1.25%, w avg 1.5%
    */
    // update expectations from activations
    expected.nbrCreditors +=2;
    expected.nbrContracts +=4;
    expected.valueContracts +=10000.;
    expected.weightedAvgInterestRate =(
          (inactive.plannedInvest()*inactive.interestRate())
          +(activeReInv1.value() *activeReInv1.interestRate())
          +(activeReInv2.value() *activeReInv2.interestRate())
          +(activeNonReIn1.value() *activeNonReIn1.interestRate())
          +(activeNonReIn2.value() *activeNonReIn2.interestRate()))
       /(inactive.plannedInvest() +activeReInv1.value() +activeReInv2.value()
         +activeNonReIn1.value() +activeNonReIn2.value());
    expected.avgInterestRate =(inactive.interestRate() +(10 *ir)) /5;
    expected.annualInterest =(expected.valueContracts *expected.weightedAvgInterestRate) /100.;
    expected.nbrCreditors_activeContracts +=2;

    expected.nbrActiveContracts +=4;
    expected.valueActiveContracts += 10 *v;
    expected.weightedAvgInterestActiveContracts =(
          (activeReInv1.value() *activeReInv1.interestRate())
          +(activeReInv2.value() *activeReInv2.interestRate())
          +(activeNonReIn1.value() *activeNonReIn1.interestRate())
          +(activeNonReIn2.value() *activeNonReIn2.interestRate()))
               /(10 *v);
    expected.avgInterestActiveContracts = (ir +2*ir +3*ir +4*ir) /4.;
    expected.annualInterestActiveContracts =expected.valueActiveContracts *expected.weightedAvgInterestActiveContracts /100.;

    expected.nbrActiveReinvesting +=2;
    expected.valueActiveReinvesting +=3*v;
    expected.annualInterestReinvestment = ((activeReInv1.value() *activeReInv1.interestRate()) +(activeReInv2.value() *activeReInv2.interestRate())) /100.;

    expected.nbrActiveNotReinvesting +=2;
    expected.valueActiveNotReinvesting =(activeNonReIn1.value() + activeNonReIn2.value());
    expected.annualInterestPayout = ((activeNonReIn1.value() *activeNonReIn1.interestRate()) + (activeNonReIn2.value() * activeNonReIn2.interestRate())) /100.;
    qInfo().noquote() << "pre settlement check. Expected Values: " << Qt::endl << expected.toString();
    QCOMPARE(expected, getStatistic());

    /*
     * run annual settlement and check statistics
     */
    activeReInv1.annualSettlement(activationDate.year());
    activeReInv2.annualSettlement(activationDate.year());
    activeNonReIn1.annualSettlement(activationDate.year());
    activeNonReIn2.annualSettlement(activationDate.year());

    contract activeReInv1_s(activeReInv1.id());
    contract activeReInv2_s(activeReInv2.id());
    contract activeNonReIn1_s(activeNonReIn1.id());
    contract activeNonReIn2_s(activeNonReIn2.id());

    qInfo().noquote() << activeReInv1_s.toString("Re1") << Qt::endl << activeReInv2_s.toString("Re2") << Qt::endl <<
        activeNonReIn1_s.toString("nRe1") << Qt::endl << activeNonReIn2_s.toString("nRe2");

    expected.valueActiveContracts = activeReInv1_s.value() +activeReInv2_s.value() +activeNonReIn1_s.value() +activeNonReIn2_s.value();
    expected.weightedAvgInterestActiveContracts =(  activeReInv1_s.value() *activeReInv1_s.interestRate()
                                                   +activeReInv2_s.value() *activeReInv2_s.interestRate()
                                                   +activeNonReIn1_s.value() *activeNonReIn1_s.interestRate()
                                                   +activeNonReIn2_s.value() *activeNonReIn2_s.interestRate())
                                                  /(activeReInv1_s.value()
                                                     +activeReInv2_s.value()
                                                     +activeNonReIn1_s.value()
                                                     +activeNonReIn2_s.value());
    expected.annualInterestActiveContracts =(activeReInv1_s.value() +activeReInv2_s.value() +activeNonReIn1_s.value() +activeNonReIn2_s.value())
                                             *expected.weightedAvgInterestActiveContracts /100.;

    // no change with inactive contracts

    expected.valueContracts += (v *activeReInv1_s.interestRate() + 2 *v *activeReInv2_s.interestRate()) /2. /100.;
    expected.weightedAvgInterestRate =( inactive.plannedInvest() *inactive.interestRate()
                                        +activeReInv1_s.value() *activeReInv1_s.interestRate()
                                        +activeReInv2_s.value() *activeReInv2_s.interestRate()
                                        +activeNonReIn1_s.value() *activeNonReIn1_s.interestRate()
                                        +activeNonReIn2_s.value() *activeNonReIn2_s.interestRate())
                                       /(inactive.plannedInvest()
                                          +activeReInv1_s.value()
                                          +activeReInv2_s.value()
                                          +activeNonReIn1_s.value()
                                          +activeNonReIn2_s.value());
    expected.annualInterest =(expected.valueContracts *expected.weightedAvgInterestRate) /100.;

    expected.valueActiveReinvesting =activeReInv1_s.value() +activeReInv2_s.value();
    expected.annualInterestReinvestment =(activeReInv1_s.value() *activeReInv1_s.interestRate() +activeReInv2_s.value() *activeReInv2_s.interestRate()) /100.;

    qInfo().noquote() << "post settlement check. Expected Values: " << Qt::endl << expected.toString();
    QCOMPARE(expected, getStatistic());

}

void test_views::test_statistics_mny_contracts()
{
    {
        dbgTimer timer("create many contracts");
        saveRandomCreditors(44);
        saveRandomContracts(50);    // contract date: 2 years back or less
        activateRandomContracts(90);// activation date: > contract date
        // 500 contracts < 1min
    }
    {
        dbgTimer timer("statistics");
        dbStatistics stats(true);
        qInfo().noquote() << stats.toString();
        // 500 contracts < 200ms
    }

}
