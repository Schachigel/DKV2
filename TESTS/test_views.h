#ifndef TEST_VIEWS_H
#define TEST_VIEWS_H

#include <QSqlDatabase>
#include <QObject>

#include "../DKV2/creditor.h"
#include "../DKV2/contract.h"

#include "testhelper.h"

class test_views : public QObject
{
    Q_OBJECT
public:
    explicit test_views(QObject *parent = nullptr) : QObject(parent){}
    ~test_views(){}

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

//    void test_stat_activateContract_reinvesting();
//    void test_stat_activateContract_wpayout();

//    void test_stat_create_activate_multipleContract();
//    void test_stat_annualSettelemnt();
//    void test_stat_extend_contract_sameYear();

//    void test_stat_mny_contracts();
};

#endif // TEST_VIEWS_H
