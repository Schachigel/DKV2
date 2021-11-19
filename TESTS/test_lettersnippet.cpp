#include <QtSql>
#include <QtTest>

#include "../DKV2/helper.h"
#include "../DKV2/helpersql.h"
#include "../DKV2/dkdbhelper.h"
#include "../DKV2/lettersnippets.h"

#include "testhelper.h"
#include "test_lettersnippet.h"


test_letterSnippet::test_letterSnippet(QObject *p) : QObject(p)
{
}

void test_letterSnippet::initTestCase()
{
    init_DKDBStruct();
}

void test_letterSnippet::init()
{   LOG_CALL;
    initTestDb();
    fill_DkDbDefaultContent(QSqlDatabase::database(), false);
}

void test_letterSnippet::cleanup()
{   LOG_CALL;
    cleanupTestDb();
}

void test_letterSnippet::test_write_read_snippet()
{   LOG_CALL;
    const QString expected {qsl("20.Januar 2021")};
    snippet snip(letterSnippet::date);
    QVERIFY(snip.write(expected));
    QCOMPARE(snip.read (), expected);
}

void test_letterSnippet::test_overwrite_snippet()
{   LOG_CALL;
    snippet snip(letterSnippet::greeting);

    QVERIFY(snip.write(qsl("firstString")));
    const QString expected {qsl("expected")};
    QVERIFY(snip.write(expected));
    QCOMPARE(snip.read(), expected);
}

void test_letterSnippet::test_many_snippet_read_writes()
{
    const int nbrCreditors =100;
    saveRandomCreditors (nbrCreditors);
    const int iter =100;

    {
    dbgTimer timer(qsl("timer_test_many_snippet_read_writes"));
    QVector<snippet> v =randomSnippets (iter);
    int count =0;
    for(const auto& snip : qAsConst(v))
    {
        count++;
        QString text =qsl("test ") + QString::number(count);
        snip.write (text);
        QCOMPARE(text, snip.read());
    }
    }
}

void test_letterSnippet::test_snippet_type_dep_read()
{
    saveRandomCreditor ();
    const QString expected {qsl("expected")};
    snippet s1(letterSnippet::date);
    s1.write (expected);
    snippet s2(letterSnippet::date);
    QCOMPARE(expected, s2.read());

    snippet s3(letterSnippet::table);
    s3.write(expected);
    snippet s4(letterSnippet::table, letterType::annInterestInfoL);
    QCOMPARE(expected, s3.read());

    snippet s5(letterSnippet::address, letterType::all, 1);
    s5.write(expected);
    snippet s6(letterSnippet::address, letterType::annInterestInfoL, 1);
    QCOMPARE(expected, s6.read());
}

void test_letterSnippet::test_fallback()
{
    saveRandomCreditor ();
//    const QString expected {qsl("expceted")};

//   // not writing a default
    snippet s2(letterSnippet::address, letterType::annPayoutL, 1);
    QCOMPARE(qsl(""), s2.read ());



}
