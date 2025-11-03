#ifndef TEST_TABLEDATAINSERTER_H
#define TEST_TABLEDATAINSERTER_H

#include <QtTest/QTest>

class test_tableDataInserter : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void init();
    void cleanup();

    void test_insert_and_retreive();

};

#endif // TEST_TABLEDATAINSERTER_H
