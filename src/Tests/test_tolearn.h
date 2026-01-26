#ifndef TEST_TOLEARN_H
#define TEST_TOLEARN_H

class test_tolearn : public QObject
{
public:
    Q_OBJECT
private slots:
    // void initTestCase(){};
    // void cleanupTestCase(){};
    // void init(){};
    // void cleanup(){};
    void test_sql_with_parameter_binding();
};

#endif // TEST_TOLEARN_H
