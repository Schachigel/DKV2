#include <qtest.h>
#include <QString>
#include <QDebug>

#include "../DKV2/finhelper.h"
#include "test_finance.h"

void test_finance::test_TageBisJahresende_data()
{
    QTest::addColumn<QDate>("date");
    QTest::addColumn<int>("tageBisJe_excel");
    bool singleTestForDebugging = false;
    if( singleTestForDebugging)
    {
        QDate testDatum(2017, 1, 1);
        QTest::newRow( (QString("Bis JE vom ") + testDatum.toString()).toLocal8Bit().data())
                << testDatum << TageBisJahresende(testDatum);
    }
    else
    {
        QDate start(2015, 12, 30);
        for (int i=0; i < 365+366; i++)
        {
            QDate testDatum(start.addDays(i));
            QTest::newRow( (QString("Bis JE vom ") + testDatum.toString()).toLocal8Bit().data())
                    << testDatum << TageBisJahresende(testDatum);
        }
    }
}
void test_finance::test_TageBisJahresende()
{
    QFETCH(QDate, date);
    QFETCH(int, tageBisJe_excel);
    QCOMPARE( TageBisJahresende_a(date), tageBisJe_excel);
}

void test_finance::test_TageSeitJahresanfang_data()
{
    QTest::addColumn<QDate>("date");
    QTest::addColumn<int>("tageSeitJa_excel");
    bool singleTestForDebugging = false;
    if( singleTestForDebugging)
    {
        QDate testDatum(2017, 1, 1);
        QTest::newRow( (QString("Seit JA bis ") + testDatum.toString()).toLocal8Bit().data())
                << testDatum << TageSeitJahresAnfang(testDatum);
    }
    else
    {
        QDate start(2015, 12, 30);
        for (int i=0; i < 365+366; i++)
        {
            QDate testDatum(start.addDays(i));
            QTest::newRow( (QString("Seit JA bis ") + testDatum.toString()).toLocal8Bit().data())
                    << testDatum << TageSeitJahresAnfang(testDatum);
        }
    }

}
void test_finance::test_TageSeitJahresanfang()
{
    QFETCH(QDate, date);
    QFETCH(int, tageSeitJa_excel);
    QCOMPARE( TageSeitJahresAnfang_a(date), tageSeitJa_excel);

}

