#ifndef TEST_TABLEDATAINSERTER_H
#define TEST_TABLEDATAINSERTER_H

#include "testhelper.h"

class test_tableDataInserter : public QObject
{
    Q_OBJECT
    std::unique_ptr<TestTempDir> m_tmp;
    std::unique_ptr<ScopedCurrentDir> m_cwd;

private slots:
    void initTestCase();

    void init();
    void cleanup();

    void test_insert_and_retreive();

};

#endif // TEST_TABLEDATAINSERTER_H
