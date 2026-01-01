
#include "test_csv.h"
#include "../DKV2/csvwriter.h"

void test_csv::test_sql_with_parameter_binding()
{
    {
        QSqlDatabase db =QSqlDatabase::addDatabase("QSQLITE");

        db.setDatabaseName(qsl(":memory:"));
        db.open();
        // clazy:exclude=unused-non-trivial-variable
        QString sqlCreateTable =QStringLiteral("CREATE TABLE testt (s TEXT, i INTEGER)");
        QSqlQuery qCreate; qCreate.prepare(sqlCreateTable);
        qCreate.exec();
    }
    // positional binding works
    {
        QString sqlInsertData =QStringLiteral("INSERT INTO testt (s, i) VALUES (?, ?)");
        QSqlQuery q;
        q.prepare (sqlInsertData);
        q.addBindValue (QVariant("text1"));
        q.addBindValue (QVariant(13));
        if( not q.exec ())
            qInfo() << sqlInsertData << q.boundValues () << q.lastError () << q.lastQuery ();
        q.addBindValue (QVariant("text2"));
        q.addBindValue (QVariant(14));
        if( not q.exec ())
            qInfo() << sqlInsertData << q.boundValues () << q.lastError () << q.lastQuery ();
    }
    // positional binding works with named parameter
    {
        QString sqlInsertData =QStringLiteral("INSERT INTO testt (s, i) VALUES (:p1, :p2)");
        QSqlQuery q;
        q.prepare (sqlInsertData);
        q.addBindValue (QVariant("text1"));
        q.addBindValue (QVariant(13));
        if( not q.exec ())
            qInfo() << sqlInsertData << q.boundValues () << q.lastError () << q.lastQuery ();
        q.addBindValue (QVariant("text2"));
        q.addBindValue (QVariant(14));
        if( not q.exec ())
            qInfo() << sqlInsertData << q.boundValues () << q.lastError () << q.lastQuery ();
    }
    // How about named binding?
    {
        QString sqlSelect =QStringLiteral("SELECT * FROM testt WHERE s=:val1 AND i=:val2");
        QSqlQuery qSelect;
        qSelect.prepare (sqlSelect);
        qSelect.bindValue (":val1", QVariant("text2"));
        qSelect.bindValue (":val2", QVariant(14));
        qInfo() << qSelect.boundValues ();
        if( qSelect.exec ()) {
            qInfo() << qSelect.lastQuery ();
            qSelect.first();
            qInfo() << qSelect.record ().value (0);
        } else
            qInfo() << sqlSelect << qSelect.boundValues () << qSelect.lastError () << qSelect.lastQuery ();
    }
    // change order: OK!
    {
        QString sqlSelect =QStringLiteral("SELECT * FROM testt WHERE s=:val1 AND i=:val2");
        QSqlQuery qSelect;
        qSelect.prepare (sqlSelect);
        qSelect.bindValue (":val2", QVariant(14));
        qSelect.bindValue (":val1", QVariant("text2"));
        qInfo() << qSelect.boundValues ();
        if( qSelect.exec ()) {
            qSelect.first();
            qInfo() << qSelect.record ().value (0);
        } else
            qInfo() << sqlSelect << qSelect.boundValues () << qSelect.lastError () << qSelect.lastQuery ();
    }
    // change placeholder mark to @ -> will not work
    {
        QString sqlSelect =QStringLiteral("SELECT * FROM testt WHERE s=@val1 AND i=@val2");
        QSqlQuery qSelect;
        qSelect.prepare (sqlSelect);
        qSelect.bindValue ("@val1", QVariant("text2"));
        qSelect.bindValue ("@val2", QVariant(14));
        qInfo() << qSelect.boundValues ();
        if( qSelect.exec ()) {
            qSelect.first();
            qInfo() << qSelect.record ().value (0);
        } else
            qInfo() << sqlSelect << qSelect.boundValues () << qSelect.lastError () << qSelect.lastQuery ();
    }
    // change placeholder mark to $ -> will not work
    {
        QString sqlSelect =QStringLiteral("SELECT * FROM testt WHERE s=$val1 AND i=$val2");
        QSqlQuery qSelect;
        qSelect.prepare (sqlSelect);
        qSelect.bindValue ("$val1", QVariant("text2"));
        qSelect.bindValue ("$val2", QVariant(14));
        qInfo() << qSelect.boundValues ();
        if( qSelect.exec ()) {
            qSelect.first();
            qInfo() << qSelect.record ().value (0);
        } else
            qInfo() << sqlSelect << qSelect.boundValues () << qSelect.lastError () << qSelect.lastQuery ();
    }
}
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

    QCOMPARE(csv.toString(), "A;B");
}

void test_csv::test_csv_twoHeader_useAddColumns()
{
    csvwriter csv;
    csv.addColumns("A;B");

    QCOMPARE(csv.toString(), "A;B");
}

void test_csv::test_csv_twoHeader_useAddColumns_rm_space()
{
    csvwriter csv;
    csv.addColumns("  A  ;  B   ");

    QCOMPARE(csv.toString(), "A;B");
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

    QCOMPARE(csv.toString(), "VN;NN;PLZ\nholger;mairon;69242");
}

void test_csv::test_csv_Headers_Rows01()
{
    csvwriter csv;
    csv.addColumns("VN; NN; PLZ");
    csv.appendToRow("holger");
    csv.appendToRow("mairon");
    csv.appendToRow("69242");

    QCOMPARE(csv.toString(), "VN;NN;PLZ\nholger;mairon;69242");
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

    QCOMPARE(csv.toString(), "VN,;,NN;P,LZ\n,holger;mairon,;69,242");
}

void test_csv::test_csv_Headers_Rows02()
{
    csvwriter csv;
    csv.addColumns("VN; NN; PLZ");
    csv.addRow("holger; mairon; 69242");
    csv.appendToRow("Lukas");
    csv.appendToRow("Felix;");
    csv.appendToRow("Oliv;er");

    QCOMPARE(csv.toString(), "VN;NN;PLZ\nholger;mairon;69242\nLukas;Felix,;Oliv,er");
}
