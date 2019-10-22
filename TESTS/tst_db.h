#ifndef TST_DB_H
#define TST_DB_H
//
#include <qobject.h>

class tst_db : public QObject
{
    Q_OBJECT
public:
    tst_db() {}
    ~tst_db() {}

private:
    const QString filename = "..\\data\\testdb.sqlite";

signals:

private slots:
    //    void initTestCase();
    //    void cleanupTestCase();
    void init();
    void cleanup();
    void test_createSimpleTable();
    void test_createSimpleTable2();
    void test_SimpleTableAddData();
    void test_createSimpleTable_wRefInt();
    void test_createSimpleTable_wRefInt2();
    void test_addRecords_wDep();
    void test_deleteRecord_wDep();
    void dbfieldCopyConst();
};
#endif // TST_DB_H
