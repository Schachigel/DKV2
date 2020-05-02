
#include <qguiapplication.h>
#include <qtest.h>
#include "../DKV2/helper.h"
#include "test_properties.h"
#include "test_lettertemplate.h"
#include "test_csv.h"
#include "tst_db.h"
#include "test_dkdbhelper.h"
#include "test_finance.h"
#include "test_sqlhelper.h"
#include "test_appconfig.h"

#ifndef TESTLIB_SELFCOVERAGE_START
#define TESTLIB_SELFCOVERAGE_START(a)
#endif

// QTEST_MAIN(tst_db)
int main(int argc, char *argv[])
{
//    qInstallMessageHandler(logger);
    TESTLIB_SELFCOVERAGE_START(#tst_db)
    QGuiApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);
    app.setOrganizationName("4-MHS");
    app.setApplicationName("DKV2-tests");

    QTest::setMainSourcePath(__FILE__, "X:/home/dev/DKV2/TESTS");

    int errCount = 0;
    auto ASSERT_TEST = [&errCount, argc, argv](QObject* obj) {
      errCount += QTest::qExec(obj, argc, argv);
      delete obj;
    };

    ASSERT_TEST(new test_appConfig);
    ASSERT_TEST(new test_dkdbhelper);
    ASSERT_TEST(new test_sqlhelper);
    ASSERT_TEST(new test_finance);
    ASSERT_TEST(new test_properties);
    ASSERT_TEST(new test_letterTemplate);
    ASSERT_TEST(new tst_db);
    ASSERT_TEST(new test_csv);


    if( errCount == 1) qDebug() << "\n>>>   There was an error   <<< ";
    else if (errCount > 1) qDebug() << "\n>>>   There were " << errCount << " errors   <<<";
    else qDebug() << "\n>>>   There were no errors   <<<";

    return errCount;
}