void test_finance::test_TageZwischen_data()
{
    struct VonBisTage
    {
        QDate von;
        QDate bis;
        int tage;
    } testdata[]{

    // leap year
        // start at BoY
    {QDate(2016, 1,  1), QDate(2016,  1,  2),  1},
    {QDate(2016, 1,  1), QDate(2016,  1, 31), 29},
    {QDate(2016, 1,  1), QDate(2016,  2,  1), 30},
    {QDate(2016, 1,  1), QDate(2016,  2, 15), 44},
        // ends around year end
    {QDate(2016, 1,  1), QDate(2016, 12, 29), 358},
    {QDate(2016, 1,  1), QDate(2016, 12, 30), 359},
    {QDate(2016, 1,  1), QDate(2016, 12, 31), 359},
    {QDate(2016, 3,  3), QDate(2016, 12, 29), 296},
    {QDate(2016, 3,  3), QDate(2016, 12, 30), 297},
    {QDate(2016, 3,  3), QDate(2016, 12, 31), 297},
        // starts around end of feb
    {QDate(2016, 2, 28), QDate(2016,  2, 29),  1},
    {QDate(2016, 2, 28), QDate(2016,  3,  1),  3},
    {QDate(2016, 2, 28), QDate(2016,  3, 13), 15},
    {QDate(2016, 2, 29), QDate(2016,  3,  1),  2},
    {QDate(2016, 2, 29), QDate(2016,  3, 13), 14},
        // end around end of feb
    {QDate(2016, 1,  5), QDate(2016,  2, 28), 53},
    {QDate(2016, 1,  5), QDate(2016,  2, 29), 54},
    {QDate(2016, 1,  5), QDate(2016,  3,  1), 56},
        // starts around 30 day month
    {QDate(2016, 4,  29), QDate(2016,  11,  9), 190},
    {QDate(2016, 4,  30), QDate(2016,  11,  9), 189},
    {QDate(2016, 5,   1), QDate(2016,  11,  9), 188},
        // ends around 30 day month
    {QDate(2016, 1,  5), QDate(2016,  4, 29), 114},
    {QDate(2016, 1,  5), QDate(2016,  4, 30), 115},
    {QDate(2016, 1,  5), QDate(2016,  5,  1), 116},
        // starts around 31 day month
    {QDate(2016, 8,  30), QDate(2016,  11,  9), 69},
    {QDate(2016, 8,  31), QDate(2016,  11,  9), 69},
    {QDate(2016, 9,   1), QDate(2016,  11,  9), 68},
        // ends around 31 day month
    {QDate(2016, 1,  5), QDate(2016,  5, 30), 145},
    {QDate(2016, 1,  5), QDate(2016,  5, 31), 145},
    {QDate(2016, 1,  5), QDate(2016,  6,  1), 146},

    // NOT ly
        // start at BoY
    {QDate(2017, 1,  1), QDate(2017,  1,  2),  1},
    {QDate(2017, 1,  1), QDate(2017,  1, 31), 29},
    {QDate(2017, 1,  1), QDate(2017,  2,  1), 30},
    {QDate(2017, 1,  1), QDate(2017,  2, 15), 44},
        // ends around year end
    {QDate(2017, 1,  1), QDate(2017, 12, 29), 358},
    {QDate(2017, 1,  1), QDate(2017, 12, 30), 359},
    {QDate(2017, 1,  1), QDate(2017, 12, 31), 359},
    {QDate(2017, 3,  3), QDate(2017, 12, 29), 296},
    {QDate(2017, 3,  3), QDate(2017, 12, 30), 297},
    {QDate(2017, 3,  3), QDate(2017, 12, 31), 297},
        // starts around end of feb
    {QDate(2017, 2, 28), QDate(2017,  3,  1),  3},
    {QDate(2017, 2, 28), QDate(2017,  3, 13), 15},
        // end around end of feb
    {QDate(2017, 1,  5), QDate(2017,  2, 28), 53},
    {QDate(2017, 1,  5), QDate(2017,  3,  1), 56},
        // starts around 30 day month
    {QDate(2017, 4,  29), QDate(2017,  11,  9), 190},
    {QDate(2017, 4,  30), QDate(2017,  11,  9), 189},
    {QDate(2017, 5,   1), QDate(2017,  11,  9), 188},
        // ends around 30 day month
    {QDate(2017, 1,  5), QDate(2017,  4, 29), 114},
    {QDate(2017, 1,  5), QDate(2017,  4, 30), 115},
    {QDate(2017, 1,  5), QDate(2017,  5,  1), 116},
        // starts around 31 day month
    {QDate(2017, 8,  30), QDate(2017,  11,  9), 69},
    {QDate(2017, 8,  31), QDate(2017,  11,  9), 69},
    {QDate(2017, 9,   1), QDate(2017,  11,  9), 68},
        // ends around 31 day month
    {QDate(2017, 1,  5), QDate(2017,  5, 30), 145},
    {QDate(2017, 1,  5), QDate(2017,  5, 31), 145},
    {QDate(2017, 1,  5), QDate(2017,  6,  1), 146},
    {QDate(), QDate(), -1}
    };

    QTest::addColumn<QDate>("von");
    QTest::addColumn<QDate>("bis");
    QTest::addColumn<int>  ("tageZwischenLt_excel");

    int i = 0;
    do
    {
        QDate von(testdata[i].von);
        QDate bis(testdata[i].bis);
        int  tage(testdata[i].tage);
        QTest::newRow((QString("Tage zw. ")+von.toString() +" und " + bis.toString()).toLocal8Bit().data())
            << von << bis << tage;
    }while(testdata[++i].tage != -1);

}
void test_finance::test_TageZwischen()
{
    QFETCH(QDate, von);
    QFETCH(QDate, bis);
    QFETCH(int, tageZwischenLt_excel);
    QCOMPARE(tageZwischenLt_excel, TageZwischen(von, bis));
}

