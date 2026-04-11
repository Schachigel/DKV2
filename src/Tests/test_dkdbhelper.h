#ifndef TEST_DKDBHELPER_H
#define TEST_DKDBHELPER_H

class test_dkdbhelper : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void test_querySingleValueInvalidQuery();
    void test_querySingleValue();
    void test_querySingleValue_multipleResults();
    void test_nextContractLabelIndex_advancesOnSave();
    void test_nextContractLabelIndex_legacyGuessFromLabels();
    void test_nextContractLabelIndex_legacyGuessHonorsHigherStartIndex();
    void test_fillDkDbDefaultContent_createsZeitstempelTriggers();
    void test_postDbUpgradeActions_backfillsZeitstempelHistorically();

};

#endif // TEST_DKDBHELPER_H
