
#include <QtTest/QTest>

#include <vector>
#include "../DKV2/helper.h"
#include "test_dkdbcopy.h"
#include "test_properties.h"
#include "test_csv.h"
#include "test_db.h"
#include "test_dkdbhelper.h"
#include "test_finance.h"
#include "test_sqlhelper.h"
#include "test_appconfig.h"
#include "test_creditor.h"
#include "test_contract.h"
#include "test_booking.h"
#include "test_statistics.h"
#include "test_tabledatainserter.h"
#include "test_dbtable.h"

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
    QDir dir =QDir::current();
    qInfo() << "Running in " << dir.path();
    dir.mkpath("../data");

    std::vector<QObject*> tests;

    int executions =5;
    do {
//        in memory db
        tests.push_back(new test_booking);
        tests.push_back(new test_appConfig);
        tests.push_back(new test_creditor);
        tests.push_back(new test_contract);
        tests.push_back(new test_sqlhelper);
        tests.push_back(new test_statistics);
        tests.push_back(new test_db);
        tests.push_back(new test_dkdbhelper);
        tests.push_back(new test_properties);

        // no db
        tests.push_back(new test_finance);
        tests.push_back(new test_csv);
        tests.push_back(new test_dbfield);

        // on disk db
        tests.push_back(new test_tableDataInserter);
        tests.push_back(new test_dkdbcopy);

// NO ACTIVE TESTS        tests.push_back(new test_letterTemplate);
// NO ACTIVE TESTS        tests.push_back(new test_views);
    } while(--executions);

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(tests.begin(), tests.end(), g);


// one test only
    tests.clear();
    tests.push_back(new test_csv);
//

    int errCount = 0;
    {
        dbgTimer timer("overall test time");
        for( auto test: tests){
//            qInfo() << " running " << test->objectName();
            errCount += QTest::qExec( test);
        }
    } // timer scope to measure test execution

    if( errCount == 1) qInfo() << "\n>>>   There was an error   <<< ";
    else if (errCount > 1) qInfo() << "\n>>>   There were " << errCount << " errors   <<<";
    else qInfo() << "\n>>>   There were no errors   <<<";

    return errCount;
}
