#ifndef TST_DB_H
#define TST_DB_H
//
#include <qobject.h>

class test_db : public QObject
{
    Q_OBJECT
public:
    test_db() {}
    ~test_db() {}

private:

signals:

private slots:
    void initTestCase();
    //    void cleanupTestCase();
    void init();
    void cleanup();
    void test_init_and_cleanup();
    void test_createSimpleTable();
    void test_createSimpleTable2();
    void test_SimpleTableAddData();
    void test_createSimpleTable_wRefInt();
    void test_createSimpleTable_wRefInt2();
    void test_addRecords_wDep();
    void test_deleteRecord_wDep();
    void dbfieldCopyConst();
    void newDbIsValid();
};
#endif // TST_DB_H
