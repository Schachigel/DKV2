#include <iso646.h>

#include <QStringLiteral>
#include <QFile>
#include <math.h>
#include <QDebug>

#include "helper.h"

#include "helperfin.h"

QString d2euro(double x)
{
    static QLocale locale;
    return locale.toCurrencyString(x);
}

int TageZwischen(QDate von, QDate bis)
{   // finanzmathematischer Abstand zwischen zwei Daten im selben Jahr
    if( bis == von) return 0;
    Q_ASSERT( von.year() == bis.year());
    Q_ASSERT(bis > von);
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

int TageBisJahresende(QDate d)
{
    if( d.day()==d.daysInMonth()) // letzter Tag des Monats
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

int TageSeitJahresAnfang(QDate d)
{
    return 30* (d.month()-1) + ((d.day() == 31) ? 30 : d.day());
}

double ZinsesZins(const double zins, const double wert,const QDate von, const QDate bis, const bool thesa)
{
    qDebug().noquote() << "\n\nZinsberechnung von " << von << " bis " << bis << ((thesa) ? (" thesaurierend\n") : ("ausschüttend\n"));
    if( not (von.isValid() and bis.isValid()) or ( von > bis)) {
        qCritical() << "Zinseszins kann nicht berechnet werden - ungültige Parameter";
        return -1.;
    }
    if( wert <= 0.) return 0.;

    if( von.year() == bis.year()) {
        return r2(TageZwischen(von, bis)/360. *zins/100. *wert);
    }
    int TageImErstenJahr = TageBisJahresende(von); // first day no intrest
    double ZinsImErstenJahr = TageImErstenJahr/360. *zins/100. *wert;
    double gesamtZins (ZinsImErstenJahr);

    double zwischenWert = (thesa) ? (wert+ZinsImErstenJahr) : (wert);
    double ZinsVolleJahre = 0.;
    int jahre(0);
    for( jahre=0; jahre < bis.year()-von.year()-1; jahre++) {
        double JahresZins = zwischenWert *zins/100.;
        gesamtZins += JahresZins; ZinsVolleJahre += JahresZins;
        zwischenWert = (thesa) ? (zwischenWert+JahresZins) : zwischenWert;
    }
    int TageImLetztenJahr = TageSeitJahresAnfang(bis);
    double ZinsRestjahr = TageImLetztenJahr/360. *zins/100. *zwischenWert;
    gesamtZins += ZinsRestjahr;
    gesamtZins = r2(gesamtZins);
    qDebug().noquote()
        << "\nErstes Jahr : " << ZinsImErstenJahr << "(" << TageImErstenJahr << " Tage)"
        << "\nVolle Jahre : " << ZinsVolleJahre << "(" << jahre << " Jahre)"
        << "\nLetztes Jahr: " << ZinsRestjahr << "(" << TageImLetztenJahr << " Tage)"
        << "\nGesamtzins  : " << gesamtZins << Qt::endl;
    return r2(gesamtZins);
}
