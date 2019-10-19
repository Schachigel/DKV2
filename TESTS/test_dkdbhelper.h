#ifndef TEST_DKDBHELPER_H
#define TEST_DKDBHELPER_H

#include <QObject>

class test_dkdbhelper : public QObject
{
    Q_OBJECT
public:
    test_dkdbhelper(){}
    ~test_dkdbhelper(){}

private:
    const QString filename = "..\\data\\testdb.sqlite";
    const QString testCon = "test_dkdb_connection";

signals:

private slots:
    //    void initTestCase();
    //    void cleanupTestCase();
    void init();
    void cleanup();
    void test_empty();
    void test_empty2();
};

#endif // TEST_DKDBHELPER_H
