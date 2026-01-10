
#include "test_csv.h"
#include "../DKV2/csvwriter.h"
#include <QtTest/QTest>
#include <QDebug>


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
        { {{{""},{""}}},       {";;"}, {"three fields of nothing (there could not be one field of nothing)"} },
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

    csvWriter csv;
    for( const auto& row : records) {
        for( const auto& field : row)
            csv.appendValueToNextRecord(field);
        csv.startNextRecord();
    }
    //    qDebug() << comment << "\n" << headers << "\n" << records;
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
        { {{{"D"},{"E"}}},             {"D;E"} ,                          {"two fields"} },
        { {{{"D"},{"E"},{" F "}}},     {R"str(D;E;" F ")str"} ,           {"three fields"} },  // strings w spaces get qupted
        { {{{" "},{""}}},              {R"str(" ";)str"},                 {"two fields of nothing (there could not be one field of nothing)"} },
        { {{{" D ","  E "," F  "}}},   {R"str(" D ";"  E ";" F  ")str"} , {"three fields, lot of spaces"} },
        { {{{"D"}},{{"E"}}},           {"D\r\nE"} ,                       {"two rows one filed each"} },
        { {{{"  D  "}},{{"  E    "}}}, {R"str("  D  ")str" "\r\n" R"str("  E    ")str"} ,  {"two rows one filed each lot of spaces"} },
        { {{{"D"}},{{"E"}},{{"F"}}},   {"D\r\nE\r\nF"} ,                                    {"three rows one filed each"} },
        { {{{"D"},{"E", "E+"},{" F "}}},           {"D\r\nE;E+\r\n\" F \""} ,               {"three rows one, two, one fileds"} },
        { {{{"D  ;"}},{{" E "}, {"  ;E+"}},{"F"}}, {"\"D  ;\"\r\n\" E \";\"  ;E+\"\r\nF"} , {"three rows one, two, one fileds,quoting"} },
        { {{{"D"," \" E \" "," F "}}},             {R"str(D;" "" E "" ";" F ")str"} ,       {"no trimming in quoted strings"} }, // THREE quotes: quote the field AND double any quote in the field
    };

    for(const auto& tdata : data) {
        QTest::newRow(tdata.c.toLocal8Bit()) << tdata.d << tdata.e;
    }
}
void test_csv::test_toString_noHeader_NO_Trimming()
{
    QFETCH(QList<QList<QString>>, records);
    QFETCH(QString, expectedFileContent);

    csvWriter csv(";", "\"", trim_input::no_trimming);
    for( const auto& row : records) {
        for( const auto& field : row)
            csv.appendValueToNextRecord(field);
        csv.startNextRecord();
    }
    //    qDebug() << comment << "\n" << headers << "\n" << records;
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

    csvWriter csv;
    for( const auto& header : headers)
        csv.addColumn(header);

    for( const auto& row : records)
        for( const auto& field : row)
            csv.appendValueToNextRecord(field);
//    qDebug() << comment << "\n" << headers << "\n" << records;
    QCOMPARE(csv.toString(), expectedFileContent);
}

// void test_csv::test_empty_csv()
// {
//     csvWriter csv;
//     QCOMPARE(csv.toString(), "");
// }

// void test_csv::test_csv_oneHeader_withSpace()
// {
//     csvWriter csv;
//     csv.addColumn(" mSpaces  ");
//     QCOMPARE( csv.toString(), "mSpaces");
// }
// void test_csv::test_csv_twoHeader()
// {
//     csvWriter csv;
//     csv.addColumn("A");
//     csv.addColumn("B");

//     QCOMPARE(csv.toString(), "A;B");
// }

// void test_csv::test_csv_twoHeader_useAddColumns()
// {
//     csvWriter csv;
//     csv.addColumns({"A","B"});

//     QCOMPARE(csv.toString(), "A;B");
// }

// void test_csv::test_csv_twoHeader_useAddColumns_rm_space()
// {
//     csvWriter csv;
//     csv.addColumns({" A  ","  B   "});

//     QCOMPARE(csv.toString(), "A;B");
// }

// void test_csv::test_csv_oneHeader_oneRow()
// {
//     csvWriter csv;
//     csv.addColumns({"H"});
//     csv.appendValueToNextRecord("R");

//     QCOMPARE(csv.toString(), "H\r\nR");
// }

// void test_csv::test_csv_oneHeader_oneRow01()
// {
//     csvWriter csv;
//     csv.addColumn("H");
//     csv.appendValueToNextRecord("R");

//     QCOMPARE(csv.toString(), "H\r\nR");
// }

// void test_csv::test_csv_Headers_Rows()
// {
//     csvWriter csv;
//     csv.addColumns({"VN", "NN","PLZ"});
//     csv.appendRecord(QList<QString>({"holger", "mairon", "69242"}));

//     QCOMPARE(csv.toString(), "VN;NN;PLZ\r\nholger;mairon;69242");
// }

// void test_csv::test_csv_Headers_Rows01()
// {
//     csvWriter csv;
//     csv.addColumns({"VN", "NN","PLZ"});
//     csv.appendValueToNextRecord("holger");
//     csv.appendValueToNextRecord("mairon");
//     csv.appendValueToNextRecord("69242");

//     QCOMPARE(csv.toString(), "VN;NN;PLZ\r\nholger;mairon;69242");
// }

// void test_csv::test_csv_fix_semicolon()
// {
//     csvWriter csv;
//     csv.addColumn("VN;");
//     csv.addColumn(";NN");
//     csv.addColumn("P;LZ");

//     csv.appendValueToNextRecord(";holger");
//     csv.appendValueToNextRecord("mairon;");
//     csv.appendValueToNextRecord("69;242");

//     QCOMPARE(csv.toString(), "\"VN;\";\";NN\";\"P;LZ\"\r\n\";holger\";\"mairon;\";\"69;242\"");
// }

// void test_csv::test_csv_Headers_Rows02()
// {
//     csvWriter csv;
//     csv.addColumns({"VN", "NN","PLZ"});
//     csv.appendRecord({"holger", " mairon", "69242"});
//     csv.appendValueToNextRecord("Lukas");
//     csv.appendValueToNextRecord("Felix;");
//     csv.appendValueToNextRecord("Oliv;er");

//     QCOMPARE(csv.toString(), "VN;NN;PLZ\r\nholger;mairon;69242\r\nLukas;\"Felix;\";\"Oliv;er\"");
// }
