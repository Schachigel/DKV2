#ifndef TEST_CREDITOR_H
#define TEST_CREDITOR_H


#include <QSqlDatabase>
#include <QObject>

#include "testhelper.h"

class test_creditor : public QObject
{
    Q_OBJECT
public:
    explicit test_creditor(QObject* p=nullptr) : QObject(p){}
    ~test_creditor(){}
private:
    // helper
signals:
private slots:
    void initTestCase();
//    void cleanupTestCase();
    void init();
    void cleanup();
    // the actual tests
    void test_createCreditor();
    void test_CreditorFromDb();
    void test_invalidCreditor();
    void test_saveManyRandomCreditors();

};

#endif // TEST_CREDITOR_H
