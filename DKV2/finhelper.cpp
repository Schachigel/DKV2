
#include <math.h>
#include <QDebug>

#include "finhelper.h"
#include "financaltimespan.h"

double round(const double d, const int stellen)
{
    return qRound((d * pow(10,stellen)))/pow(10,stellen);
}
double round2(const double d)
{
    return round( d, 2);
}
double round6(const double d)
{
    return round( d, 6);
}

int TageBisMonatsende_exclusiv(const QDate& d)
{
    if( d.day() == 31) return 0;
    return 30 -d.day();
}

int TageSeitMonatsAnfang_inclusive(const QDate& d)
{
    if( d.day() == 31) return 30;
    return d.day();
}

int TageZwischen(const QDate& von, const QDate& bis)
{
    Q_ASSERT( von.year() == bis.year());
    Q_ASSERT(bis >= von);
    if( bis.day() == 31)
        return TageZwischen(von, QDate(bis.year(), bis.month(), 30));
    if( von.day() == 31)
        return TageZwischen(QDate(von.year(), von.month(), 30), bis);
    if( von.month() == bis.month())
        return bis.day() -von.day();

    int tageErsterMonat = 30-von.day();
    int tageVolleMonate = 30* (bis.month() -von.month() -1);
    int tageLetzterMonat = bis.day();
    qDebug() << "TageZwischen (" << von << ") und (" << bis << "): "
             << tageErsterMonat << " + " << tageVolleMonate << " + " << tageLetzterMonat
             << " =" << tageErsterMonat+tageVolleMonate+tageLetzterMonat;
    return tageErsterMonat + tageVolleMonate + tageLetzterMonat;
}

int TageBisJahresende_a(const QDate& d)
{
    if( d.day()==d.daysInMonth())
    {
        if( d.month()==2)
            // Feb hat 30 Tage (da Vertrag über Feb hinaus geht)
            return 30* (12-d.month()) +30-d.day();
        else
            // der 31. wird behandelt wie der 30.
            // alle restlichen Monate des Jahres
            return 30* (12-d.month());
    }
    int month = 12 - d.month();
    int days { 30 -d.day()};
    return 30*month +days;
}

int TageBisJahresende(const QDate& d)
{
    if( QDate::isLeapYear(d.year()))
    {
        QDate mapToKeyYear(2016, d.month(), d.day());
        return DateDiffLookup_leapYear[mapToKeyYear].tillEoY;
    }
    else
    {
        QDate mapToKeyYear(2017, d.month(), d.day());
        return DateDiffLookup[mapToKeyYear].tillEoY;
    }

}

int TageSeitJahresAnfang_a(const QDate& d)
{
    return 30* (d.month()-1) + ((d.day() == 31) ? 30 : d.day());
}

int TageSeitJahresAnfang(const QDate& d)
{
    if( QDate::isLeapYear(d.year()))
    {
        QDate mapToKeyYear(2016, d.month(), d.day());
        return DateDiffLookup_leapYear[mapToKeyYear].sinceBoY;
    }
    else
    {
        QDate mapToKeyYear(2017, d.month(), d.day());
        return DateDiffLookup[mapToKeyYear].sinceBoY;
    }

}

double ZinsesZins(const double zins, const double wert,const QDate von, const QDate bis, const bool tesa)
{
    qDebug().noquote() << "\n\nZinsberechnung von " << von << " bis " << bis << QString((tesa)? " tesaurierend" : ("bei Zinsauszahlungen"));
    if( !(von.isValid() && bis.isValid()) || ( von > bis))
    {
        qCritical() << "Zinseszins kann nicht berechnet werden - ungültige Parameter";
        return -1.;
    }

    if( von.year() == bis.year())
    {
        return round2(double(TageZwischen(von, bis))/360. *zins/100. *wert);
    }
    int TageImErstenJahr = TageBisJahresende(von); // first day no intrest
    double ZinsImErstenJahr = round2(double(TageImErstenJahr)/360. *zins/100. *wert);
    double gesamtZins (ZinsImErstenJahr);

    double zwischenWert = (tesa) ? (wert+ZinsImErstenJahr) : (wert);
    double ZinsVolleJahre = 0.;
    int jahre(0);
    for( jahre=0; jahre < bis.year()-von.year()-1; jahre++)
    {
        double JahresZins = zwischenWert *zins/100.;
        gesamtZins += JahresZins; ZinsVolleJahre += JahresZins;
        zwischenWert = (tesa) ? (zwischenWert+JahresZins) : zwischenWert;
    }
    gesamtZins = round2(gesamtZins);
    int TageImLetztenJahr = TageSeitJahresAnfang(bis);
    double ZinsRestjahr = round2(double(TageImLetztenJahr)/360. *zins/100. *zwischenWert);
    gesamtZins += ZinsRestjahr;
    qDebug().noquote()
        << "\nErstes Jahr : " << ZinsImErstenJahr << "(" << TageImErstenJahr << " Tage)"
        << "\nVolle Jahre : " << ZinsVolleJahre << "(" << jahre << " Jahre)"
        << "\nLetztes Jahr: " << ZinsRestjahr << "(" << TageImLetztenJahr << " Tage)"
        << "\nGesamtzins  : " << gesamtZins << endl;
    return round2(gesamtZins);
}
