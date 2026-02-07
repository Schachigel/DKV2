#include "test_csv.h"

#include "../DKV2/csvwriter.h"

#include <QtTest/QTest>

void test_csv::test_toString_noHeader_wTrimming_data()
{
    QTest::addColumn<QList<QList<QString>>>( "records");
    QTest::addColumn<QString>("expectedFileContent");

    struct testdata { QList<QList<QString>> d; QString e; QString c;};

    testdata data[] {
        { {{}},                {""} , {"empty file"} },
        { {{{"D"}}},           {"D"} , {"one field"} },
        { {{{"D\"D"}}},        {"\"D\"\"D\""} , {"one field, quote inside"} },
        { {{{"D","E"}}},       {"D;E"} , {"two fields"} },
        { {{{"D","E"," F "}}}, {"D;E;F"} , {"three fields"} },
        { {{{""},{""}}},       {"\"\";\"\""}, {"two fields of nothing (there could not be one field of nothing)"} },
        { {{{" D ","  E "," F  "}}},   {"D;E;F"} , {"three fields, lot of spaces"} },
        { {{{"D"}},{{"E"}}},           {"D\r\nE"} , {"two rows one filed each"} },
        { {{{"  D  "}},{{"  E    "}}}, {"D\r\nE"} , {"two rows one filed each lot of spaces"} },
        { {{{"D"}},{{"E"}},{{"F"}}},   {"D\r\nE\r\nF"} , {"three rows one filed each"} },
        { {{{"D"},{"E", "E+"},{"F"}}}, {"D\r\nE;E+\r\nF"} , {"three rows one, two, one fileds"} },
        { {{{"D  ;"}},{{" E "}, {"  ;E+"}},{"F"}}, {"\"D  ;\"\r\nE;\";E+\"\r\nF"} , {"three rows one, two, one fileds,quoting"} },
        { {{{"D"," \" E \" "," F "}}}, {"D;\"\"\" E \"\"\";F"} , {"no trimming in quoted strings"} }, // THREE quotes: quote the field AND double any quote in the field
    };

    for(const auto& tdata : data) {
        QTest::newRow(tdata.c.toLocal8Bit()) << tdata.d << tdata.e;
    }
}
void test_csv::test_toString_noHeader_wTrimming()
{
    QFETCH(QList<QList<QString>>, records);
    QFETCH(QString, expectedFileContent);

    CsvWriter csv;
    for( const auto& row : records) {
        for( const auto& field : row)
            csv.appendValueToNextRecord(field);
        csv.startNextRecord();
    }
    QCOMPARE(csv.toString(), expectedFileContent);
}

