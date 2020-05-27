#ifndef TEST_TABLEDATAINSERTER_H
#define TEST_TABLEDATAINSERTER_H

#include <QObject>

class test_tableDataInserter : public QObject
{
    Q_OBJECT
public:
    explicit test_tableDataInserter(QObject *parent = nullptr);
private slots:
    void initTestCase();
    void cleanupTestCase();
    // void init();
    // void cleanup();
    void test_insert_and_retreive();

};

#endif // TEST_TABLEDATAINSERTER_H
