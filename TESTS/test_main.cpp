
#include <qguiapplication.h>
#include <qtest.h>
#include "tst_db.h"
#include "test_dkdbhelper.h"
#include "test_finance.h"

#ifndef TESTLIB_SELFCOVERAGE_START
#define TESTLIB_SELFCOVERAGE_START(a)
#endif

// QTEST_MAIN(tst_db)
int main(int argc, char *argv[])
{
    TESTLIB_SELFCOVERAGE_START(#tst_db)
    QGuiApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);
    QTest::setMainSourcePath(__FILE__, "X:/home/dev/DKV2/TESTS");

    int status = 0;
    auto ASSERT_TEST = [&status, argc, argv](QObject* obj) {
      status += QTest::qExec(obj, argc, argv);
      delete obj;
    };

//    ASSERT_TEST(new tst_db);
//    ASSERT_TEST(new test_dkdbhelper);
    ASSERT_TEST(new test_finance);


    if( status == 1) qDebug() << "\n>>>   There was an error   <<< ";
    else if (status > 1) qDebug() << "\n>>>   There were " << status << " errors   <<<";
    else qDebug() << "\n>>>   There were no errors   <<<";

    return status;
}
