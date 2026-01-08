#ifndef TST_CSV_H
#define TST_CSV_H

#include <QtTest/QtTest>

class test_csv : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase(){};
    void cleanupTestCase(){};
    void init(){};
    void cleanup(){};

    // void test_sql_with_parameter_binding();
    void test_toString_noHeader_wTrimming_data();
    void test_toString_noHeader_wTrimming();
    void test_toString_noHeader_NO_Trimming_data();
    void test_toString_noHeader_NO_Trimming();
    void test_toString_wHeader_wTrimming_data();
    void test_toString_wHeader_wTrimming();

    // void test_empty_csv();
    // void test_csv_oneHeader();
    // void test_csv_oneHeader_withSpace();
    // void test_csv_twoHeader();
    // void test_csv_twoHeader_useAddColumns();
    // void test_csv_twoHeader_useAddColumns_rm_space();
    // void test_csv_oneHeader_oneRow();
    // void test_csv_oneHeader_oneRow01();
    // void test_csv_Headers_Rows();
    // void test_csv_Headers_Rows01();
    // void test_csv_Headers_Rows02();
    // void test_csv_fix_semicolon();
    //void test_csv_output();

signals:
public slots:
};

#endif // TST_CSV_H
