#include <QStandardPaths>

#include "../DKV2/appconfig.h"
#include "../DKV2/dkdbhelper.h"
#include "testhelper.h"
#include "test_appconfig.h"

void test_appConfig::initTestCase()
{
    appConfig::setLastDb("c:\\temp\\data.dkdb");
    QVERIFY(!appConfig::LastDb().isEmpty());
    appConfig::setOutDir("C:\\temp\\output");
    QVERIFY(!appConfig::Outdir().isEmpty());
    appConfig::setCurrentDb("c:\\temp\\current.dkdb");
    QVERIFY(!appConfig::CurrentDb().isEmpty());
    initTestDb();
    init_DKDBStruct();
    create_DK_TablesAndContent();

}
void test_appConfig::cleanupTestCase()
{
    appConfig::delLastDb();
    QVERIFY(appConfig::LastDb().isEmpty());
    appConfig::delOutDir();
    QVERIFY(appConfig::Outdir() == QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    appConfig::delCurrentDb();
    QVERIFY(appConfig::CurrentDb().isEmpty());
    cleanupTestDb();
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

void test_appConfig::test_dbConfig_RuntimeData()
{
    dbConfig in;
    in.address1 ="1"; in.address2 ="2"; in.street ="street";
    in.plz ="plz"; in.city ="city"; in.email ="email";
    in.url ="url"; in.pi ="pi"; in.hre ="hre";
    in.gefue1 ="g1"; in.gefue2 ="g2"; in.gefue3 ="g3";
    in.dkv ="dkv"; in.startindex =1; in.dbId ="dbid";
    in.dbVersion =3.1;
    in.toRuntimeData();
    QCOMPARE(in, in);
    QCOMPARE(in, dbConfig::fromRuntimeData());
}

void test_appConfig::test_dbConfig_Db()
{
    dbConfig in;
    in.address1 ="1"; in.address2 ="2"; in.street ="street";
    in.plz ="plz"; in.city ="city"; in.email ="email";
    in.url ="url"; in.pi ="pi"; in.hre ="hre";
    in.gefue1 ="g1"; in.gefue2 ="g2"; in.gefue3 ="g3";
    in.dkv ="dkv"; in.startindex =1; in.dbId ="dbid";
    in.dbVersion =3.1;

    in.toDb();
    QCOMPARE(in, dbConfig::fromDb());
}
