#include "test_htmlbrief.h"
#include "../DKV2/htmlbrief.h"

test_htmlbrief::test_htmlbrief(QObject *parent) : QObject(parent)
{
}

void test_htmlbrief::test_parseEmptyString()
{
    htmlTemplate brief;
    QVERIFY(brief.Positions().isEmpty());
}

void test_htmlbrief::test_parseString_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("out");

    QTest::newRow("Positon whole string") << "[[POS1]]" << "Position Text 1";
    QTest::newRow("Positons whole string") << "[[POS1]][[POS2]]" << "Position Text 1Pos-2";
    QTest::newRow("Positon start of string") << "[[POS1]] xxx " << "Position Text 1 xxx ";
    QTest::newRow("Positon end of string") << "xxx  [[POS2]]" << "xxx  Pos-2";
    QTest::newRow("Positon middle of string") << "xxx  [[POS1]]xxx" << "xxx  Position Text 1xxx";
    QTest::newRow("Positon start and end of string") << "[[POS1]]xxx[[POS2]]" << "Position Text 1xxxPos-2";
    QTest::newRow("two Positons in the middle of string") << " m m m [[POS1]] xx   \nx[[POS2]]\nx" << " m m m Position Text 1 xx   \nxPos-2\nx";
    QTest::newRow("many Positons") << "[[POS1]] m [[POS2]]m m [[POS3]] xx  [[POS4]] \nx[[POS5]]\nx" << "Position Text 1 m Pos-2m m  xx   \nx\nx";
}

void test_htmlbrief::test_parseString()
{
    QFETCH (QString, input);
    QFETCH (QString, out);

    htmlTemplate brief(input);
    brief.setPositionText("POS1", "Position Text 1");
    brief.setPositionText("POS2", "Pos-2");

    QCOMPARE( brief.out(), out);
}
