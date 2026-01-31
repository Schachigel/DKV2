#include "test_creditor.h"

#include "../DKV2/helper_core.h"
#include "../DKV2/helpersql.h"
#include "../DKV2/creditor.h"
#include "../DKV2/contract.h"

#include "testhelper.h"

#include <QtTest/QTest>

void test_creditor::init()
{
    initTestDkDb_InMemory();
}

void test_creditor::cleanup()
{   LOG_CALL;
    cleanupTestDb_InMemory ();
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
    c.setTel("+49 123 56789012");
    c.setContact("Holger selbst");
    c.setComment("bester DK Geber");
    c.setIban("DE02120300000000202051");
    c.setBic("BYLADEM1001");
    c.setAccount("80123");
    QVERIFY2(0 <= c.save(), "creating creditor failed");
    QCOMPARE(rowCount("Kreditoren"), 1);
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
    QVERIFY2( not c.isValid(errortext), errortext.toUtf8());
    c.setFirstname(qsl("Holger"));
    QVERIFY2( not c.isValid(errortext), errortext.toUtf8());
    c.setLastname(qsl("Mairon"));
    QVERIFY2( not c.isValid(errortext), errortext.toUtf8());
    c.setStreet(qsl("Sesamstrasse"));
    QVERIFY2( not c.isValid(errortext), errortext.toUtf8());
    c.setPostalCode(qsl("49534"));
    QVERIFY2( not c.isValid(errortext), errortext.toUtf8());
    c.setCity(qsl("braunschweig")); // now all mandatory values are set
    QVERIFY2( c.isValid(errortext), errortext.toUtf8());
    c.setEmail(qsl("invalid_email"));
    QVERIFY2( not c.isValid(errortext), errortext.toUtf8());
    c.setEmail(qsl("holger@mairon.esp"));
    QVERIFY2( c.isValid(errortext), errortext.toUtf8());
    c.setIban(qsl("invalid_iban"));
    QVERIFY2( not c.isValid(errortext), errortext.toUtf8());
    c.setIban(qsl("DE07123412341234123412"));
    QVERIFY2( c.isValid(errortext), errortext.toUtf8());
}

void test_creditor::test_saveManyRandomCreditors()
{   LOG_CALL;
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
    co.bookInitialPayment(QDate::currentDate(), 1000.0);
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
    co.bookInitialPayment(QDate::currentDate(), 1000.0);
    QCOMPARE(c.remove(), false);
}

void test_creditor::test_deleteCreditor_wTerminatedContractFails()
{
    creditor c = saveRandomCreditor();
    contract co = saveRandomContract(c.id());
    co.bookInitialPayment(QDate(2000, 6, 1), 1000);
    double interestPayout =0, payout =0.;
    co.finalize(false, QDate(2020, 5, 31), interestPayout, payout);
    QCOMPARE(c.remove(), false);
}

void test_creditor::test_getAllCreditorInfoSorted ()
{
    int numberOfCreditors =2;
    saveRandomCreditors (numberOfCreditors);
    QList<QPair<qlonglong, QString>> allCreditorsWithNames;
    // function under test
    getAllCreditorInfoSorted (allCreditorsWithNames);
    // test: all creditors
    QCOMPARE( allCreditorsWithNames.size (), numberOfCreditors);

    QList<qlonglong> creditorIds;
    int yearOfAnnualSettlement =QDate::currentDate ().year () +2;
    // function under test
    creditorsWithAnnualSettlement(creditorIds, yearOfAnnualSettlement);
    // test no creditors
    QCOMPARE( creditorIds.size (), 0);


    saveRandomContractPerCreditor();
    activateAllContracts (yearOfAnnualSettlement);
    // function under test
    creditorsWithAnnualSettlement(creditorIds, yearOfAnnualSettlement);
    // test some creditors
    QCOMPARE( creditorIds.size (), numberOfCreditors);
}
