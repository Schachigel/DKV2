#include "../DKV2/pch.h"



#include "../DKV2/helper.h"
#include "../DKV2/helperfin.h"
#include "../DKV2/ibanvalidator.h"
#include "financaltimespan.h"
#include "test_finance.h"

bool lastDayOfMonth(const QDate d)
{
    switch (d.month())
    {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
        return d.day() == 31;
        break;
    case 4:
    case 6:
    case 9:
    case 11:
        return d.day() == 30;
    }
    // month == 2
    if( QDate::isLeapYear (d.year()))
        return d.day() == 29;
    else
        return d.day() == 28;
}

// https://stackoverflow.com/questions/30168056/what-is-the-exact-excel-days360-algorithm
int t_helper_dateDiff360( QDate StartDate, QDate EndDate/*, bool methodUS*/)
{
   return (EndDate.day() == 31 ? 30 : EndDate.day())
            +EndDate.month() * 30
            +EndDate.year() * 360
            -(StartDate.day() == 31 ? 30 : StartDate.day())
            -StartDate.month() * 30 - StartDate.year() * 360;
}

int t_helper_lookupAnzTageZeitraum(const QDate &StartDate, const QDate &EndDate)
{
   int ret = t_helper_dateDiff360(StartDate, EndDate/*, false*/);
   return ret;
}

int t_helper_getAnzTageUndJahreZeitraum(const QDate &dateFrom, const QDate &dateTo, int &tageBis, int &ganzeJahre, int &tageVon){
   int ret = 0;
   tageBis = 0;
   ganzeJahre = 0;
   tageVon = 0;
   if( dateFrom.isValid() and dateTo.isValid() and (dateFrom <= dateTo))
   {
       if(dateFrom.year() == dateTo.year())
       {
           tageBis = t_helper_lookupAnzTageZeitraum(dateFrom, dateTo);
           return 0;
       }
      if(dateFrom.year() < dateTo.year())
      {
          tageBis = t_helper_lookupAnzTageZeitraum(dateFrom, QDate(dateFrom.year(), 12, 31));
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
            tageVon = t_helper_lookupAnzTageZeitraum(startDate, dateTo) + 1;
            break;
         }
      }
   }
   return ret;
}

double t_helper_computeDkZinsen(double Betrag, double Zinssatz, int anzTage)
{
    double Zinsen = ((Betrag * Zinssatz) / 100.0);
    Zinsen = Zinsen * anzTage / 360;
    // Zinsen = round2(double(anzTage)/360. *Zinssatz/100. *Betrag);
    return Zinsen;
}

double t_helper_computeDkZinsenZeitraum(double Betrag, double Zinssatz, const QDate &dateFrom, const QDate &dateTo)
{
    double Zinsen = 0;
    if( dateFrom.isValid() and dateTo.isValid() and (dateFrom <= dateTo))
    {
        int tageBis = 0;
        int ganzeJahre = 0;
        int tageVon = 0;
        t_helper_getAnzTageUndJahreZeitraum(dateFrom, dateTo, tageBis, ganzeJahre, tageVon);
        Zinsen += t_helper_computeDkZinsen(Betrag + Zinsen, Zinssatz, tageBis);
        for(int i=0;i<ganzeJahre;i++) {
            Zinsen += (Betrag + Zinsen) * Zinssatz / 100.0;
        }
        Zinsen += t_helper_computeDkZinsen(Betrag + Zinsen, Zinssatz, tageVon);
    }
    return r2(Zinsen);
}

int t_helper_TageImZeitraum(QDate von, QDate bis)
{
    if (von.year() == bis.year()) {
        return TageZwischen_30_360(von, bis);
    }
    int ret = {0};
    int TageImErstenJahr {t_helper_TageBisJahresende_lookup(von)}; // first day no intrest
    int jahre{0};
    int TageDazwischen{0};
    for (jahre = 0; jahre < bis.year() - von.year() - 1; jahre++) {
        TageDazwischen += 360;
    }
    int TageImLetztenJahr = t_helper_TageSeitJahresAnfang_lookup(bis);
    ret = TageImErstenJahr + TageDazwischen + TageImLetztenJahr;
    return ret;
}

