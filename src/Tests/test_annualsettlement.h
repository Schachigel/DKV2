#ifndef TEST_ANNUALSETTLEMENT_H
#define TEST_ANNUALSETTLEMENT_H

#include <QObject>

class test_annualSettlement : public QObject
{
    Q_OBJECT

private slots:
    void init(); // for each test
    void cleanup();

    void test_noContract_noAS();
    void test_oneContract_InitMidYear_onContract();
    void test_oneContract_InitMidYear_globally();
    void test_oneContract_InitOnYearEnd_onContract();
    void test_oneContract_InitOnYearEnd_globally();
signals:
};

#endif // TEST_ANNUALSETTLEMENT_H
