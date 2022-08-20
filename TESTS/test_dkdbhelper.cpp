#include <QtSql>
#include <QtTest>

#include "../DKV2/helper.h"
#include "../DKV2/tabledatainserter.h"
#include "../DKV2/helpersql.h"

#include "testhelper.h"
#include "test_dkdbhelper.h"

void test_dkdbhelper::init()
{   LOG_CALL;
    initTestDb_InMemory();
}

void test_dkdbhelper::cleanup()
{   LOG_CALL;
    cleanupTestDb_InMemory();
}

void test_dkdbhelper::test_selectQueryFromFields()
{   /* this test should be in test_sqlhelper
    but as the dkdbstructur is useful for this
    the test was moved here */
    QString sql = selectQueryFromFields(dkdbstructur["Buchungen"].Fields(), dkdbstructur["Buchungen"].ForeignKeys());
    qInfo() << sql << Qt::endl;
    QCOMPARE(sql,  "SELECT Buchungen.id, Buchungen.VertragsId, Buchungen.Datum, Buchungen.BuchungsArt, Buchungen.Betrag, Buchungen.Zeitstempel FROM Buchungen WHERE true AND Buchungen.VertragsId=Vertraege.id");
}

void test_dkdbhelper::test_querySingleValueInvalidQuery()
{   LOG_CALL;
    QString sql ("SELECT NOTEXISTINGFIELD FROM NOTEXISTINGTABLE WHERE NOTEXISTINGFIELD='0'");
    QVariant result;
    result = executeSingleValueSql(sql);
    QVERIFY2(QVariant::Invalid == result.type(),
             "Invalid single value sql has poditiv result");
}

void test_dkdbhelper::test_querySingleValue()
{   LOG_CALL;
    dbstructure s = dbstructure()
        .appendTable(dbtable("t")
            .append(dbfield("id", QVariant::Int))
            .append(dbfield("f")));
    s.createDb();
    TableDataInserter tdi(s["t"]);
    tdi.setValue("id", 1);
    tdi.setValue("f", "Hallo");
    tdi.WriteData();
    QVariant hallo = executeSingleValueSql("SELECT [f] FROM [t] WHERE id=1");
    QVERIFY2(hallo.toString() == "Hallo", "ExecuteSingleValueSql failed");
}

void test_dkdbhelper::test_querySingleValue_multipleResults()
{   LOG_CALL;
    dbstructure s = dbstructure()
        .appendTable(dbtable("t")
            .append(dbfield("id", QVariant::Int))
            .append(dbfield("f")));
    s.createDb();
    TableDataInserter tdi(s["t"]);
    tdi.setValue("id", 1);
    tdi.setValue("f", "Hallo");
    tdi.WriteData();
    tdi.setValue("id", 1);
    tdi.setValue("f", "Hallo1");
    tdi.WriteData();
    QVariant hallo = executeSingleValueSql("SELECT [f] FROM [t] WHERE id=1");
    QVERIFY2(hallo.type() == QVariant::Invalid , "ExecuteSingleValueSql failed");
}



