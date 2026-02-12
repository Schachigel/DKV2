#include "test_annualsettlement.h"
#include "annualSettlement.h"

#include "creditor.h"
#include "contract.h"
#include "testhelper.h"
#include "filewriter.h"

#include <QtTest/QTest>

static const QDate ye_1998 {1998,12,31};
static const QDate ye_1999 {1999,12,31};
static const QDate ye_2000 {2000,12,31};
static const QDate ye_2001 {2001,12,31};
static const QDate ye_2002 {2002,12,31};
static const QDate ye_2003 {2003,12,31};
static const QDate ye_2004 {2004,12,31};
static const QDate ys_2000 {2000, 1, 1};


void test_annualSettlement::init()
{
    initTestDkDb_InMemory();
}

void test_annualSettlement::cleanup()
{
    cleanupTestDb_InMemory();
}

void test_annualSettlement::test_noContract_noAS()
{
    QCOMPARE(QDate(), dateOfnextSettlement());
}

void test_annualSettlement::test_oneContract_Mid_Year()
{
    // prep one contract
    contract c{saveRandomContract(saveRandomCreditor().id())};
    c.updateInterestActive(true);
    QDate cDate (2000, 6, 15);
    c.updateConclusionDate(cDate);

    // TEST: no AS
    QCOMPARE(QDate(), dateOfnextSettlement());
    QCOMPARE(executeCompleteAS(ye_2000.year()), 0);
    // init payment
    QVERIFY( c.bookInitialPayment(cDate.addDays(15), 1000));
    // TEST first AS
    QCOMPARE(ye_2000, dateOfnextSettlement());
    QCOMPARE(executeCompleteAS(ye_2000.year()), 1);
    // TEST second AS
    QCOMPARE(ye_2001, dateOfnextSettlement());
    QCOMPARE(executeCompleteAS(ye_2001.year()), 1);
}

void test_annualSettlement::test_contract_intrest_activation()
{
    // prep one contract
    contract c(saveRandomContract(saveRandomCreditor().id()));
    c.updateInterestActive(false);
    QDate cDate (2000, 6, 15);
    c.updateConclusionDate(cDate);

    // TEST: zActive false -> no AS
    QDate ye_2000(2000,12,31);
    QCOMPARE(QDate(), dateOfnextSettlement());
    QCOMPARE(executeCompleteAS(ye_2000.year()), 0); // no AS executed
    // interest activation fails w/o init payment
    QVERIFY(not c.bookActivateInterest(cDate.addDays(14)));
    // TEST: with initial payment
    QVERIFY(c.bookInitialPayment(cDate.addDays(14), 1000));
    // interest activation after initial payment
    QVERIFY(c.bookActivateInterest(cDate.addMonths(1)));
    QCOMPARE(ye_2000, dateOfnextSettlement());
    QCOMPARE(executeCompleteAS(ye_2000.year()), 1);
}

void test_annualSettlement::test_contract_intrest_activation_yearEnd()
{
    // prep one contract
    contract c(saveRandomContract(saveRandomCreditor().id()));
    c.updateInterestActive(false);
    QDate cDate (2000, 11, 15);
    c.updateConclusionDate(cDate);

    // init payment
    QDate yearEnd_afterConclusion(cDate.year(), 12, 31);
    QVERIFY(c.bookInitialPayment(cDate.addMonths(1), 1000));
    // act. interest on year end
    QVERIFY(c.bookActivateInterest(yearEnd_afterConclusion));
    // AS should work only in 2001 not in 2000
    QCOMPARE(dateOfnextSettlement(), ye_2001);
    QCOMPARE(executeCompleteAS(yearEnd_afterConclusion.year()), 0);
    QCOMPARE(executeCompleteAS(ye_2001.year()), 1);
}

