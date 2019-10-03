#include <QtTest>
#include <QCoreApplication>

#include "..\DKV2\dkdbhelper.h"

// add necessary includes here

class testRefInt : public QObject
{
    Q_OBJECT

public:
    testRefInt();
    ~testRefInt();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_case1();

};

testRefInt::testRefInt()
{

}

testRefInt::~testRefInt()
{

}

void testRefInt::initTestCase()
{

}

void testRefInt::cleanupTestCase()
{

}

void testRefInt::test_case1()
{
    QVERIFY(1==1);
}

QTEST_MAIN(testRefInt)

#include "tst_db.moc"
