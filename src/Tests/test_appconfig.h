#ifndef TEST_APPCONFIG_H
#define TEST_APPCONFIG_H

class test_appConfig : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
//    void init();
//    void cleanup();
    void test_initials();
    void test_overwrite_value();
    void test_dbConfig_RuntimeData();
    void test_dbConfig_Db();
    void test_getMetaTableAsMap();
};

#endif // TEST_APPCONFIG_H
