#include <QtTest>

#include "../DKV2/dbtable.h"
#include "../DKV2/helper.h"

#include "test_dbtable.h"



void test_dbfield::simpleTable_defaultValues()
{
    dbtable t("t");
    dbfield f("f");
    t.append (f);
    QCOMPARE(t.createTableSql(), qsl("CREATE TABLE t (f TEXT)"));
}

void test_dbfield::simpleTable_differentTypes()
{
    {
        dbtable t("t");
        dbfield f("f", QVariant::LongLong);
        t.append (f);
        QCOMPARE(t.createTableSql(), qsl("CREATE TABLE t (f INTEGER)"));
    }
    {
        dbtable t("t");
        dbfield f("f", QVariant::Date);
        t.append (f);
        QCOMPARE(t.createTableSql(), qsl("CREATE TABLE t (f TEXTDATE)"));
    }
    {   // multiple fields
        dbtable t("t");
        dbfield f("f", QVariant::Date);
        t.append (f);
        dbfield g("g", QVariant::String);
        t.append (g);
        QCOMPARE(t.createTableSql(), qsl("CREATE TABLE t (f TEXTDATE, g TEXT)"));
    }
    {   // multiple fields II
        dbtable t("t");
        dbfield e("e", QVariant::Int);
        t.append (e);
        dbfield f("f", QVariant::Date);
        t.append (f);
        dbfield g("g", QVariant::String);
        t.append (g);
        QCOMPARE(t.createTableSql(), qsl("CREATE TABLE t (e INTEGER, f TEXTDATE, g TEXT)"));
    }
}

void test_dbfield::simpleTable_withDefaultValues()
{
    {
        dbtable t("t");
        dbfield f("f", QVariant::LongLong);
        t.append (f.setDefault (QVariant(13)));
        QCOMPARE(t.createTableSql(), qsl("CREATE TABLE t (f INTEGER DEFAULT 13)"));
    }
    {
        dbtable t("t");
        dbfield f("f", QVariant::String);
        t.append (f.setDefault (QVariant("Hallo World")));
        QCOMPARE(t.createTableSql(), qsl("CREATE TABLE t (f TEXT DEFAULT 'Hallo World')"));
    }
    {   // with "unique" field
        dbtable t("t");
        dbfield f("f", QVariant::String);
        t.append (f.setUnique ());
        QCOMPARE(t.createTableSql(), qsl("CREATE TABLE t (f TEXT UNIQUE)"));
    }
    {
        dbtable t("t");
        dbfield f("f", QVariant::Date);
        t.append (f.setDefault (QVariant(QDate(1965, 5, 7))));
        QCOMPARE(t.createTableSql(), qsl("CREATE TABLE t (f TEXTDATE DEFAULT '1965-05-07')"));
    }
    {
        dbtable t("t");
        dbfield f("f", QVariant::Bool);
        t.append (f.setDefault (QVariant(false)));
        QCOMPARE(t.createTableSql(), qsl("CREATE TABLE t (f BOOLEAN DEFAULT FALSE)"));
    }
    { // multiple fields
        dbtable t("t");
        dbfield f("f", QVariant::LongLong);
        t.append (f.setDefault (QVariant(13)));
        dbfield g("g", QVariant::String);
        t.append(g.setDefault (qsl("Hallo World !! "))); // dbfield will not trim
        dbfield h("h", QVariant::Bool);
        t.append(h.setDefault (false));
        QCOMPARE(t.createTableSql(), qsl("CREATE TABLE t (f INTEGER DEFAULT 13, g TEXT DEFAULT 'Hallo World !! ', h BOOLEAN DEFAULT FALSE)"));
    }
}

void test_dbfield::table_wForeignKey()
{
    dbtable t("t");
    dbfield foreign("for", QVariant::LongLong);
    t.append (foreign);
    dbfield parent("parentkey", QVariant::LongLong);
    parent.setTableName ("parentTable");
    dbForeignKey fKey(foreign, parent, ODOU_Action::SET_NULL);
    t.append (fKey);
    QCOMPARE (t.createTableSql (), qsl("CREATE TABLE t (for INTEGER, FOREIGN KEY (for) REFERENCES parentTable (parentkey) ON DELETE SET NULL ON UPDATE NO ACTION)"));
}

void test_dbfield::table_wMultiFieldUniqueness()
{
    dbtable t("t");
    dbfield rowid("rowid", QVariant::LongLong);
    t.append(rowid.setAutoInc ());
    dbfield f1("f1");
    t.append (f1);
    dbfield f2("f2");
    t.append (f2);
    t.setUnique ({f1, f2});
    QCOMPARE (t.createTableSql (), qsl("CREATE TABLE t (rowid INTEGER PRIMARY KEY AUTOINCREMENT, f1 TEXT, f2 TEXT, UNIQUE (f1, f2))"));
}
