#ifndef TEST_DKDBHELPER_H
#define TEST_DKDBHELPER_H

#include <QSqlDatabase>
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

signals:

private slots:
    void init();
    void cleanup();

    void test_selectQueryFromFields();

    void test_querySingleValueInvalidQuery();
    void test_querySingleValue();
    void test_querySingleValue_multipleResults();

};

#endif // TEST_DKDBHELPER_H
