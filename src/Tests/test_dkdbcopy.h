#ifndef TEST_DKDBCOPY_H
#define TEST_DKDBCOPY_H


#include "testhelper.h"

class test_dkdbcopy : public QObject
{
    Q_OBJECT
    std::unique_ptr<TestTempDir> m_tmp;
    std::unique_ptr<ScopedCurrentDir> m_cwd;

private:
    const QString dbfn1{qsl("testdb1.sqlite")};
    const QString dbfn2{qsl("testdb2.sqlite")};
    const QString tempFileName{testDbFilename +qsl(".preconversion.sqlite")};
signals:
private slots:
    void init();
    void cleanup();
    void test_init_and_cleanup();
    void test_moveToPreconversionBackup();
    void test_moveToPreconversionBackup_tmpfn();
    void test_dbsHaveSameTables();
    void test_dbsHaveSameTables_mtpl_tables();
    void test_dbsHaveSameTables_more_fields();
    void test_dbsHaveSameTables_fails_more_tables();
    void test_dbsHaveSameTables_fails_diffRowCount();
    void test_copyDatabase();
    void test_convertDatabaseInplace();
    void test_convertDatabaseInplace_wNewColumn();
    void test_copyDb_anonymous();
};

#endif // TEST_DKDBCOPY_H
