#include <QtTest>

#include "../DKV2/helper.h"
#include "../DKV2/sqlhelper.h"
#include "../DKV2/dkdbhelper.h"

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