int t_helper_TageUndJahreImZeitraum(const QDate &von, const QDate &bis, int &TageImErstenJahr, int &JahreZwischen, int &TageImLetztenJahr){
    int ret = 0;
    TageImErstenJahr = 0;
    JahreZwischen = 0;
    TageImLetztenJahr = 0;
    if( von.year() == bis.year())
    {
        TageImErstenJahr = TageZwischen_30_360(von, bis);
        return 0;
    }
   TageImErstenJahr = t_helper_TageBisJahresende_lookup(von); // first day no intrest
   int jahre(0);
   for( jahre=0; jahre < bis.year()-von.year()-1; jahre++)
   {
      JahreZwischen++;
   }
   TageImLetztenJahr = t_helper_TageSeitJahresAnfang_lookup(bis);
   return ret;

}

void test_finance::test_TageBisJahresende_data()
{
   // create test data with test helper functions and excel lookup table
    QTest::addColumn<QDate>("date");
    QTest::addColumn<int>("tageBisJe_excel");
    bool singleTestForDebugging = false;
    if( singleTestForDebugging) {
        QDate testDatum(2017, 1, 1);
        QTest::newRow( (QString("Bis JE vom ") + testDatum.toString()).toLocal8Bit().data())
                << testDatum << t_helper_TageBisJahresende_lookup(testDatum);
    } else {
        QDate start(2015, 12, 30);
        for (int i=0; i < 365+366; i++) {
            QDate testDatum(start.addDays(i));
            QTest::newRow( (QString("Bis JE vom ") + testDatum.toString()).toLocal8Bit().data())
                << testDatum << t_helper_TageBisJahresende_lookup(testDatum);
        }
    }
}
void test_finance::test_TageBisJahresende()
{
    QFETCH(QDate, date);
    QFETCH(int, tageBisJe_excel);
    // CUT: TageBisJahresende_30_360, vergleich mit Excel
    QCOMPARE( TageBisJahresende_30_360(date), tageBisJe_excel);
}

void test_finance::test_TageBisJahresendeSample() {
    // Vertragsbeginn: erster Tag zinsfrei
    // Jeder Monat zählt für 30 Tage, 31ter zählt nicht
    QCOMPARE( TageBisJahresende_30_360(QDate(2015, 1,  1)), 29/*rest vom Jan.*/    +11*30/*Feb.-Dez.*/);
    QCOMPARE( TageBisJahresende_30_360(QDate(2015, 1, 15)), 15/*rest vom Jan.*/    +11*30/*Feb.-Dez.*/);
    QCOMPARE( TageBisJahresende_30_360(QDate(2015, 1, 30)),  0/*31. ist zinsfrei*/ +11*30/*Feb.-Dez.*/);
    QCOMPARE( TageBisJahresende_30_360(QDate(2015, 1, 31)),  0/*31. ist zinsfrei*/ +11*30/*Feb.-Dez.*/);
    QCOMPARE( TageBisJahresende_30_360(QDate(2015, 2,  1)), 29/*rest vom Feb.*/    +10*30/*Mär.-Dez.*/);
    QCOMPARE( TageBisJahresende_30_360(QDate(2015, 4, 29)),  1/*rest vom Apr.*/    + 8*30/*Mai.-Dez.*/);
    QCOMPARE( TageBisJahresende_30_360(QDate(2015, 4, 30)),  0/*rest vom Apr.*/    + 8*30/*Mai.-Dez.*/);

    // 2015 ist KEIN Schaltjahr
    QCOMPARE( TageBisJahresende_30_360(QDate(2015, 2, 28)),  2/*rest vom Feb.*/    +10*30/*Feb.-Dez.*/);
    // 2016 IST  ein Schaltjahr
    QCOMPARE( TageBisJahresende_30_360(QDate(2016, 2, 28)),  2/*rest vom Feb.*/    +10*30/*Feb.-Dez.*/);
    QCOMPARE( TageBisJahresende_30_360(QDate(2016, 2, 29)),  1/*rest vom Feb.*/    +10*30/*Feb.-Dez.*/);
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
                << testDatum << t_helper_TageSeitJahresAnfang_lookup(testDatum);
    }
    else
    {
        QDate start(2015, 12, 30);
        for (int i=0; i < 365+366; i++)
        {
            QDate testDatum(start.addDays(i));
            QTest::newRow( (QString("Seit JA bis ") + testDatum.toString()).toLocal8Bit().data())
                    << testDatum << t_helper_TageSeitJahresAnfang_lookup(testDatum);
        }
    }

}
void test_finance::test_TageSeitJahresanfang()
{
    QFETCH(QDate, date);
    QFETCH(int, tageSeitJa_excel);
    // CUT: TageSeitJahresAnfang_30_360: Vergleich der Berechnung mit Excel
    QCOMPARE( TageSeitJahresAnfang_30_360(date), tageSeitJa_excel);
}

