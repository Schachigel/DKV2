
#include "../DKV2/helper.h"
#include "../DKV2/dbstructure.h"
#include "../DKV2/tabledatainserter.h"
#include "../DKV2/helpersql.h"

#include "testhelper.h"
#include "test_dkdbhelper.h"

void test_dkdbhelper::init()
{   LOG_CALL;
    initTestDkDb_InMemory();
}

void test_dkdbhelper::cleanup()
{   LOG_CALL;
    cleanupTestDb_InMemory();
}

void test_dkdbhelper::test_querySingleValueInvalidQuery()
{   LOG_CALL;
    QString sql ("SELECT NOTEXISTINGFIELD FROM NOTEXISTINGTABLE WHERE NOTEXISTINGFIELD='0'");
    QVariant result;
    result = executeSingleValueSql(sql);
    QVERIFY2( not result.isValid(),
             "Invalid single value sql has poditiv result");
}

void test_dkdbhelper::test_querySingleValue()
{   LOG_CALL;
    dbstructure s = dbstructure()
        .appendTable(dbtable("t")
            .append(dbfield("id", QMetaType::Int))
            .append(dbfield("f")));
    s.createDb();
    TableDataInserter tdi(s["t"]);
    tdi.setValue("id", 1);
    tdi.setValue("f", "Hallo");
    tdi.InsertRecord();
    QVariant hallo = executeSingleValueSql("SELECT [f] FROM [t] WHERE id=1");
    QVERIFY2(hallo.toString() == "Hallo", "ExecuteSingleValueSql failed");
}

void test_dkdbhelper::test_querySingleValue_multipleResults()
{   LOG_CALL;
    dbstructure s = dbstructure()
        .appendTable(dbtable("t")
            .append(dbfield("id", QMetaType::Int))
            .append(dbfield("f")));
    s.createDb();
    TableDataInserter tdi(s["t"]);
    tdi.setValue("id", 1);
    tdi.setValue("f", "Hallo");
    tdi.InsertRecord();

    tdi.setValue("id", 1);
    tdi.setValue("f", "Hallo1");
    tdi.InsertRecord();

    QVariant hallo = executeSingleValueSql("SELECT [f] FROM [t] WHERE id=1");
    QVERIFY2(not hallo.isValid(), "ExecuteSingleValueSql failed");
}



