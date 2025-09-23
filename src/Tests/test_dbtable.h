#ifndef TEST_DBTABLE_H
#define TEST_DBTABLE_H

#include <QtTest/QTest>

class test_dbfield : public QObject
{
    Q_OBJECT
public:
    test_dbfield() {}
    ~test_dbfield() {}

private:

signals:

private slots:
    // void initTestCase();
    // void cleanupTestCase();
    // void init();
    // void cleanup();
    void simpleTable_defaultValues();
    void simpleTable_differentTypes();
    void simpleTable_withDefaultValues();
    void table_wForeignKey();
    void table_wMultiFieldUniqueness();
};
#endif // TEST_DBTABLE_H
