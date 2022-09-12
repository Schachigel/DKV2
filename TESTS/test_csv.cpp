#include "test_csv.h"
#include "../DKV2/csvwriter.h"

void test_csv::test_empty_csv()
{
    csvwriter csv;
    QCOMPARE(csv.toString(), "");
}

void test_csv::test_csv_oneHeader()
{
    csvwriter csv;
    csv.addColumn("A");
    QCOMPARE(csv.toString(), "A");
}

void test_csv::test_csv_twoHeader()
{
    csvwriter csv;
    csv.addColumn("A");
    csv.addColumn("B");

    QCOMPARE(csv.toString(), "A; B");
}

void test_csv::test_csv_twoHeader_useAddColumns()
{
    csvwriter csv;
    csv.addColumns("A;B");

    QCOMPARE(csv.toString(), "A; B");
}

void test_csv::test_csv_twoHeader_useAddColumns_rm_space()
{
    csvwriter csv;
    csv.addColumns("  A  ;  B   ");

    QCOMPARE(csv.toString(), "A; B");
}

void test_csv::test_csv_oneHeader_oneRow()
{
    csvwriter csv;
    csv.addColumns("H");
    csv.appendToRow("R");

    QCOMPARE(csv.toString(), "H\nR");
}

void test_csv::test_csv_oneHeader_oneRow01()
{
    csvwriter csv;
    csv.addColumn("H");
    csv.appendToRow("R");

    QCOMPARE(csv.toString(), "H\nR");
}

void test_csv::test_csv_Headers_Rows()
{
    csvwriter csv;
    csv.addColumns("VN; NN; PLZ");
    csv.addRow(QList<QString>({"holger", "mairon", "69242"}));

    QCOMPARE(csv.toString(), "VN; NN; PLZ\nholger; mairon; 69242");
}

void test_csv::test_csv_Headers_Rows01()
{
    csvwriter csv;
    csv.addColumns("VN; NN; PLZ");
    csv.appendToRow("holger");
    csv.appendToRow("mairon");
    csv.appendToRow("69242");

    QCOMPARE(csv.toString(), "VN; NN; PLZ\nholger; mairon; 69242");
}

void test_csv::test_csv_fix_semicolon()
{
    csvwriter csv;
    csv.addColumn("VN;");
    csv.addColumn(";NN");
    csv.addColumn("P;LZ");

    csv.appendToRow(";holger");
    csv.appendToRow("mairon;");
    csv.appendToRow("69;242");

    QCOMPARE(csv.toString(), "VN,; ,NN; P,LZ\n,holger; mairon,; 69,242");
}

void test_csv::test_csv_Headers_Rows02()
{
    csvwriter csv;
    csv.addColumns("VN; NN; PLZ");
    csv.addRow("holger; mairon; 69242");
    csv.appendToRow("Lukas");
    csv.appendToRow("Felix;");
    csv.appendToRow("Oliv;er");

    QCOMPARE(csv.toString(), "VN; NN; PLZ\nholger; mairon; 69242\nLukas; Felix,; Oliv,er");
}
