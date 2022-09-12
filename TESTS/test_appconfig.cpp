#include <QStandardPaths>

#include "../DKV2/appconfig.h"
#include "../DKV2/helpersql.h"
#include "../DKV2/dkdbhelper.h"

#include "testhelper.h"
#include "test_appconfig.h"

void test_appConfig::initTestCase()
{
    appConfig::setLastDb("c:/temp/data.dkdb");
    QVERIFY( appConfig::LastDb().size());
    appConfig::setOutDir("C:/temp/output");
    QVERIFY( appConfig::Outdir().size());
    initTestDb_InMemory();
    fill_DkDbDefaultContent(QSqlDatabase::database(), false);

}
void test_appConfig::cleanupTestCase()
{
    appConfig::delLastDb();
    appConfig::delOutDir();
    QCOMPARE(appConfig::Outdir(), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +qsl("/DKV2"));
    cleanupTestDb_InMemory();
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
}

void test_appConfig::test_dbConfig_RuntimeData()
{
    QString expectedString{qsl("gmbh adresse 1")};
    dbConfig::writeValue(GMBH_ADDRESS1, expectedString);
    QCOMPARE(dbConfig::readValue(GMBH_ADDRESS1), expectedString);

    QVariant vInt{13};
    dbConfig::writeValue(STARTINDEX, vInt);
    QCOMPARE(dbConfig::readValue(STARTINDEX), vInt);

//    QVariant vDouble{42.24};
    dbConfig::writeValue(MIN_AMOUNT, vInt);
    QCOMPARE(dbConfig::readValue(MIN_AMOUNT), vInt);
}

void test_appConfig::test_dbConfig_Db()
{
    // the "global" config data
    QString expectedString{qsl("original Db Value")};
    dbConfig::writeValue(GMBH_ADDRESS1, expectedString);
    QCOMPARE(dbConfig::readValue(GMBH_ADDRESS1), expectedString);

    // now lets start a second db
    QString newDbFilename{qsl("../data/new.dkdb")};
    if( QFile::exists(newDbFilename)) QFile::remove(newDbFilename);
    QVERIFY( not QFile::exists(newDbFilename));
    {
        QSqlDatabase newDb =QSqlDatabase::addDatabase(dbTypeName, qsl("newdb"));
        newDb.setDatabaseName(newDbFilename);
        QVERIFY(newDb.open());
        QVERIFY(dkdbstructur.createDb(newDb));
        fill_DkDbDefaultContent(newDb);

        QString newValue{qsl("Value of new DB")};
        dbConfig::writeValue(GMBH_ADDRESS1, newValue, newDb);
        // the value in runtime and default db has to stay
        QCOMPARE(dbConfig::readValue(GMBH_ADDRESS1).toString(), expectedString);
        // the value in the newDB should be independent of the runtime value
        QCOMPARE(dbConfig::readValue(GMBH_ADDRESS1, newDb).toString(), newValue);
        newDb.close();
    }
    QSqlDatabase::removeDatabase(qsl("newdb"));
    QFile::remove(newDbFilename);
}
