#ifndef TEST_CONTRACT_H
#define TEST_CONTRACT_H

#include <QSqlDatabase>
#include <QObject>

#include "testhelper.h"

class test_contract : public QObject
{
    Q_OBJECT
public:
    explicit test_contract(QObject *parent = nullptr) : QObject(parent){}
    ~test_contract(){}
private:
    // helper
signals:
private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    // the actual tests
    void test_createContract();
};

#endif // TEST_CONTRACT_H
