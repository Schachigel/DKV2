#ifndef TEST_APPCONFIG_H
#define TEST_APPCONFIG_H

#include "testhelper.h"

class test_appconfig : public QObject
{
    Q_OBJECT
    std::unique_ptr<TestTempDir> m_tmp;
    std::unique_ptr<ScopedCurrentDir> m_cwd;

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
private:
    QString oldDir;
};

#endif // TEST_APPCONFIG_H
