#include <QtTest>

#include "../DKV2/helper.h"
#include "../DKV2/helpersql.h"
#include "../DKV2/dkdbhelper.h"
#include "../DKV2/contract.h"
#include "../DKV2/creditor.h"
#include "test_creditor.h"

void test_creditor::initTestCase()
{   LOG_CALL;
    init_DKDBStruct();
}
void test_creditor::init()
{   LOG_CALL_W("test");
    initTestDb();
    fill_DkDbDefaultContent();
}
void test_creditor::cleanup()
{   LOG_CALL;
    cleanupTestDb();
}

void test_creditor::test_createCreditor()
{   LOG_CALL;
    creditor c;
    c.setFirstname("Holger");
    c.setLastname("Mairon");
    c.setStreet("Str.1");
    c.setPostalCode("68205");
    c.setCity("Mannheim");
    c.setEmail("holger.mairon@test.de");
    c.setComment("bester DK Geber");
    c.setIban("DE02120300000000202051");
    c.setBic("BYLADEM1001");
    QVERIFY2(0 <= c.save(), "creating creditor failed");
    QCOMPARE(tableRecordCount("Kreditoren"), 1);
}

void test_creditor::test_CreditorFromDb()
{   LOG_CALL;
    creditor c = saveRandomCreditor();
    QVERIFY2(c.isValid(), "randomly create creditor failed");

    creditor d(c.id());
    QCOMPARE( c, d);
}

void test_creditor::test_invalidCreditor()
{   LOG_CALL;
    creditor c;
    QString errortext;
    QVERIFY2( ! c.isValid(errortext), errortext.toUtf8());
    c.setFirstname("Holger");
    QVERIFY2( ! c.isValid(errortext), errortext.toUtf8());
    c.setLastname("Mairon");
    QVERIFY2( ! c.isValid(errortext), errortext.toUtf8());
    c.setStreet("Sesamstrasse");
    QVERIFY2( ! c.isValid(errortext), errortext.toUtf8());
    c.setPostalCode("49534");
    QVERIFY2( ! c.isValid(errortext), errortext.toUtf8());
    c.setCity("braunschweig"); // now all mandatory values are set
    QVERIFY2( c.isValid(errortext), errortext.toUtf8());
    c.setEmail("invalid_email");
    QVERIFY2( ! c.isValid(errortext), errortext.toUtf8());
    c.setEmail("holger@mairon.esp");
    QVERIFY2( c.isValid(errortext), errortext.toUtf8());
    c.setIban("invalid_iban");
    QVERIFY2( ! c.isValid(errortext), errortext.toUtf8());
    c.setIban("DE07123412341234123412");
    QVERIFY2( c.isValid(errortext), errortext.toUtf8());
}

void test_creditor::test_saveManyRandomCreditors()
{   LOG_CALL;
    dbgTimer t;
    int numberOfCreditors = 50;
    saveRandomCreditors(numberOfCreditors);
    //QVERIFY2(rowCount("Kreditoren") == numberOfCreditors, "random creditor creation failed");
    QCOMPARE(rowCount("Kreditoren"), numberOfCreditors);
}

void test_creditor::test_hasActiveContracts_noContracts()
{
    creditor c = saveRandomCreditor();
    QCOMPARE(c.hasActiveContracts(), false);
}

void test_creditor::test_hasActiveContracts_hasInactContract()
{
    creditor c = saveRandomCreditor();
    /*contract co = */saveRandomContract(c.id());
    QCOMPARE(c.hasActiveContracts(), false);
}

void test_creditor::test_hasActiveContracts_hasActContract()
{
    creditor c = saveRandomCreditor();
    contract co = saveRandomContract(c.id());
    co.activate(QDate::currentDate(), 1000.0);
    QCOMPARE(c.hasActiveContracts(), true);
}
void test_creditor::test_deleteCreditor_woContract()
{
    creditor c = saveRandomCreditor();
    QVERIFY(c.remove());
}

void test_creditor::test_deleteCreditor_wInactiveContract()
{
    creditor c = saveRandomCreditor();
    /*contract co = */saveRandomContract(c.id());
    QVERIFY(c.remove());
}

void test_creditor::test_deleteCredtior_wActiveContractFails()
{
    creditor c = saveRandomCreditor();
    contract co = saveRandomContract(c.id());
    co.activate(QDate::currentDate(), 1000.0);
    QCOMPARE(c.remove(), false);
}

void test_creditor::test_deleteCreditor_wTerminatedContractFails()
{
    creditor c = saveRandomCreditor();
    contract co = saveRandomContract(c.id());
    co.activate(QDate(2000, 6, 1), 1000);
    double interestPayout =0, payout =0.;
    co.finalize(false, QDate(2020, 5, 31), interestPayout, payout);
    QCOMPARE(c.remove(), false);
}
