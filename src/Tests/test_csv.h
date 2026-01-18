#ifndef TST_CSV_H
#define TST_CSV_H

class test_csv : public QObject
{
    Q_OBJECT

private slots:
    // void initTestCase(){};
    // void cleanupTestCase(){};
    // void init(){};
    // void cleanup(){};

    void test_toString_noHeader_wTrimming_data();
    void test_toString_noHeader_wTrimming();
    void test_toString_noHeader_NO_Trimming_data();
    void test_toString_noHeader_NO_Trimming();
    void test_toString_wHeader_wTrimming_data();
    void test_toString_wHeader_wTrimming();
};

#endif // TST_CSV_H