void test_finance::test_TageSeitJahresanfangSample()
{
    // schon der erste Tag zählt, da innerhalb einer Zinsperiode
    QCOMPARE( TageSeitJahresAnfang_30_360(QDate(2015, 1,  1)),  1);
    // auch Monate m 31 Tagen zählen nur als 30
    QCOMPARE( TageSeitJahresAnfang_30_360(QDate(2015, 1, 31)), 30);
    QCOMPARE( TageSeitJahresAnfang_30_360(QDate(2015, 2,  1)), 31);
    // Schaltjahr
    QCOMPARE( TageSeitJahresAnfang_30_360(QDate(2016, 2, 28)), 30 +28);
    QCOMPARE( TageSeitJahresAnfang_30_360(QDate(2016, 2, 29)), 30 +29);
    QCOMPARE( TageSeitJahresAnfang_30_360(QDate(2016, 3,  1)), 30 +30 +1);
    // NichtSchaltjahre
    QCOMPARE( TageSeitJahresAnfang_30_360(QDate(2017, 2, 27)), 30 +27);
    QCOMPARE( TageSeitJahresAnfang_30_360(QDate(2017, 2, 28)), 30 +28);
    QCOMPARE( TageSeitJahresAnfang_30_360(QDate(2017, 3,  1)), 30 +30 +1);
    // volles Jahr
    QCOMPARE( TageSeitJahresAnfang_30_360(QDate(2017, 12, 29)), 11*30 +29);
    QCOMPARE( TageSeitJahresAnfang_30_360(QDate(2017, 12, 30)), 12*30);
    QCOMPARE( TageSeitJahresAnfang_30_360(QDate(2017, 12, 31)), 12*30);
}

