#include "test_htmlbrief.h"
#include "../DKV2/htmlbrief.h"

test_htmlbrief::test_htmlbrief(QObject *parent) : QObject(parent)
{
}

void test_htmlbrief::test_parseEmptyString()
{
    htmlTemplate brief;
    QVERIFY(brief.getPositions().isEmpty());
}

void test_htmlbrief::test_parseString_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("nbr");
    QTest::addColumn<QStringList>("result");

    QTest::newRow("Positon whole string") << "[[POSITION]]" << 1 << QStringList ("[[POSITION]]");
    QTest::newRow("Positon start of string") << "[[POSITION]] xxx " << 1 << QStringList ("[[POSITION]]");
    QTest::newRow("Positon end of string") << "xxx  [[POSITION]]" << 1 << QStringList ("[[POSITION]]");
    QTest::newRow("Positon middle of string") << "xxx  [[POSITION]]xxx" << 1 << QStringList ("[[POSITION]]");
    QTest::newRow("Positon start and end of string") << "[[POS01]]xxx[[POS02]]" << 2 << QStringList ({"[[POS01]]","[[POS02]]"} );
}

void test_htmlbrief::test_parseString()
{
    QFETCH (QString, input);
    QFETCH (int, nbr);
    QFETCH (QStringList, result);

    htmlTemplate brief(input);
    QCOMPARE( brief.getPositions().size(), nbr);
    QCOMPARE( brief.getPositions(), result);

}
