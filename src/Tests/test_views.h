#ifndef TEST_VIEWS_H
#define TEST_VIEWS_H

class test_views : public QObject
{
    Q_OBJECT
public:
    explicit test_views(QObject *parent = nullptr) : QObject(parent){}
    ~test_views(){}

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    void test_investmentOverview_includesDeletedContractsAndBookings();
    void test_getStatisticData_anchorsOnFirstPaymentElseContractDate();
    void test_investmentsOverview_fortlaufend_anchorsOnFirstPaymentElseContractDate();
    void test_interestByYearOverview_classifiesInterimInterestByContractMode();
    void test_shortInfo_overviews_keepDeferredMarkerNeutral();
    void test_perpetualInvestmentBookings_executesForOpenInvestment();
    void test_perpetualInvestmentBookings_keepsFinalizedContractsPositiveForOneYear();
    void test_perpetualInvestmentBookings_skipsZeroNetValueChangeDates();
    void test_perpetualInvestmentBookings_referenceCases_data();
    void test_perpetualInvestmentBookings_referenceCases();

//    void test_stat_activateContract_reinvesting();
//    void test_stat_activateContract_wpayout();

//    void test_stat_create_activate_multipleContract();
//    void test_stat_annualSettelemnt();
//    void test_stat_extend_contract_sameYear();

//    void test_stat_mny_contracts();
};

#endif // TEST_VIEWS_H
