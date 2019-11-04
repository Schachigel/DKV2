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
        //        // remember: last day is IN, first day is OUT
        //        // expected test results validated with docs\\Zinsberechnung.xlsx

        // period within the same year
        {QDate(2019,  1, 03), QDate(2019,  11,  04), 10., 100.,  8.36 ,  8.36 },
        {QDate(2019,  1, 31), QDate(2019,  11,  30), 10., 100.,  8.33 ,  8.33 },
        {QDate(2019,  6,  1), QDate(2019,  12,  31), 10., 100.,  5.81 ,  5.81 },
        {QDate(2016,  1, 15), QDate(2016,   2,  29), 10., 100.,  1.22 ,  1.22 },
        {QDate(2016,  1, 15), QDate(2016,   3,   1), 10., 100.,  1.28 ,  1.28 },

        {QDate(2016,  2, 28), QDate(2016,   3,  30), 10., 100.,  0.89 ,  0.89 },
        {QDate(2016,  2, 29), QDate(2016,   3,  30), 10., 100.,  0.86 ,  0.86 },
        {QDate(2016,  3,  1), QDate(2016,   3,  30), 10., 100.,  0.81 ,  0.81 },
        {QDate(2015,  2, 28), QDate(2015,   3,  30), 10., 100.,  0.89 ,  0.89 },
        {QDate(2015,  3,  1), QDate(2015,   3,  30), 10., 100.,  0.81 ,  0.81 },

        {QDate(2015, 12,  1), QDate(2017,  01,  31), 10., 100., 11.81 , 11.64 },
        {QDate(2016, 12,  1), QDate(2018,  01,  01), 10., 100., 10.92 , 10.84 },
        // geht der Vertrag über Feb hinaus, zählt Feb er 30 Tage
        {QDate(2017, 01, 31), QDate(2018,  03,  01), 10., 100., 11.02 , 10.86 },

        {QDate(2016, 12, 31), QDate(2017,  12,  31), 10., 100., 10.00 , 10.00 },
        {QDate(2016, 12, 31), QDate(2018,  01,  30), 10., 100., 10.92 , 10.83 },
        // Abweichung von sparbuch-rechner: Ende Feb sind 28 nicht 30 Tage
        {QDate(2017, 01, 31), QDate(2018,  02,  28), 10., 100., 10.93 , 10.78 },
        // Ein Jahr ist nicht gleich ein Jahr ...zinstechnisch
        {QDate(2016,  1, 31), QDate(2017,   2,   1), 10., 100., 10.11 , 10.03 },
        {QDate(2016,  2, 29), QDate(2017,   3,   1), 10., 100., 10.20 , 10.05 },
        {QDate(2016,  3, 31), QDate(2017,   4,   1), 10., 100., 10.22 , 10.03 },
        {QDate(2016,  4, 30), QDate(2017,   5,   1), 10., 100., 10.26 , 10.03 },
        {QDate(2016,  5, 31), QDate(2017,   6,   1), 10., 100., 10.27 , 10.02 },
        {QDate(2016,  6, 30), QDate(2017,   7,   1), 10., 100., 10.28 , 10.03 },
        {QDate(2016,  7, 31), QDate(2017,   8,   1), 10., 100., 10.28 , 10.03 },
        {QDate(2016,  8, 31), QDate(2017,   9,   1), 10., 100., 10.25 , 10.02 },
        {QDate(2016,  9, 30), QDate(2017,  10,   1), 10., 100., 10.22 , 10.03 },
        {QDate(2016, 10, 31), QDate(2017,  11,   1), 10., 100., 10.17 , 10.03 },
        {QDate(2016, 11, 30), QDate(2017,  12,   1), 10., 100., 10.10 , 10.02 },
        // halbe Jahre
        {QDate(2016, 12, 31), QDate(2017,   6,  30), 10., 100.,  5.0 ,   5.0  },
        {QDate(2016, 12, 31), QDate(2018,   6,  30), 10., 100., 15.50,  15.0  },
        {QDate(2015, 12, 31), QDate(2018,   1,   1), 10., 100., 21.03 , 20.03 },
        {QDate(2016, 12, 31), QDate(2019,   6,  30), 10., 100., 27.05 , 25.0  },
        {QDate(2015, 12, 31), QDate(2019,   1,   1), 10., 100., 33.14 , 30.03 },
        {QDate(2016, 12, 31), QDate(2020,   6,  30), 10., 100., 39.76 , 35.0  },
        {QDate(2016,  6, 30), QDate(2020,  12,  31), 10., 100., 53.74 , 45.0  },
        {QDate(2015, 12, 31), QDate(2020,   1,   1), 10., 100., 46.45 , 40.03 },
        {QDate(2016, 12, 31), QDate(2021,   6,  30), 10., 100., 53.73 , 45.0  },
        {QDate(2015, 12, 31), QDate(2021,   1,   1), 10., 100., 61.09 , 50.03 },
        // längere Zeiträume
        {QDate(2015, 12, 31), QDate(2024,   2,   28), 10., 100.,117.81, 81.61 },
        {QDate(2015, 12, 31), QDate(2024,   2,   29), 10., 100.,117.87 ,81.64 },
        {QDate(2015,  1,  1), QDate(2025,   1,    1), 10., 100.,159.37 ,100.0 },

        {QDate(), QDate(), 0., 0., 0., 0.}
    };

    int l= 0;
    do
    {
        QTest::newRow(QString().toLocal8Bit().data())
            << d[l].von << d[l].bis << d[l].zinssatz << d[l].wert << d[l].ZinsTesauriert <<  d[l].Zins ;
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
    QCOMPARE(ZinsesZins(zinssatz, wert, von, bis, true ), ZinsTesauriert);
    QCOMPARE(ZinsesZins(zinssatz, wert, von, bis, false), Zins);

}
