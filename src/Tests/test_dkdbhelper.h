#ifndef TEST_DKDBHELPER_H
#define TEST_DKDBHELPER_H

class test_dkdbhelper : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void test_querySingleValueInvalidQuery();
    void test_querySingleValue();
    void test_querySingleValue_multipleResults();

};

#endif // TEST_DKDBHELPER_H
