#ifndef TEST_DKDBHELPER_H
#define TEST_DKDBHELPER_H

#include <QObject>
#include <QDir>
#include <QDebug>

class test_dkdbhelper : public QObject
{
    Q_OBJECT
public:
    test_dkdbhelper(){}
    ~test_dkdbhelper(){}

private:
    const QString filename = "..\\..\\data\\testdb.sqlite";
    const QString testCon = "test_dkdb_connection";

signals:

private slots:
    //    void initTestCase();
    //    void cleanupTestCase();
    void init();
    void cleanup();
    void test_querySingleValueInvalidQuery();
    void test_querySingleValue();
    void test_querySingleValue_multipleResults();
    void test_berechneZusammenfassung();
    void test_ensureTable_existingTable();
    void test_ensureTable_notExistingTable();
};

#endif // TEST_DKDBHELPER_H
