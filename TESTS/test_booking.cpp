#include "../DKV2/helper.h"
#include "../DKV2/sqlhelper.h"
#include "../DKV2/dkdbhelper.h"

#include "test_booking.h"

void test_booking::initTestCase()
{   LOG_CALL;
    init_DKDBStruct();
}
void test_booking::cleanupTestCase()
{    LOG_CALL;
}
void test_booking::init()
{   LOG_CALL;
    initTestCase();
    create_DK_databaseContent();
}
void test_booking::cleanup()
{   LOG_CALL;
    cleanupTestDb();
}

void test_booking::test_createCreditor()
{   LOG_CALL;

}
