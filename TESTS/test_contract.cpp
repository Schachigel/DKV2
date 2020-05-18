#include <QtTest>

#include "../DKV2/helper.h"
#include "../DKV2/sqlhelper.h"
#include "../DKV2/dkdbhelper.h"
#include "../DKV2/creditor.h"
#include "../DKV2/contract.h"

#include "test_contract.h"

void test_contract::initTestCase()
{   LOG_CALL;
    init_DKDBStruct();
}
void test_contract::cleanupTestCase()
{   LOG_CALL;
}
void test_contract::init()
{   LOG_CALL;
    initTestDb();
    QVERIFY(create_DK_databaseContent());
}
void test_contract::cleanup()
{   LOG_CALL;
    cleanupTestDb();
}

void test_contract::test_createContract()
{   LOG_CALL;

}

void test_contract::test_randomContract()
{   LOG_CALL;
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));
    QCOMPARE(rowCount("Vertraege"), 1);
}

void test_contract::test_randomContracts()
{   LOG_CALL;
    int count = 90;
    dbgTimer t(QString::number(count) + " contracts");
    saveRandomCreditors(50);
    saveRandomContracts(count);
    QCOMPARE(rowCount("Vertraege"), count);
}
