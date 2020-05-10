#include <QStandardPaths>

#include "test_appconfig.h"
#include "../DKV2/appconfig.h"

void test_appConfig::initTestCase()
{
//    appConfig::setTestmode();
    appConfig::setLastDb("c:\\temp\\data.dkdb");
    QVERIFY(!appConfig::LastDb().isEmpty());
    appConfig::setOutDir("C:\\temp\\output");
    QVERIFY(!appConfig::Outdir().isEmpty());
    appConfig::setCurrentDb("c:\\temp\\current.dkdb");
    QVERIFY(!appConfig::CurrentDb().isEmpty());
}
void test_appConfig::cleanupTestCase()
{
    appConfig::delLastDb();
    QVERIFY(appConfig::LastDb().isEmpty());
    appConfig::delOutDir();
    QVERIFY(appConfig::Outdir() == QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    appConfig::delCurrentDb();
    QVERIFY(appConfig::CurrentDb().isEmpty());
}

void test_appConfig::test_initials()
{
    // initTastCase runs w/o error
}
void test_appConfig::test_overwrite_value()
{
    QString newValue= "newvalue";
    // overwrite value fro initTestCase
    appConfig::setLastDb(newValue +"ldb");
    QCOMPARE(appConfig::LastDb(), newValue +"ldb");
    appConfig::setOutDir(newValue +"od");
    QCOMPARE(appConfig::Outdir(), newValue +"od");
    appConfig::setCurrentDb(newValue +"cdb");
    QCOMPARE(appConfig::CurrentDb(), newValue +"cdb");
}
