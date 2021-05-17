#include <iso646.h>

#include <qtest.h>
#include <QString>
#include <QDebug>

#include "../DKV2/helperfin.h"
#include "../DKV2/ibanvalidator.h"
#include "test_finance.h"

// https://stackoverflow.com/questions/30168056/what-is-the-exact-excel-days360-algorithm
int dateDiff360(int startDay, int startMonth, int startYear, int endDay, int endMonth, int endYear, bool methodUS)
{
    if (startDay == 31) {
        --startDay;
    } else if (methodUS and (startMonth == 2 and (startDay == 29 or (startDay == 28 and not QDate::isLeapYear(startYear))))) {
        startDay = 30;
    }
    if (endDay == 31) {
        if (methodUS and startDay not_eq 30) {
            endDay = 1;
            if (endMonth == 12) {
                ++endYear;
                endMonth = 1;
            } else {
                ++endMonth;
            }
        } else {
            endDay = 30;
        }
    }
    return endDay + endMonth * 30 + endYear * 360 - startDay - startMonth * 30 - startYear * 360;
}

int dateDiff360(const QDate &StartDate, const QDate &EndDate, bool methodUS)
{
   int startDay = StartDate.day();
   int startMonth = StartDate.month();
   int startYear = StartDate.year();
   int endDay = EndDate.day();
   int endMonth = EndDate.month();
   int endYear = EndDate.year();
   int ret = dateDiff360(startDay, startMonth, startYear, endDay, endMonth, endYear, methodUS);
   return ret;
}

int lookupAnzTageZeitraum(const QDate &StartDate, const QDate &EndDate)
{
   int ret = dateDiff360(StartDate, EndDate, false);
   return ret;
}

int getAnzTageUndJahreZeitraum(const QDate &dateFrom, const QDate &dateTo, int &tageBis, int &ganzeJahre, int &tageVon){
   int ret = 0;
   tageBis = 0;
   ganzeJahre = 0;
   tageVon = 0;
   if( dateFrom.isValid() and dateTo.isValid() and (dateFrom <= dateTo))
   {
       if(dateFrom.year() == dateTo.year())
       {
           tageBis = lookupAnzTageZeitraum(dateFrom, dateTo);
           return 0;
       }
      if(dateFrom.year() < dateTo.year())
      {
          tageBis = lookupAnzTageZeitraum(dateFrom, QDate(dateFrom.year(), 12, 31));
      }
      QDate startDate = dateFrom;
      QDate endDate = dateTo;
      startDate = QDate(startDate.year()+1, 1, 1);
      while(startDate <= endDate)
      {
         if(startDate.year() < endDate.year())
         {
            ganzeJahre++;
            startDate = QDate(startDate.year()+1, 1, 1);
         }else{
            tageVon = lookupAnzTageZeitraum(startDate, dateTo) + 1;
            break;
         }
      }
   }
   return ret;
}

double computeDkZinsen(double Betrag, double Zinssatz, int anzTage)
{
    double Zinsen = ((Betrag * Zinssatz) / 100.0);
    Zinsen = Zinsen * anzTage / 360;
    // Zinsen = round2(double(anzTage)/360. *Zinssatz/100. *Betrag);
    return Zinsen;
}

double computeDkZinsenZeitraum(double Betrag, double Zinssatz, const QDate &dateFrom, const QDate &dateTo)
{
    double Zinsen = 0;
    if( dateFrom.isValid() and dateTo.isValid() and (dateFrom <= dateTo))
    {
        int tageBis = 0;
        int ganzeJahre = 0;
        int tageVon = 0;
        getAnzTageUndJahreZeitraum(dateFrom, dateTo, tageBis, ganzeJahre, tageVon);
        Zinsen += computeDkZinsen(Betrag + Zinsen, Zinssatz, tageBis);
        for(int i=0;i<ganzeJahre;i++) {
            Zinsen += (Betrag + Zinsen) * Zinssatz / 100.0;
        }
        Zinsen += computeDkZinsen(Betrag + Zinsen, Zinssatz, tageVon);
    }
    return r2(Zinsen);
}

int TageImZeitraum(QDate von, QDate bis)
{
    if( von.year() == bis.year())
    {
        return TageZwischen(von, bis);
    }
   int ret = 0;
   int TageImErstenJahr = TageBisJahresende_lookup(von); // first day no intrest
   int jahre(0);
   int TageDazwischen = 0;
   for( jahre=0; jahre < bis.year()-von.year()-1; jahre++)
   {
       TageDazwischen += 360;
   }
   int TageImLetztenJahr = TageSeitJahresAnfang_lookup(bis);
   ret = TageImErstenJahr + TageDazwischen + TageImLetztenJahr;
   return ret;
}

int TageUndJahreImZeitraum(const QDate &von, const QDate &bis, int &TageImErstenJahr, int &JahreZwischen, int &TageImLetztenJahr){
    int ret = 0;
    TageImErstenJahr = 0;
    JahreZwischen = 0;
    TageImLetztenJahr = 0;
    if( von.year() == bis.year())
    {
        TageImErstenJahr = TageZwischen(von, bis);
        return 0;
    }
   TageImErstenJahr = TageBisJahresende_lookup(von); // first day no intrest
   int jahre(0);
   for( jahre=0; jahre < bis.year()-von.year()-1; jahre++)
   {
      JahreZwischen++;
   }
   TageImLetztenJahr = TageSeitJahresAnfang_lookup(bis);
   return ret;

}

