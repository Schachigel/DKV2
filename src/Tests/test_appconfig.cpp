#include "test_appconfig.h"

#include "../DKV2/helpersql.h"
#include "../DKV2/appconfig.h"
#include "../DKV2/dkdbhelper.h"

#include "testhelper.h"

#include <QtTest/QTest>

void test_appconfig::init()
{
    m_tmp = std::make_unique<TestTempDir>(this);
    QVERIFY(m_tmp->isValid());
    m_cwd = std::make_unique<ScopedCurrentDir>(m_tmp->path());
    QVERIFY(m_cwd->ok());

    appconfig::setLastDb("c:/temp/data.dkdb");
    QVERIFY( appconfig::LastDb().size());
    appconfig::setOutDir("C:/temp/output");
    QVERIFY( appconfig::Outdir().size());
    initTestDkDb_InMemory();
    fill_DkDbDefaultContent(QSqlDatabase::database(), false);
}
void test_appconfig::cleanup()
{
    appconfig::delLastDb();
    appconfig::delOutDir();
    QCOMPARE(appconfig::Outdir(), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +qsl("/DKV2"));
    cleanupTestDb_InMemory();

    m_cwd.reset();  // leave directory first
    m_tmp.reset();  // then delete directory
}

void test_appconfig::test_initials()
{
    // init/cleanup runs w/o error
}
void test_appconfig::test_overwrite_value()
{
    QString newValue= "newvalue";
    // overwrite value for initTestCase
    appconfig::setLastDb(newValue +"ldb");
    QCOMPARE(appconfig::LastDb(), newValue +"ldb");
    appconfig::setOutDir(newValue +"od");
    QCOMPARE(appconfig::Outdir(), newValue +"od");
}

void test_appconfig::test_dbConfig_RuntimeData()
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

void test_appconfig::test_dbConfig_Db()
{
    // the "global" config data
    QString expectedString{qsl("original Db Value")};
    dbConfig::writeValue(GMBH_ADDRESS1, expectedString);
    QCOMPARE(dbConfig::readValue(GMBH_ADDRESS1), expectedString);

    // now lets start a second db
    QString newDbFilename{qsl("new.dkdb")};
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
    QVERIFY(QFile::remove(newDbFilename));
}

void test_appconfig::test_getMetaTableAsMap()
{
    auto map =getMetaTableAsMap ();
//     qDebug() << map;
    QCOMPARE( map.value ("gmbhprojekt"), "Esperanza");
    QCOMPARE( map.size (), projectConfiguration::MAX_PC_INDEX);
}