void test_annualSettlement::test_dateOfNextSettlement_nextSettlement()
{
    // prep contract w AS until 2001
    contract c1(saveRandomContract(saveRandomCreditor().id()));
    c1.updateInterestActive(true);
    c1.updateConclusionDate(QDate(2000, 5, 1));

    QCOMPARE(dateOfnextSettlement(), QDate());
    QVERIFY(c1.bookInitialPayment(QDate(2000, 6, 1), 1000.));
    QCOMPARE(dateOfnextSettlement(), c1.dateOf_next_AS()); // contract local vs. global
    QCOMPARE(c1.dateOf_next_AS(), ye_2000);
    QCOMPARE(executeCompleteAS(2000), 1);
    QCOMPARE(dateOfnextSettlement(), c1.dateOf_next_AS()); // contract local vs. global
    QCOMPARE(c1.dateOf_next_AS(), ye_2001);
    QCOMPARE(executeCompleteAS(2001), 1);
    QCOMPARE(dateOfnextSettlement(), c1.dateOf_next_AS()); // contract local vs. global
    // prep contract earlier
    contract c2(saveRandomContract(c1.credId()));
    c2.updateInterestActive(true);
    c2.updateConclusionDate(QDate(2001, 5, 1));
    QVERIFY(c2.bookInitialPayment(QDate(2001, 6, 1), 1000.));
    QCOMPARE(dateOfnextSettlement(), c2.dateOf_next_AS()); // contract local vs. global
    QCOMPARE(executeCompleteAS(2000), 0);
    QCOMPARE(executeCompleteAS(2001), 1);
    QCOMPARE(dateOfnextSettlement(), c1.dateOf_next_AS()); // contract local vs. global
    QCOMPARE(executeCompleteAS(2002), 2);
}

void test_annualSettlement::test_dateOfNextSettlement_activatedContracts()
{
    contract c1(saveRandomContract(saveRandomCreditor().id()));
    c1.updateInterestActive(true);
    c1.updateConclusionDate(QDate(2000, 11, 30));
    QVERIFY(c1.bookInitialPayment(ye_2000, 1000.));
    // test
    QCOMPARE(dateOfnextSettlement(), c1.dateOf_next_AS());
    QCOMPARE(dateOfnextSettlement(), ye_2001);

    contract c2(saveRandomContract(c1.credId()));
    c2.updateInterestActive(true);
    c2.updateConclusionDate(QDate(1999, 11, 30));
    QVERIFY(c2.bookInitialPayment(ys_2000, 1000.));
    // test
    QCOMPARE(c1.dateOf_next_AS(), ye_2001); // unchanged
    QCOMPARE(c2.dateOf_next_AS(), ye_2000);
    QCOMPARE(dateOfnextSettlement(), ye_2000);

    contract c3(saveRandomContract(c1.credId()));
    c3.updateInterestActive(true);
    c3.updateConclusionDate(QDate(1998, 11, 30));
    QVERIFY(c3.bookInitialPayment(ye_1998, 1000.));
    QCOMPARE(c1.dateOf_next_AS(), ye_2001); // unchanged
    QCOMPARE(c2.dateOf_next_AS(), ye_2000); // unchanged
    QCOMPARE(c3.dateOf_next_AS(), ye_1999);
    QCOMPARE(dateOfnextSettlement(), ye_1999);

    contract c4(saveRandomContract(c1.credId()));
    c4.updateInterestActive(true);
    c4.updateConclusionDate(QDate(1998, 4, 2));
    QCOMPARE(c1.dateOf_next_AS(), ye_2001); // unchanged
    QCOMPARE(c2.dateOf_next_AS(), ye_2000); // unchanged
    QCOMPARE(c3.dateOf_next_AS(), ye_1999); // unchanged
    QCOMPARE(c4.dateOf_next_AS(), EndOfTheFuckingWorld);
    QCOMPARE(dateOfnextSettlement(), ye_1999);

    QVERIFY(c4.bookInitialPayment(QDate(1998, 4, 3), 1000.));
    QCOMPARE(c1.dateOf_next_AS(), ye_2001); // unchanged
    QCOMPARE(c2.dateOf_next_AS(), ye_2000); // unchanged
    QCOMPARE(c3.dateOf_next_AS(), ye_1999); // unchanged
    QCOMPARE(c4.dateOf_next_AS(), ye_1998);
    QCOMPARE(dateOfnextSettlement(), ye_1998);
}

