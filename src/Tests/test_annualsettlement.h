#ifndef TEST_ANNUALSETTLEMENT_H
#define TEST_ANNUALSETTLEMENT_H

#include <QObject>

class test_annualSettlement : public QObject
{
    Q_OBJECT

private slots:
    void init(); // for each test
    void cleanup();

    void test_noContract_noAS();
    void test_oneContract_Mid_Year();
    void test_contract_intrest_activation();
    void test_contract_intrest_activation_yearEnd();
    void test_dateOfNextSettlement_nextSettlement();
    void test_dateOfNextSettlement_activatedContracts();
    void test_dateOfNextSettlement_mixedStates_earliestWins();
    void test_dateOfNextSettlement_mixedStates_laterContractIgnored();
    void test_dateOfNextSettlement_mixedStates_deterministic();
    void test_multipleContracts();
    void test_csvCreation_fails_with_no_AS();
    void test_csvCreation_check_headers();
};

#endif // TEST_ANNUALSETTLEMENT_H
