#ifndef TEST_PROPERTIES_H
#define TEST_PROPERTIES_H

#include <QObject>
#include <QTest>

class test_properties : public QObject
{
    Q_OBJECT
public:
    explicit test_properties(QObject *p = nullptr);
    virtual ~test_properties(){}

private:
    const QString filename = "..\\..\\data\\testdb.sqlite";
    const QString testCon = "test_dkdb_connection";

private slots:
    void initTestCase();
    //    void cleanupTestCase();
    void init();
    void cleanup();
    void test_setProperty_getProperty();
    void test_initProperty_getProperty();
    void test_set_init_getProperty();
    void test_get_uninit();

    void test_set_get_num();
    void test_init_get_num();
    void test_set_init_get_num();
    void test_get_uninit_num();
};

#endif // TEST_PROPERTIES_H