void test_finance::test_ZinsesZins_data()
{
    QTest::addColumn<QDate>( "von");
    QTest::addColumn<QDate>( "bis");
    QTest::addColumn<double>("dZinssatz");
    QTest::addColumn<double>("dWert");
    QTest::addColumn<double>("dZinsThesauriert");
    QTest::addColumn<double>("dZins");

    struct testdata
    {
        QDate von; QDate bis;
        double zinssatz; double wert;
        double ZinsThesauriert; double Zins;
    };
    testdata d[]={
        // // Der letzte Tag wird verzinst, der erste Tag NICHT !!
        // // siehe excel Datei docs\\Zinsberechnung.xlsx

        // Zinsperioden innerhalb eines Jahres
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
    QFETCH(double, dZinssatz);
    QFETCH(double, dWert);
    QFETCH(double, dZinsThesauriert);
    QFETCH(double, dZins);
    QString msg("%3% Zinsen von %4 euro vom %1 bis %2, ");
    msg = msg.arg(von.toString("dd.MM.yyyy"), bis.toString("dd.MM.yyyy"), QString::number(dZinssatz), QString::number(dWert));
    qInfo().noquote() <<  msg;
    // compare our calculation with the result from excel
    QCOMPARE(ZinsesZins_30_360(dZinssatz, dWert, von, bis, true ), dZinsThesauriert);
    QCOMPARE(ZinsesZins_30_360(dZinssatz, dWert, von, bis, false), dZins);
    QCOMPARE(t_helper_lookupAnzTageZeitraum(von, bis), t_helper_TageImZeitraum(von, bis));
    int TageImErstenJahr=0;
    int JahreZwischen=0;
    int TageImLetztenJahr=0;
    t_helper_TageUndJahreImZeitraum(von, bis, TageImErstenJahr, JahreZwischen, TageImLetztenJahr);
    int tageBis = 0;
    int ganzeJahre = 0;
    int tageVon = 0;
    t_helper_getAnzTageUndJahreZeitraum(von, bis, tageBis, ganzeJahre, tageVon);
    QCOMPARE(TageImErstenJahr, tageBis);
    QCOMPARE(JahreZwischen, ganzeJahre);
    QCOMPARE(TageImLetztenJahr, tageVon);
    QCOMPARE(r2(t_helper_computeDkZinsen(dWert, dZinssatz, tageBis)), r2(double(TageImErstenJahr)/360. *dZinssatz/100. *dWert));
    QCOMPARE(t_helper_computeDkZinsenZeitraum(dWert, dZinssatz, von, bis ), dZinsThesauriert);
    qInfo()<< "\n------------------------------------------------------------------------------\n";
}

void test_finance::test_act_act_data()
{
    QTest::addColumn<QDate>("von");
    QTest::addColumn<QDate>("bis");
    QTest::addColumn<double>("expected");
    QTest::addColumn<bool>("thesa");

    QTest::newRow ("first month thesa") << QDate(2010, 1, 1) << QDate(2010, 1,31) << r2(100000./100./365.*30) << true;
    QTest::newRow ("first month") << QDate(2010, 1, 1) << QDate(2010, 1,31) << r2(100000./100./365.*30) << false;
    QTest::newRow ("first month ly thesa") << QDate(2016, 1, 1) << QDate(2016, 1,31) << r2(100000./100./366.*30) << true;
    QTest::newRow ("first month ly") << QDate(2016, 1, 1) << QDate(2016, 1,31) << r2(100000./100./366.*30) << false;
    QTest::newRow ("non ly 2 ly thesa") << QDate(2015, 7, 1) << QDate(2016, 6,30) << r2(100000./100./365.*183 + 100501.37/100./366.*182) << true;
    QTest::newRow ("non ly 2 ly") << QDate(2015, 7, 1) << QDate(2016, 6,30) << r2(100000./100./365.*183 + 100000./100./366.*182) << false;
    QTest::newRow ("ly 2 non ly thesa") << QDate(2016, 7, 1) << QDate(2017, 6,30) << r2(100000./100./366.*183 + 100500./100./365.*181) << true;
    QTest::newRow ("ly 2 non ly") << QDate(2016, 7, 1) << QDate(2017, 6,30) << r2(100000./100./366.*183 + 100000./100./365.*181) << false;
}
void test_finance::test_act_act()
{
    QFETCH (QDate, von);
    QFETCH (QDate, bis);
    QFETCH (double, expected);
    QFETCH (bool, thesa);
    double inter =1.;
    double amount =100000.;
    double interest =ZinsesZins_act_act (inter, amount, von, bis, thesa);

    QCOMPARE (interest, expected);
}

void createData()
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
    QTest::newRow("Iban 13") << "DE23152931057149592044" << true;
}