void test_annualSettlement::test_dateOfNextSettlement_mixedStates_earliestWins()
{
    contract progressed(saveRandomContract(saveRandomCreditor().id()));
    progressed.updateInterestActive(true);
    progressed.updateConclusionDate(QDate(2000, 5, 15));
    QVERIFY(progressed.bookInitialPayment(QDate(2000, 6, 1), 1000.));
    QCOMPARE(executeCompleteAS(2000), 1);

    contract newer(saveRandomContract(progressed.credId()));
    newer.updateInterestActive(true);
    newer.updateConclusionDate(QDate(2000, 5, 30));
    QVERIFY(newer.bookInitialPayment(QDate(2000, 6, 1), 1000.));

    QCOMPARE(progressed.dateOf_next_AS(), ye_2001);
    QCOMPARE(newer.dateOf_next_AS(), ye_2000);
    QCOMPARE(dateOfnextSettlement(), newer.dateOf_next_AS());
    QCOMPARE(dateOfnextSettlement(), ye_2000);
}

void test_annualSettlement::test_dateOfNextSettlement_mixedStates_laterContractIgnored()
{
    contract progressed(saveRandomContract(saveRandomCreditor().id()));
    progressed.updateInterestActive(true);
    progressed.updateConclusionDate(QDate(2000, 5, 4));
    QVERIFY(progressed.bookInitialPayment(QDate(2000, 6, 1), 1000.));
    QCOMPARE(executeCompleteAS(2000), 1);

    contract later(saveRandomContract(progressed.credId()));
    later.updateInterestActive(true);
    later.updateConclusionDate(QDate(2002, 5, 5));
    QVERIFY(later.bookInitialPayment(QDate(2002, 6, 1), 1000.));

    QCOMPARE(progressed.dateOf_next_AS(), ye_2001);
    QCOMPARE(later.dateOf_next_AS(), ye_2002);
    QCOMPARE(dateOfnextSettlement(), progressed.dateOf_next_AS());
    QCOMPARE(dateOfnextSettlement(), ye_2001);
}

void test_annualSettlement::test_dateOfNextSettlement_mixedStates_deterministic()
{
    contract noActivation(saveRandomContract(saveRandomCreditor().id()));
    noActivation.updateInterestActive(true);
    noActivation.updateConclusionDate(QDate(2003, 4, 5));

    contract activatedLate(saveRandomContract(noActivation.credId()));
    activatedLate.updateInterestActive(true);
    activatedLate.updateConclusionDate(QDate(2003, 5, 5));
    QVERIFY(activatedLate.bookInitialPayment(ye_2003, 1000.));

    contract alreadySettled(saveRandomContract(noActivation.credId()));
    alreadySettled.updateInterestActive(true);
    alreadySettled.updateConclusionDate(QDate(2002, 5, 5));
    QVERIFY(alreadySettled.bookInitialPayment(QDate(2002, 6, 1), 1000.));
    QCOMPARE(executeCompleteAS(2002), 1);
    QCOMPARE(executeCompleteAS(2003), 1);

    // noActivation has no deposit and must be ignored.
    QCOMPARE(noActivation.dateOf_next_AS(), EndOfTheFuckingWorld);
    QCOMPARE(activatedLate.dateOf_next_AS(), ye_2004);
    QCOMPARE(alreadySettled.dateOf_next_AS(), ye_2004);
    QCOMPARE(dateOfnextSettlement(), activatedLate.dateOf_next_AS());
    QCOMPARE(dateOfnextSettlement(), ye_2004);
}

void test_annualSettlement::test_multipleContracts()
{
    // prep creditors and contracts
    saveRandomCreditor();
    saveRandomCreditor();
    struct test_contract {
        tableindex_t creditor;
        double amount;
        double interestRate;
        interestModel iModel;
        bool interestActive;
        QDate conclusionDate;
    };

    QVector<test_contract>  contractData {
        {1, 1000., 10., interestModel::reinvest, true,  QDate(2000, 1,  1)},
        {2, 2000.,  1., interestModel::payout,   false, QDate(2000, 2,  1)},
        {1, 3000.,  5., interestModel::fixed,    true,  QDate(2001, 1, 22)},
        {2, 1500., 10., interestModel::zero,     false, QDate(2001, 4,  5)},
        {1, 2500.,  1., interestModel::reinvest, true,  QDate(2002, 3, 15)},
        {2, 3500.,  5., interestModel::payout,   false, QDate(2002, 6, 22)},
        {1, 3000.,  5., interestModel::fixed,    true,  QDate(2003, 3,  5)},
        {2, 1000., 10., interestModel::zero,     false, QDate(2003, 8,  7)},
        };

    int i {0};
    for (const auto& dataset : contractData) {
        contract c1;
        c1.setCreditorId(creditorId_t{dataset.creditor});
        c1.setInterestRate(dataset.interestRate);
        c1.setInterestActive(dataset.interestActive);
        c1.setInterestModel(dataset.iModel);
        c1.setConclusionDate(dataset.conclusionDate);
        c1.setLabel(qsl("Vertrag #%1").arg(QString::number(i++)));
        c1.saveNewContract();
    }
}

