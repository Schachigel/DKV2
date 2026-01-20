#include <QtTest/QTest>
#include <QCoreApplication>

// Pro Target wird GEN_TEST_HEADER und GEN_TEST_CLASS gesetzt.
#ifndef GEN_TEST_HEADER
#  error "GEN_TEST_HEADER not defined"
#endif
#ifndef GEN_TEST_CLASS
#  error "GEN_TEST_CLASS not defined"
#endif

#include GEN_TEST_HEADER

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("DKV2-Test");
    QCoreApplication::setApplicationName("test_DKV2");

    GEN_TEST_CLASS tc;
    // argc =1;
    // char* arg =new (char[]){"-maxwarnings 0"};
    // char * v[] {arg, NULL};
    return QTest::qExec(&tc, argc, argv);
}