void test_finance::test_IbanValidator_data()
{
    createData();
}
void test_finance::test_IbanValidator()
{
    QFETCH(QString, IBAN);
    QFETCH(bool, isValid);
    IbanValidator iv;
    int pos = 0;
//    dbgTimer t(qsl("IsValidIban"));
    QCOMPARE(iv.validate(IBAN, pos) == IbanValidator::State::Acceptable, isValid);
}

void test_finance::test_checkIban_data()
{
    createData();
}
void test_finance::test_checkIban()
{
    QFETCH(QString, IBAN);
    QFETCH(bool, isValid);
//    dbgTimer t(qsl("checkIban"));
    QCOMPARE(checkIban(IBAN), isValid);
}

void test_finance::test_euroFromCt()
{
    for( int i=0; i>-1000; i--) {
        double euro =euroFromCt(i);
        int    ct   =ctFromEuro(euro);
//        qInfo() << i << ", " << QString::number (euro, 'f', 2) << ", " << ct;
        QCOMPARE(ct, i);
    }
    for( int i=0; i<1000; i++) {
        double euro =euroFromCt(i);
        int    ct   =ctFromEuro(euro);
//        qInfo() << i << ", " << QString::number (euro, 'f', 2) << ", " << ct;
        QCOMPARE(ct, i);
    }
    for( int i=9999; i<10999; i++) {
        double euro =euroFromCt(i);
        int    ct   =ctFromEuro(euro);
//        qInfo() << i << ", " << QString::number (euro, 'f', 2) << ", " << ct;
        QCOMPARE(ct, i);
    }
    for( int i=999000; i<1000100; i++) {
        double euro =euroFromCt(i);
        int    ct   =ctFromEuro(euro);
//        qInfo() << i << ", " << QString::number (euro, 'f', 2) << ", " << ct;
        QCOMPARE(ct, i);
    }
    for( int i=9999000; i<10000100; i++) {
        double euro =euroFromCt(i);
        int    ct   =ctFromEuro(euro);
//        qInfo() << i << ", " << QString::number (euro, 'f', 2) << ", " << ct;
        QCOMPARE(ct, i);
    }
    for( int i=99999000; i<100000100; i++) {
        double euro =euroFromCt(i);
        int    ct   =ctFromEuro(euro);
//        qInfo() << i << ", " << QString::number (euro, 'f', 2) << ", " << ct;
        QCOMPARE(ct, i);
    }
}

void test_finance::test_convertionFuncs()
{
    QCOMPARE( i2s(1), qsl("1"));
    QCOMPARE( i2s(-1), qsl("-1"));
    QCOMPARE( i2s(-99), qsl("-99"));
    QCOMPARE( i2s(987654321), qsl("987654321"));

    QCOMPARE( s_d2euro(1), qsl("1,00 €"));
    QCOMPARE( s_d2euro(1.234), qsl("1,23 €"));
    QCOMPARE( s_d2euro(1.235), qsl("1,24 €"));
    QCOMPARE( s_d2euro(1111.235), qsl("1.111,24 €"));
    QCOMPARE( s_d2euro(1234567), qsl("1.234.567,00 €"));

    QCOMPARE( prozent2prozent_str (1.), qsl("1,00 %"));
    QCOMPARE( prozent2prozent_str (1.5), qsl("1,50 %"));
    QCOMPARE( prozent2prozent_str (0.454), qsl("0,45 %"));
    QCOMPARE( prozent2prozent_str (0.455), qsl("0,46 %"));
    QCOMPARE( prozent2prozent_str (15.), qsl("15,00 %"));

    QCOMPARE( dbInterest2_str(0),   qsl("0,00 %"));
    QCOMPARE( dbInterest2_str(1),   qsl("0,01 %"));
    QCOMPARE( dbInterest2_str(10),  qsl("0,10 %"));
    QCOMPARE( dbInterest2_str(190), qsl("1,90 %"));
    QCOMPARE( dbInterest2_str(999), qsl("9,99 %"));
    QCOMPARE( dbInterest2_str(10000),   qsl("100,00 %"));

}
