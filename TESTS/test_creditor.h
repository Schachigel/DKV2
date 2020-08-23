#ifndef TEST_CREDITOR_H
#define TEST_CREDITOR_H


#include <QSqlDatabase>
#include <QObject>

#include "testhelper.h"

class test_creditor : public QObject
{
    Q_OBJECT
public:
    explicit test_creditor(QObject* p=nullptr) : QObject(p){}
    ~test_creditor(){}
private:
    // helper
signals:
private slots:
    void initTestCase();
//    void cleanupTestCase();
    void init();
    void cleanup();
    // the actual tests
    void test_createCreditor();
    void test_CreditorFromDb();
    void test_invalidCreditor();
    void test_saveManyRandomCreditors();
    void test_hasActiveContracts_noContracts();
    void test_hasActiveContracts_hasInactContract();
    void test_hasActiveContracts_hasActContract();
    void test_deleteCreditor_woContract();
    void test_deleteCreditor_wInactiveContract();
    void test_deleteCredtior_wActiveContractFails();
    void test_deleteCreditor_wTerminatedContractFails();
};

#endif // TEST_CREDITOR_H
