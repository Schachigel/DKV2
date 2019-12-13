#ifndef TST_CSV_H
#define TST_CSV_H

#include <QObject>
#include <QTest>

class test_csv : public QObject
{
    Q_OBJECT
public:
    explicit test_csv(QObject *parent = nullptr);
    ~test_csv(){}
private slots:
    //    void initTestCase();
    //    void cleanupTestCase();
    void init(){}
    void cleanup(){}
    void test_empty_csv();
    void test_csv_oneHeader();
    void test_csv_twoHeader();
    void test_csv_twoHeader_useAddColumns();
    void test_csv_twoHeader_useAddColumns_rm_space();
    void test_csv_oneHeader_oneRow();
    void test_csv_oneHeader_oneRow01();
    void test_csv_Headers_Rows();
    void test_csv_Headers_Rows01();
    void test_csv_Headers_Rows02();
    void test_csv_fix_semicolon();
    //void test_csv_output();

signals:

public slots:
};

#endif // TST_CSV_H
