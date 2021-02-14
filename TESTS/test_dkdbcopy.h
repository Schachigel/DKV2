#ifndef TEST_DKDBCOPY_H
#define TEST_DKDBCOPY_H

#include <QSqlDatabase>
#include <QObject>
#include <QDir>
#include <QDebug>

#include "testhelper.h"

class test_dkdbcopy : public QObject{
    Q_OBJECT
public:
    test_dkdbcopy(){}
    ~test_dkdbcopy(){}
private:
    const QString dbfn1{qsl("../data/testdb1.sql")};
    const QString dbfn2{qsl("../data/testdb2.sql")};
    const QString tempFileName{testDbFilename +qsl(".preconversion")};
signals:
private slots:
    void init();
    void cleanup();
    void test_moveToPreconversionBackup();
    void test_moveToPreconversionBackup_tmpfn();
    void test_dbsHaveSameTables();
    void test_dbsHaveSameTables_mtpl_tables();
    void test_dbsHaveSameTables_more_fields();
    void test_dbsHaveSameTables_fails_more_tables();
    void test_dbsHaveSameTables_fails_diffRowCount();
    void test_convertDatabaseInplace();
};

#endif // TEST_DKDBCOPY_H