void test_finance::test_TageBisJahresende_data()
{
    QTest::addColumn<QDate>("date");
    QTest::addColumn<int>("tageBisJe_excel");
    bool singleTestForDebugging = false;
    if( singleTestForDebugging)
    {
        QDate testDatum(2017, 1, 1);
        QTest::newRow( (QString("Bis JE vom ") + testDatum.toString()).toLocal8Bit().data())
                << testDatum << TageBisJahresende_lookup(testDatum);
    }
    else
    {
        QDate start(2015, 12, 30);
        for (int i=0; i < 365+366; i++)
        {
            QDate testDatum(start.addDays(i));
            QTest::newRow( (QString("Bis JE vom ") + testDatum.toString()).toLocal8Bit().data())
                    << testDatum << TageBisJahresende_lookup(testDatum);
        }
    }
}
void test_finance::test_TageBisJahresende()
{
    QFETCH(QDate, date);
    QFETCH(int, tageBisJe_excel);
    // compare our calculation with the result from excel
    QCOMPARE( TageBisJahresende(date), tageBisJe_excel);
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
                << testDatum << TageSeitJahresAnfang_lookup(testDatum);
    }
    else
    {
        QDate start(2015, 12, 30);
        for (int i=0; i < 365+366; i++)
        {
            QDate testDatum(start.addDays(i));
            QTest::newRow( (QString("Seit JA bis ") + testDatum.toString()).toLocal8Bit().data())
                    << testDatum << TageSeitJahresAnfang_lookup(testDatum);
        }
    }

}
void test_finance::test_TageSeitJahresanfang()
{
    QFETCH(QDate, date);
    QFETCH(int, tageSeitJa_excel);
    // compare our calculation with the result from excel
    QCOMPARE( TageSeitJahresAnfang(date), tageSeitJa_excel);

}

void test_finance::test_ZinsesZins_data()
{
    QTest::addColumn<QDate>( "von");
    QTest::addColumn<QDate>( "bis");
    QTest::addColumn<double>("zinssatz");
    QTest::addColumn<double>("wert");
    QTest::addColumn<double>("ZinsThesauriert");
    QTest::addColumn<double>("Zins");

    struct testdata
    {
        QDate von; QDate bis;
        double zinssatz; double wert;
        double ZinsThesauriert; double Zins;
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
        {QDate(2016, 12,  1), QDate(2018,  01,  01), 10., 100., 10.92 , 10.83 },
        // geht der Vertrag über Feb hinaus, zählt Feb er 30 Tage
        {QDate(2017, 01, 31), QDate(2018,  03,  01), 10., 100., 11.02 , 10.86 },

        {QDate(2016, 12, 31), QDate(2017,  12,  31), 10., 100., 10.00 , 10.00 },
        {QDate(2016, 12, 31), QDate(2018,  01,  30), 10., 100., 10.92 , 10.83 },
        // Abweichung von sparbuch-rechner: Ende Feb sind 28 nicht 30 Tage
        {QDate(2017, 01, 31), QDate(2018,  02,  28), 10., 100., 10.93 , 10.78 },
        // Ein Jahr ist nicht gleich ein Jahr ...zinstechnisch
        {QDate(2016,  1, 31), QDate(2017,   2,   1), 10., 100., 10.11 , 10.03 },
        {QDate(2016,  2, 29), QDate(2017,   3,   1), 10., 100., 10.20 , 10.06 },
        {QDate(2016,  3, 31), QDate(2017,   4,   1), 10., 100., 10.22 , 10.03 },
        {QDate(2016,  4, 30), QDate(2017,   5,   1), 10., 100., 10.25 , 10.03 },
        {QDate(2016,  5, 31), QDate(2017,   6,   1), 10., 100., 10.27 , 10.03 },
        {QDate(2016,  6, 30), QDate(2017,   7,   1), 10., 100., 10.28 , 10.03 },
        {QDate(2016,  7, 31), QDate(2017,   8,   1), 10., 100., 10.27 , 10.03 },
        {QDate(2016,  8, 31), QDate(2017,   9,   1), 10., 100., 10.25 , 10.03 },
        {QDate(2016,  9, 30), QDate(2017,  10,   1), 10., 100., 10.22 , 10.03 },
        {QDate(2016, 10, 31), QDate(2017,  11,   1), 10., 100., 10.17 , 10.03 },
        {QDate(2016, 11, 30), QDate(2017,  12,   1), 10., 100., 10.10 , 10.03 },
        // halbe Jahre
        {QDate(2016, 12, 31), QDate(2017,   6,  30), 10., 100.,  5.0 ,   5.0  },
        {QDate(2016, 12, 31), QDate(2018,   6,  30), 10., 100., 15.50,  15.0  },
        {QDate(2015, 12, 31), QDate(2018,   1,   1), 10., 100., 21.03 , 20.03 },
        {QDate(2016, 12, 31), QDate(2019,   6,  30), 10., 100., 27.05 , 25.0  },
        {QDate(2015, 12, 31), QDate(2019,   1,   1), 10., 100., 33.14 , 30.03 },
        {QDate(2016, 12, 31), QDate(2020,   6,  30), 10., 100., 39.76 , 35.0  },
        {QDate(2016,  6, 30), QDate(2020,  12,  31), 10., 100., 53.73 , 45.0  },
        {QDate(2015, 12, 31), QDate(2020,   1,   1), 10., 100., 46.45 , 40.03 },
        {QDate(2016, 12, 31), QDate(2021,   6,  30), 10., 100., 53.73 , 45.0  },
        {QDate(2015, 12, 31), QDate(2021,   1,   1), 10., 100., 61.10 , 50.03 },
        // längere Zeiträume
        {QDate(2015, 12, 31), QDate(2024,   2,   28), 10., 100.,117.81, 81.61 },
        {QDate(2015, 12, 31), QDate(2024,   2,   29), 10., 100.,117.87 ,81.64 },
        {QDate(2015,  1,  1), QDate(2025,   1,    1), 10., 100.,159.38 ,100.0 },

        {QDate(), QDate(), 0., 0., 0., 0.}
    };

    int l= 0;
    do
    {
        QTest::newRow(QString().toLocal8Bit().data())
            << d[l].von << d[l].bis << d[l].zinssatz << d[l].wert << d[l].ZinsThesauriert <<  d[l].Zins ;
        l++;
    }while( d[l].von.isValid());
}

