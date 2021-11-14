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
    QRandomGenerator* rand =QRandomGenerator::system ();
    const int nbrCreditors =10;
    saveRandomCreditors (nbrCreditors);
    const int iter =100;

    dbgTimer timer(qsl("test_many_snippet_read_writes"));
    for(int i=0; i<iter; i++)
    {
        const letterType lid =letterTypeFromInt(rand->bounded (100)%int(letterType::maxValue));
        const letterSnippet sid =letterSnippetFromInt(rand->bounded (200)%int(letterSnippet::maxValue));
        const qlonglong kid =rand->bounded (nbrCreditors);
        QUuid uuid =QUuid::createUuid ();
        const QString text =uuid.toString ();
        snippet snip(sid, lid, kid);
        snip.write (text);
        QCOMPARE(text, snip.read());
    }
}

