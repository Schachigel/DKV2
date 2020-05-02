#include <QStandardPaths>

#include "test_appconfig.h"
#include "../DKV2/appconfig.h"

void test_appConfig::initTestCase()
{
    appConfig::setTestmode();
    appConfig::setLastDb("c:\\temp\\data.dkdb");
    appConfig::setOutDir("C:\\temp\\output");
    appConfig::setCurrentDb("c:\\temp\\current.dkdb");
    QVERIFY(!appConfig::LastDb().isEmpty());
    QVERIFY(!appConfig::Outdir().isEmpty());
    QVERIFY(!appConfig::CurrentDb().isEmpty());
}
void test_appConfig::cleanupTestCase()
{
    appConfig::deleteUserData("test-db/last");
    appConfig::deleteUserData("test-outdir");
    appConfig::deleteRuntimeData("test-db/current");
    QVERIFY(appConfig::LastDb().isEmpty());
    QVERIFY(appConfig::Outdir() == QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
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