void test_finance::test_ZinsesZins()
{
    QFETCH(QDate,  von);
    QFETCH(QDate,  bis);
    QFETCH(double, zinssatz);
    QFETCH(double, wert);
    QFETCH(double, ZinsThesauriert);
    QFETCH(double, Zins);
    QString msg("%3% Zinsen von %4 euro vom %1 bis %2, ");
    msg = msg.arg(von.toString("dd.MM.yyyy"), bis.toString("dd.MM.yyyy"), QString::number(zinssatz), QString::number(wert));
    qDebug().noquote() <<  msg;
    // compare our calculation with the result from excel
    QCOMPARE(ZinsesZins(zinssatz, wert, von, bis, true ), ZinsThesauriert);
    QCOMPARE(ZinsesZins(zinssatz, wert, von, bis, false), Zins);
    QCOMPARE(lookupAnzTageZeitraum(von, bis), TageImZeitraum(von, bis));
    int TageImErstenJahr=0;
    int JahreZwischen=0;
    int TageImLetztenJahr=0;
    TageUndJahreImZeitraum(von, bis, TageImErstenJahr, JahreZwischen, TageImLetztenJahr);
    int tageBis = 0;
    int ganzeJahre = 0;
    int tageVon = 0;
    getAnzTageUndJahreZeitraum(von, bis, tageBis, ganzeJahre, tageVon);
    QCOMPARE(TageImErstenJahr, tageBis);
    QCOMPARE(JahreZwischen, ganzeJahre);
    QCOMPARE(TageImLetztenJahr, tageVon);
    QCOMPARE(r2(computeDkZinsen(wert, zinssatz, tageBis)), r2(double(TageImErstenJahr)/360. *zinssatz/100. *wert));
    QCOMPARE(computeDkZinsenZeitraum(wert, zinssatz, von, bis ), ZinsThesauriert);
    qInfo()<< "\n------------------------------------------------------------------------------\n";
}

void test_finance::test_IsValidIban_data()
{
    QTest::addColumn<QString>("IBAN");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("Iban -2") << "" << false;
    QTest::newRow("Iban -1") << "DE04500105175411049860" << false;
    QTest::newRow("Iban 00") << "IT68D0300203280000400162854" << true;
    QTest::newRow("Iban 01") << "DE68 2105 0170 0012 3456 78" << true;
    QTest::newRow("Iban 02") << "DE27100777770209299700 " << true;
    QTest::newRow("Iban 03") << "DE11 5205 1373 5120 7101 31 " << true;
    QTest::newRow("Iban 04") << "AL90208110080000001039531801" << true;
    QTest::newRow("Iban 05") << "BE68844010370034" << true;
    QTest::newRow("Iban 06") << "DK5750510001322617" << true;
    QTest::newRow("Iban 07") << "DE12500105170648489890" << true;
    QTest::newRow("Iban 08") << "EE342200221034126658" << true;
    QTest::newRow("Iban 09") << "FI9814283500171141" << true;
    QTest::newRow("Iban 10") << "DE28 5001 0517 5407 5101 98" << true;
    QTest::newRow("Iban 11") << "MC1112739000700011111000H79" << true;
    QTest::newRow("Iban 12") << "NL18ABNA0484869868    " << true;
}
void test_finance::test_IsValidIban()
{
    QFETCH(QString, IBAN);
    QFETCH(bool, isValid);
    IbanValidator iv;
    int pos = 0;
    QCOMPARE(iv.validate(IBAN, pos) == IbanValidator::State::Acceptable, isValid);
}
