#ifndef TEST_CONTRACT_H
#define TEST_CONTRACT_H

#include <QTest>

class test_contract : public QObject
{
    Q_OBJECT
public:
    explicit test_contract(QObject *parent = nullptr) : QObject(parent){}
private:
    // helper
signals:
private slots:
    void init();
    void cleanup();
    // the actual tests
    void test_createUninit_Contract();
    void test_set_get_interest();
    void test_activateContract();
    void test_readContractFromDb();
    void test_randomContract();
    void test_randomContracts();
    void test_write_read_contract();
    void deposit_inactive_contract_fails();
    void too_high_payout_fails();
    void unsequenced_bookings_fail();

    void test_annualSettlement_inactive_fails();
    void test_annualSettlement_fullYear();
    void test_annualSettlement_twoYear();
    void test_deposit01();
    void test_depositSwitches_31_12();
    void test_deposit_wSettlement_reinvesting();
    void test_deposit_wSettlement_wPayout();
    void test_payout();
    void test_payout_wSettlement_reinvesting();
    void test_payout_wSettlement_wPayout();

    void test_activationDate();
    void test_activate_interest_on_same_date_will_fail();
    //void test_latestSettlementDate();
    void test_getValue_byDate();
    void test_contract_cv_wInterestPayout();
    void test_contract_cv_reInvesting();
    void test_finalize();
    void test_readExContract();
};

#endif // TEST_CONTRACT_H