void test_finance::test_ZinsesZins_data()
{
    QTest::addColumn<QDate>( "von");
    QTest::addColumn<QDate>( "bis");
    QTest::addColumn<double>("zinssatz");
    QTest::addColumn<double>("wert");
    QTest::addColumn<double>("ZinsTesauriert");
    QTest::addColumn<double>("Zins");

    struct testdata
    {
        QDate von; QDate bis;
        double zinssatz; double wert;
        double ZinsTesauriert; double Zins;
    };
    testdata d[]={
        {QDate(2015, 12, 31), QDate(2017,  1,  1), 10., 100., 10.03 , 10.03 },
        {QDate(2016,  1, 31), QDate(2017,  2,  1), 10., 100., 10.08 , 10.03 },
        {QDate(2016,  2, 29), QDate(2017,  3,  1), 10., 100., 10.14 , 10.03 },
        {QDate(2016,  3, 31), QDate(2017,  4,  1), 10., 100., 10.19 , 10.03 },
        {QDate(2016,  4, 30), QDate(2017,  5,  1), 10., 100., 10.23 , 10.03 },
        {QDate(2016,  5, 31), QDate(2017,  6,  1), 10., 100., 10.24 , 10.03 },
        {QDate(2016,  6, 30), QDate(2017,  7,  1), 10., 100., 10.25 , 10.03 },
        {QDate(2016,  7, 31), QDate(2017,  8,  1), 10., 100., 10.25 , 10.03 },
        {QDate(2016,  8, 31), QDate(2017,  9,  1), 10., 100., 10.22 , 10.03 },
        {QDate(2016,  9, 30), QDate(2017, 10,  1), 10., 100., 10.19 , 10.03 },
        {QDate(2016, 10, 31), QDate(2017, 11,  1), 10., 100., 10.14 , 10.03 },
        {QDate(2016, 11, 30), QDate(2017, 12,  1), 10., 100., 10.07 , 10.03 },


        {QDate(2016, 12, 31), QDate(2017,  6, 30), 1., 100., 0.50 , 0.50 },
        {QDate(2016, 12, 31), QDate(2018,  6, 30), 1., 100., 1.51 , 1.50 },
        {QDate(2015, 12, 31), QDate(2018,  1,  1), 1., 100., 2.01 , 2.00 },
        {QDate(2016, 12, 31), QDate(2019,  6, 30), 1., 100., 2.52 , 2.50 },
        {QDate(2015, 12, 31), QDate(2019,  1,  1), 1., 100., 3.03 , 3.00 },
        {QDate(2016, 12, 31), QDate(2020,  6, 30), 1., 100., 3.55 , 3.50 },
        {QDate(2016,  6, 31), QDate(2020, 12, 31), 1., 100., 3.56 , 3.50 },
        {QDate(2015, 12, 31), QDate(2020,  1,  1), 1., 100., 4.06 , 4.00 },
        {QDate(2016, 12, 31), QDate(2021,  6, 30), 1., 100., 4.58 , 4.50 },
        {QDate(2015, 12, 31), QDate(2021,  1,  1), 1., 100., 5.10 , 5.00 },

        {QDate(), QDate(), 0., 0., 0., 0.}
    };

    int l= 0;
    do
    {
        QTest::newRow(QString().toLocal8Bit().data())
            << d[l].von << d[l].bis << d[l].zinssatz << d[l].wert <<  d[l].Zins << d[l].ZinsTesauriert;
        l++;
    }while( d[l].von.isValid());
}
void test_finance::test_ZinsesZins()
{
    QFETCH(QDate,  von);
    QFETCH(QDate,  bis);
    QFETCH(double, zinssatz);
    QFETCH(double, wert);
    QFETCH(double, ZinsTesauriert);
    QFETCH(double, Zins);
    QString msg("%3% Zinsen von %4 euro vom %1 bis %2, ");
    msg = msg.arg(von.toString("dd.MM.yyyy"), bis.toString("dd.MM.yyyy"), QString::number(zinssatz), QString::number(wert));
    qDebug().noquote() <<  msg;
    QCOMPARE(ZinsesZins(zinssatz, wert, von, bis, false), ZinsTesauriert);
    QCOMPARE(ZinsesZins(zinssatz, wert, von, bis, true), Zins);

}
