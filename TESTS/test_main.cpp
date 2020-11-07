
#include <QGuiApplication>
#include <QList>
#include <QTest>

#include <vector>
#include "../DKV2/helper.h"
#include "test_properties.h"
#include "test_lettertemplate.h"
#include "test_csv.h"
#include "tst_db.h"
#include "test_dkdbhelper.h"
#include "test_finance.h"
#include "test_sqlhelper.h"
#include "test_appconfig.h"
#include "test_creditor.h"
#include "test_contract.h"
#include "test_views.h"
#include "test_booking.h"
#include "test_tabledatainserter.h"

#ifndef TESTLIB_SELFCOVERAGE_START
#define TESTLIB_SELFCOVERAGE_START(a)
#endif

// QTEST_MAIN(tst_db)
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);
    app.setOrganizationName("4-MHS");
    app.setApplicationName("DKV2-tests");

//    QTest::setMainSourcePath(__FILE__, "X:/home/dev/DKV2/TESTS");
    int errCount = 0;
    auto ASSERT_TEST = [&errCount, argc, argv](QObject* obj)
    {
        errCount += QTest::qExec(obj, argc, argv);
        delete obj;
    };

    std::vector<QObject*> tests;
    tests.push_back(new test_appConfig);
    tests.push_back(new test_booking);
    tests.push_back(new test_contract);
    tests.push_back(new test_views);
    tests.push_back(new test_db);
    tests.push_back(new test_creditor);
    tests.push_back(new test_csv);
    tests.push_back(new test_dkdbhelper);
    tests.push_back(new test_letterTemplate);
    tests.push_back(new test_finance);
    tests.push_back(new test_properties);
    tests.push_back(new test_sqlhelper);
    tests.push_back(new test_tableDataInserter);

    srand(time(0));
    std::random_shuffle(tests.begin(), tests.end());

    for( auto test: tests)
        ASSERT_TEST(test);

    if( errCount == 1) qDebug() << "\n>>>   There was an error   <<< ";
    else if (errCount > 1) qDebug() << "\n>>>   There were " << errCount << " errors   <<<";
    else qDebug() << "\n>>>   There were no errors   <<<";

    return errCount;
}
