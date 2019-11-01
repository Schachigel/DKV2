#ifndef TEST_FINANCE_H
#define TEST_FINANCE_H

#include <QObject>

class test_finance : public QObject
{
    Q_OBJECT
public:
    explicit test_finance(){}
    ~test_finance(){}
private:
    // helper functions, test global stuff
signals:

private slots:
    //    void initTestCase();
    //    void cleanupTestCase();
    //    void init();
    //    void cleanup();

    void test_TageBisJahresende_data();
    void test_TageBisJahresende();
    void test_TageSeitJahresanfang_data();
    void test_TageSeitJahresanfang();
//    void test_TageZwischen_data();
//    void test_TageZwischen();
    void test_ZinsesZins_data();
    void test_ZinsesZins();
};

#endif // TEST_FINANCE_H