void test_annualSettlement::test_csvCreation_fails_with_no_AS()
{
    QVERIFY( formulate_AS_as_CSV(1999).isEmpty());
}

void test_annualSettlement::test_csvCreation_check_headers()
{
    // prep one contract w initial payment and AS
    QDate conclusionDate{2001,12,19};
    QDate initialPaymentDate{2001,12,31};
    creditor ctor;
    ctor.setFirstname(  qsl("first name"));
    ctor.setLastname(   qsl("lastname_ÄÖÜ"));
    ctor.setStreet(     qsl("Streetname  number 44"));
    ctor.setPostalCode( qsl("D-123456"));
    ctor.setCity(       qsl("San Franzis KO -CA"));
    ctor.setEmail(      qsl("ganzlangeemailadresse@mailprovider.uk.co"));
    ctor.setAccount(    qsl("0815/42 Aktenzeichen xy ungelöst"));
    ctor.setIban(       qsl("DE03 2345 2345 2345 234 E33"));
    ctor.setBic(        qsl("Bicß0394586"));
    contract cont;
    cont.setCreditorId( ctor.save());
    cont.setLabel(         qsl("ESP-DK-2022-1029456"));
    cont.setConclusionDate(conclusionDate);
    cont.setInterestActive(true);
    cont.setNoticePeriod(  6/*month*/);
    cont.setInterestModel(interestModel::reinvest);
    cont.setInterestRate(10.);
    cont.saveNewContract();
    QVERIFY( cont.bookInitialPayment(initialPaymentDate, 1000));
    QCOMPARE(executeCompleteAS(ye_2002.year()), 1);
    // TEST csv
    QString csv =formulate_AS_as_CSV(2002);
    QVERIFY(not csv.isEmpty());
//     saveStringToUtf8File(qsl("test_csvCreation_check_headers.%1").arg("csv"),  csv);

    const QStringList lines = csv.replace(qsl("\r\n"), qsl("\n")).split('\n', Qt::KeepEmptyParts);
    QCOMPARE(lines.size(), 2); // header + one data line

    const QStringList expectedHeaders {
        qsl("Vorname"),
        qsl("Nachname"),
        qsl("Strasse"),
        qsl("Plz"),
        qsl("Stadt"),
        qsl("Email"),
        qsl("Buchungskonto"),
        qsl("IBAN"),
        qsl("BIC"),
        qsl("Kennung"),
        qsl("Auszahlungsart"),
        qsl("Beginn"),
        qsl("Buchungsdatum"),
        qsl("Kreditbetrag"),
        qsl("Zinssatz"),
        qsl("Zins"),
        qsl("Endbetrag")
    };
    QCOMPARE(lines[0].split(';', Qt::KeepEmptyParts), expectedHeaders);

    const QStringList row = lines[1].split(';', Qt::KeepEmptyParts);
    QCOMPARE(row.size(), expectedHeaders.size());

    const int idxVorname      = expectedHeaders.indexOf(qsl("Vorname"));
    const int idxNachname     = expectedHeaders.indexOf(qsl("Nachname"));
    const int idxKennung      = expectedHeaders.indexOf(qsl("Kennung"));
    const int idxAuszahlungs  = expectedHeaders.indexOf(qsl("Auszahlungsart"));
    const int idxBuchungsdate = expectedHeaders.indexOf(qsl("Buchungsdatum"));
    const int idxBeginn       = expectedHeaders.indexOf(qsl("Beginn"));

    QCOMPARE(row[idxVorname], qsl("first name"));
    QCOMPARE(row[idxNachname], qsl("lastname_ÄÖÜ"));
    QCOMPARE(row[idxKennung], qsl("ESP-DK-2022-1029456"));
    QCOMPARE(row[idxAuszahlungs], qsl("ansparend"));
    QCOMPARE(row[idxBeginn], initialPaymentDate.toString(Qt::ISODate));
    QCOMPARE(row[idxBuchungsdate], ye_2002.toString(Qt::ISODate));
}
