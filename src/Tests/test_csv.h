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
    void test_toString_newlines_in_fields_wTrimming_data();
    void test_toString_newlines_in_fields_wTrimming();
    void test_toString_tortureField_combo_data();
    void test_toString_tortureField_combo();
    void test_toString_quoteAllFields_data();
    void test_toString_quoteAllFields();
    void test_toString_emits_unfinished_last_record();
    void test_toString_withHeader_incomplete_last_row_is_written();
    void test_toString_whitespace_only_fields_data();
    void test_toString_whitespace_only_fields();
    void test_toString_unicode_fields();
    void test_toString_custom_separator_comma();
};

#endif // TST_CSV_H
