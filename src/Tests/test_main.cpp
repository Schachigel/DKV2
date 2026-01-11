
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
#include "xtest_tolearn.h"

#ifndef TESTLIB_SELFCOVERAGE_START
#define TESTLIB_SELFCOVERAGE_START(a)
#endif

void initTestRun(QApplication& app)
{
    app.setAttribute(Qt::AA_Use96Dpi, true);
    app.setOrganizationName("4-MHS");
    app.setApplicationName("DKV2-tests");
    QDir dir = QDir::current();
    qInfo() << "Running in " << dir.path();
    dir.mkpath("../data");
}
void shuffleTests(std::vector<std::unique_ptr<QObject *>> &tests) {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(tests.begin(), tests.end(), g);
}

void prepareTests(int executions, std::vector<std::unique_ptr<QObject *>> &tests)
{
    Q_ASSERT(executions);
    if (tests.size())
        // for singel test debugging
        return;
    do {
        // in memory db
        tests.push_back(std::make_unique<QObject *>(new test_booking));
        tests.push_back(std::make_unique<QObject *>(new test_appConfig));
        tests.push_back(std::make_unique<QObject *>(new test_creditor));
        tests.push_back(std::make_unique<QObject *>(new test_contract));
        tests.push_back(std::make_unique<QObject *>(new test_sqlhelper));
        tests.push_back(std::make_unique<QObject *>(new test_statistics));
        tests.push_back(std::make_unique<QObject *>(new test_db));
        tests.push_back(std::make_unique<QObject *>(new test_dkdbhelper));
        tests.push_back(std::make_unique<QObject *>(new test_properties));
        tests.push_back(std::make_unique<QObject *>(new test_toLearn));

        // no db
        tests.push_back(std::make_unique<QObject *>(new test_finance));
        tests.push_back(std::make_unique<QObject *>(new test_csv));
        tests.push_back(std::make_unique<QObject *>(new test_dbfield));

        // on disk db
        tests.push_back(std::make_unique<QObject *>(new test_tableDataInserter));
        tests.push_back(std::make_unique<QObject *>(new test_dkdbcopy));

        // NO ACTIVE TESTS        tests.push_back(new test_letterTemplate);
        // NO ACTIVE TESTS        tests.push_back(new test_views);
    } while (--executions);

    shuffleTests( tests);
}

int runTests(const std::vector<std::unique_ptr<QObject *>> &tests)
{
    int errCount {0};
    dbgTimer timer("overall test time");
    for( auto& test: tests) {
        QObject* p =*test.get();
        qInfo() << " running " << p->objectName();
        // THE TESTs RUN HERE
        errCount += QTest::qExec( p);
        // THE TESTs RUN HERE
    }
    return errCount;
}

int main(int argc, char *argv[]) {

    QApplication app(argc, argv);
    initTestRun(app);

    const int nbrRuns = 5;
    std::vector<std::unique_ptr<QObject *>> tests;
// use the following line for single test Debugging
    //tests.push_back(std::make_unique<QObject *>(new test_csv));

    prepareTests(nbrRuns, tests);

    int errCount { runTests(tests)};

    if( errCount == 1) qInfo() << "\n>>>   There was an error   <<< ";
    else if (errCount > 1) qInfo() << "\n>>>   There were " << errCount << " errors   <<<";
    else qInfo() << "\n>>>   There were no errors   <<<";

    return errCount;
}
