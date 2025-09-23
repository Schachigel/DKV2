#ifndef TEST_SQLHELPER_H
#define TEST_SQLHELPER_H

#include <QtTest/QTest>

class test_sqlhelper : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void test_init_cleanup();

    void test_rowCount();
    void test_getHighestRowId();
    void test_tableExists();
    void test_ensureTable_existingTableOK();
    void test_ensureTable_existingTable_tableSizeMismatch();
    void test_ensureTable_existingtable_fieldTypeMismatch();
    void test_ensureTable_nonexistingTable();

    void test_executeSql_noParams();
    void test_executeSql_namedParams();
    void test_executeSql_posParams();

    void test_switchForeignKeyHandling();
    void test_eSingleValueSql_query();
    void test_eSingleValueSql_field_table_PreservesValues();
    void test_eSingleValueSql_dbfield_PreservsValue();
    void test_selectQueryFromFields_noWhere();
    void test_selectQueryFromFields_withWhere();
    void test_executeSingleColumnSql();
    void test_variantTypeConservation();
    void test_createDbViews();
};

#endif // TEST_SQLHELPER_H