void test_csv::test_toString_noHeader_NO_Trimming_data()
{
    QTest::addColumn<QList<QList<QString>>>( "records");
    QTest::addColumn<QString>("expectedFileContent");

    struct testdata { QList<QList<QString>> d; QString e; QString c;};

    testdata data[] {
        { {{}},                        {""} ,                             {"empty file"} },
        { {{{"D"}}},                   {"D"},                             {"one field"} },
        { {{{R"str(D"D)str"}}},        {R"str("D""D")str"} ,              {"one field, quote inside"} }, // quotes get doubled; strings w quotes inside get quoted
        { {{{"D","E"}}},               {"D;E"} ,                          {"two fields"} },
        { {{{"D","E"," F "}}},     {R"str(D;E;" F ")str"} ,             {"three fields"} },  // strings w spaces get qupted
        { {{{" ",""}}},                {R"str(" ";"")str"},                 {"two fields of nothing (there could not be one field of nothing)"} },
        { {{{" D ","  E "," F  "}}},   {R"str(" D ";"  E ";" F  ")str"} ,       {"three fields, lot of spaces"} },
        { {{{"D"}},{{"E"}}},           {"D\r\nE"} ,                       {"two rows one filed each"} },
        { {{{"  D  "}},{{"  E    "}}}, {R"str("  D  ")str" "\r\n" R"str("  E    ")str"} ,  {"two rows one filed each lot of spaces"} },
        { {{{"D"}},{{"E"}},{{"F"}}},   {"D\r\nE\r\nF"} ,                               {"three rows one filed each"} },
        { {{{"D"},{"E", "E+"},{" F "}}},           {"D\r\nE;E+\r\n\" F \""} ,              {"three rows one, two, one fileds"} },
        { {{{"D  ;"}},{{" E ", "  ;E+"}},{"F"}}, {"\"D  ;\"\r\n\" E \";\"  ;E+\"\r\nF"} ,{"three rows one, two, one fileds,quoting"} },
        { {{{"D"," \" E \" "," F "}}},             {R"str(D;" "" E "" ";" F ")str"} ,    {"no trimming in quoted strings"} }, // THREE quotes: quote the field AND double any quote in the field
    };

    for(const auto& tdata : data) {
        QTest::newRow(tdata.c.toLocal8Bit()) << tdata.d << tdata.e;
    }
}
void test_csv::test_toString_noHeader_NO_Trimming()
{
    QFETCH(QList<QList<QString>>, records);
    QFETCH(QString, expectedFileContent);

    CsvWriter csv(';', '\"', CsvWriter::TrimInput::NoTrimming);
    for( const auto& row : records) {
        for( const auto& field : row)
            csv.appendValueToNextRecord(field);
        csv.startNextRecord();
    }
    QCOMPARE(csv.toString(), expectedFileContent);
}

void test_csv::test_toString_wHeader_wTrimming_data()
{
    QTest::addColumn<QList<QString>>( "headers");
    QTest::addColumn<QList<QList<QString>>>( "records");
    QTest::addColumn<QString>("expectedFileContent");

    struct testdata { QList<QString> h; QList<QList<QString>> d; QString e; QString c;};

    testdata data[] {
        // { {{""}},{{""}}, {";"} , {"one row w one empty field"} }, //not possible w/o call to "startNextRecord"
        { {{"H"}},          {}, {"H"}, {"one header only"} }, //
        { {{" H  "}},       {}, {"H"} , {"header with spaces"} }, //
        { {{" h\" H \" "}}, {}, {"\"h\"\" H \"\"\""} , {"quoted header with inner n outer spaces 1"} },
        { {{" \" H \" "}},  {}, {"\"\"\" H \"\"\""} , {"quoted header with inner n outer spaces 2"} },// note: inner quote gets quoted AND string gets quoted because it contains quotes
        { {{"H"}},{{{"D"}}}, {"H\r\nD"}, {"one header, one row, one field"} }, //
        { {{"A"},{"B"},{"C"}}, {{"D","\" E \""," F "}}, {"A;B;C\r\nD;\"\"\" E \"\"\";F"} , {"no trimming in quoted strings"} },
        { {{{"FieldA"}, {"FieldB"}, {"FieldC"}, {"FieldD"}, {"FieldE"}, {"FieldF"}, {"FieldG"}, {"FieldH"}, {"FieldI"}}},
         {{{"DATA01"}, {"DATA02"}, {"DATA03"}, {"DATA04"}, {"DATA05"}, {"DATA06"}, {"DATA07"}, {"DATA08"}, {"DATA09"}},
          {{"DATA01"}, {"DATA02"}, {"DATA03"}, {"DATA04"}, {"DATA05"}, {"DATA06"}, {"DATA07"}, {"DATA08"}, {"DATA09"}},
          {{"DATA01"}, {"DATA02"}, {"DATA03"}, {"DATA04"}, {"DATA05"}, {"DATA06"}, {"DATA07"}, {"DATA08"}, {"DATA09"}},
          {{"DATA01"}, {"DATA02"}, {"DATA03"}, {"DATA04"}, {"DATA05"}, {"DATA06"}, {"DATA07"}, {"DATA08"}, {"DATA09"}},
          {{"DATA01"}, {"DATA02"}, {"DATA03"}, {"DATA04"}, {"DATA05"}, {"DATA06"}, {"DATA07"}, {"DATA08"}, {"DATA09"}},
          {{"DATA01"}, {"DATA02"}, {"DATA03"}, {"DATA04"}, {"DATA05"}, {"DATA06"}, {"DATA07"}, {"DATA08"}, {"DATA09"}},},
         {"FieldA;FieldB;FieldC;FieldD;FieldE;FieldF;FieldG;FieldH;FieldI\r\nDATA01;"
          "DATA02;DATA03;DATA04;DATA05;DATA06;DATA07;DATA08;DATA09\r\n"
          "DATA01;DATA02;DATA03;DATA04;DATA05;DATA06;DATA07;DATA08;DATA09\r\n"
          "DATA01;DATA02;DATA03;DATA04;DATA05;DATA06;DATA07;DATA08;DATA09\r\n"
          "DATA01;DATA02;DATA03;DATA04;DATA05;DATA06;DATA07;DATA08;DATA09\r\n"
          "DATA01;DATA02;DATA03;DATA04;DATA05;DATA06;DATA07;DATA08;DATA09\r\n"
          "DATA01;DATA02;DATA03;DATA04;DATA05;DATA06;DATA07;DATA08;DATA09"}}
     };

    for(const auto& tdata : data) {
        QTest::newRow(tdata.c.toLocal8Bit()) << tdata.h << tdata.d << tdata.e;
    }
}
void test_csv::test_toString_wHeader_wTrimming()
{
    QFETCH(QList<QString>, headers);
    QFETCH(QList<QList<QString>>, records);
    QFETCH(QString, expectedFileContent);

    CsvWriter csv;
    for( const auto& header : headers)
        csv.addColumn(header);

    for( const auto& row : records)
        for( const auto& field : row)
            csv.appendValueToNextRecord(field);
    QCOMPARE(csv.toString(), expectedFileContent);
}

void test_csv::test_toString_newlines_in_fields_wTrimming_data()
{
    QTest::addColumn<QList<QList<QString>>>("records");
    QTest::addColumn<QString>("expectedFileContent");

    struct testdata { QList<QList<QString>> d; QString e; QString c; };

    testdata data[] {
                    { {{"A","B\nC","D"}},    "A;\"B\nC\";D",    "LF inside field gets quoted" },
                    { {{"A","B\r\nC","D"}},  "A;\"B\r\nC\";D",  "CRLF inside field gets quoted" },
                    { {{"A","\n","D"}},      "A;\"\";D",        "field is just LF -> trimmed to empty" },
                    { {{"A","\r\n","D"}},    "A;\"\";D",        "field is just CRLF -> trimmed to empty" },
                    };

    for (const auto& tdata : data)
        QTest::newRow(tdata.c.toLocal8Bit()) << tdata.d << tdata.e;
}

void test_csv::test_toString_newlines_in_fields_wTrimming()
{
    QFETCH(QList<QList<QString>>, records);
    QFETCH(QString, expectedFileContent);

    CsvWriter csv;
    for (const auto& row : records) {
        for (const auto& field : row)
            csv.appendValueToNextRecord(field);
        csv.startNextRecord();
    }
    QCOMPARE(csv.toString(), expectedFileContent);
}

void test_csv::test_toString_tortureField_combo_data()
{
    QTest::addColumn<CsvWriter::TrimInput>("trim");
    QTest::addColumn<QList<QList<QString>>>("records");
    QTest::addColumn<QString>("expectedFileContent");

    struct testdata { CsvWriter::TrimInput t; QList<QList<QString>> d; QString e; QString c; };

    const QString field = QStringLiteral(" x;\"y\"\r\nz "); // spaces + ; + quotes + CRLF

    testdata data[] {
                    // TrimWhitespace: outer spaces trimmed, still must quote due to ;, quotes, CRLF
                    { CsvWriter::TrimInput::TrimWhitespace,
                     {{ "A", field, "D" }},
                     "A;\"x;\"\"y\"\"\r\nz\";D",
                     "TrimWhitespace: combo field trimmed + quoted + doubled quotes" },

                    // NoTrimming: outer spaces kept => still quoted, spaces preserved
                    { CsvWriter::TrimInput::NoTrimming,
                     {{ "A", field, "D" }},
                     "A;\" x;\"\"y\"\"\r\nz \";D",
                     "NoTrimming: combo field preserved + quoted + doubled quotes" },
                    };

    for (const auto& tdata : data)
        QTest::newRow(tdata.c.toLocal8Bit()) << tdata.t << tdata.d << tdata.e;
}

void test_csv::test_toString_tortureField_combo()
{
    QFETCH(CsvWriter::TrimInput, trim);
    QFETCH(QList<QList<QString>>, records);
    QFETCH(QString, expectedFileContent);

    CsvWriter csv(';', '"', trim);
    for (const auto& row : records) {
        for (const auto& field : row)
            csv.appendValueToNextRecord(field);
        csv.startNextRecord();
    }
    QCOMPARE(csv.toString(), expectedFileContent);
}

void test_csv::test_toString_quoteAllFields_data()
{
    QTest::addColumn<QList<QList<QString>>>("records");
    QTest::addColumn<QString>("expectedFileContent");

    struct testdata { QList<QList<QString>> d; QString e; QString c; };

    testdata data[] {
                    { {{"A","B",""}},     "\"A\";\"B\";\"\"",  "all fields quoted incl empty" },
                    { {{"D\"D"}},         "\"D\"\"D\"",        "all fields quoted + inner quotes doubled" },
                    { {{";","\n"," "}},   "\";\";\"\";\"\"",   "separator/newline/space trimmed to empty" },
                    };

    for (const auto& tdata : data)
        QTest::newRow(tdata.c.toLocal8Bit()) << tdata.d << tdata.e;
}

void test_csv::test_toString_quoteAllFields()
{
    QFETCH(QList<QList<QString>>, records);
    QFETCH(QString, expectedFileContent);

    CsvWriter csv(';', '"', CsvWriter::TrimInput::TrimWhitespace, CsvWriter::QuoteMode::AllFields);
    for (const auto& row : records) {
        for (const auto& field : row)
            csv.appendValueToNextRecord(field);
        csv.startNextRecord();
    }
    QCOMPARE(csv.toString(), expectedFileContent);
}

void test_csv::test_toString_emits_unfinished_last_record()
{
    CsvWriter csv;
    csv.appendValueToNextRecord("A");
    csv.appendValueToNextRecord("B");

    // no startNextRecord() call
    QCOMPARE(csv.toString(), QString("A;B"));
}

void test_csv::test_toString_withHeader_incomplete_last_row_is_written()
{
    CsvWriter csv;
    csv.addColumns({ "A", "B", "C" });

    csv.appendValueToNextRecord("1");
    csv.appendValueToNextRecord("2"); // incomplete (missing "C")

    // Current behavior: header + incomplete row written
    QCOMPARE(csv.toString(), QString("A;B;C\r\n1;2"));
}

void test_csv::test_toString_whitespace_only_fields_data()
{
    QTest::addColumn<CsvWriter::TrimInput>("trim");
    QTest::addColumn<QList<QList<QString>>>("records");
    QTest::addColumn<QString>("expectedFileContent");

    struct testdata { CsvWriter::TrimInput t; QList<QList<QString>> d; QString e; QString c; };

    testdata data[] {
                    // TrimWhitespace: whitespace-only becomes empty => ""
                    { CsvWriter::TrimInput::TrimWhitespace, {{ "   ", "\t", " \t " }}, "\"\";\"\";\"\"", "TrimWhitespace: whitespace-only -> empty fields" },

                    // NoTrimming: outer whitespace preserved => quoted (Excel/Calc safe)
                    { CsvWriter::TrimInput::NoTrimming, {{ "   ", "\t", " \t " }}, "\"   \";\"\t\";\" \t \"", "NoTrimming: whitespace-only preserved & quoted" },
                    };

    for (const auto& tdata : data)
        QTest::newRow(tdata.c.toLocal8Bit()) << tdata.t << tdata.d << tdata.e;
}

void test_csv::test_toString_whitespace_only_fields()
{
    QFETCH(CsvWriter::TrimInput, trim);
    QFETCH(QList<QList<QString>>, records);
    QFETCH(QString, expectedFileContent);

    CsvWriter csv(';', '"', trim);
    for (const auto& row : records) {
        for (const auto& field : row)
            csv.appendValueToNextRecord(field);
        csv.startNextRecord();
    }
    QCOMPARE(csv.toString(), expectedFileContent);
}

void test_csv::test_toString_unicode_fields()
{
    CsvWriter csv;
    csv.appendValueToNextRecord(QStringLiteral("Müller"));
    csv.appendValueToNextRecord(QStringLiteral("😀"));
    csv.startNextRecord();

    QCOMPARE(csv.toString(), QStringLiteral("Müller;😀"));
}

void test_csv::test_toString_custom_separator_comma()
{
    CsvWriter csv(',', '"', CsvWriter::TrimInput::TrimWhitespace);

    csv.appendValueToNextRecord("A,B");  // contains separator => must quote
    csv.appendValueToNextRecord("C");
    csv.startNextRecord();

    QCOMPARE(csv.toString(), QString("\"A,B\",C"));
}
