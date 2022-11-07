#include <iso646.h>

#include "helper.h"
#include "helperfin.h"
#include "appconfig.h"

int TageZwischen_30_360(QDate von, QDate bis)
{   // finanzmathematischer Abstand zwischen zwei Daten im selben Jahr
    Q_ASSERT( von.year() == bis.year());
    if( von.year() not_eq bis.year()) {
        qCritical() << __FUNCTION__ << " ungültige Argumente: " << von << " - " << bis;
        return -1;
    }
    if( bis == von) return 0;
    Q_ASSERT(bis > von);
    if( bis.day() == 31)
        return TageZwischen_30_360(von, QDate(bis.year(), bis.month(), 30));
    if( von.day() == 31)
        return TageZwischen_30_360(QDate(von.year(), von.month(), 30), bis);
    if( von.month() == bis.month())
        return bis.day() -von.day();

    int tageErsterMonat = 30-von.day();
    int tageVolleMonate = 30* (bis.month() -von.month() -1);
    int tageLetzterMonat = bis.day();
    qInfo() << "TageZwischen (" << von << ") und (" << bis << "): "
             << tageErsterMonat << " + " << tageVolleMonate << " + " << tageLetzterMonat
             << " =" << tageErsterMonat+tageVolleMonate+tageLetzterMonat;
    return tageErsterMonat + tageVolleMonate + tageLetzterMonat;
}

int TageBisJahresende_30_360(QDate d)
{
    if( not d.isValid ())
        qCritical() << "Invalid date for interest calculation";
    if( d.day()==d.daysInMonth()) {
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
int TageSeitJahresAnfang_30_360(QDate d)
{
    return 30* (d.month()-1) + ((d.day() == 31) ? 30 : d.day());
}
double ZinsesZins_30_360(const double zins, const double wert,const QDate von, const QDate bis, const bool thesa)
{
    qInfo().noquote() << "\n\nZinsberechnung (30/360) von " << von << " bis " << bis << ((thesa) ? (" thesaurierend\n") : ("ausschüttend\n"))
                       << "Wert: " << d2euro(wert) << " Zinssatz: " << prozent2prozent_str (zins) << "\n";
    if( not (von.isValid() and bis.isValid())
            or ( von > bis)
            or ( wert <= 0.)) {
        qCritical() << "Zinseszins kann nicht berechnet werden - ungültige Parameter";
        return -1.;
    }
    if( bis.year () - von.year () > 1) {
        qCritical() << "Zinszeitraum über mehrere Jahre";
    }

    if( von.year() == bis.year()) {
        return r2(TageZwischen_30_360(von, bis)/360. *zins/100. *wert);
    }
    int TageImErstenJahr = TageBisJahresende_30_360(von); // first day no intrest
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
    int TageImLetztenJahr = TageSeitJahresAnfang_30_360(bis);
    double ZinsRestjahr = TageImLetztenJahr/360. *zins/100. *zwischenWert;
    gesamtZins += ZinsRestjahr;
    gesamtZins = r2(gesamtZins);
    qInfo().noquote()
        << "\nErstes Jahr : " << ZinsImErstenJahr << "(" << TageImErstenJahr << " Tage)"
        << "\nVolle Jahre : " << ZinsVolleJahre << "(" << jahre << " Jahre)"
        << "\nLetztes Jahr: " << ZinsRestjahr << "(" << TageImLetztenJahr << " Tage)"
        << "\nGesamtzins  : " << gesamtZins << qsl("\n");
    return r2(gesamtZins);
}

int TageZwischen_act_act(QDate von, QDate bis)
{
    Q_ASSERT(von.year () == bis.year());
    Q_ASSERT (bis >= von);

    int days =von.daysTo(bis);
    qInfo() << "Tage(a/a) von " << von << " bis " << bis << ": " << days;
    return days;
}
int TageBisJahresende_act_act(QDate date)
{
    return date.daysTo (QDate(date.year(), 12, 31));
}
int TageSeitJahresAnfang_act_act(QDate date)
{
    return QDate(date.year()-1, 12, 31).daysTo (date);
}

double ZinsesZins_act_act(const double zins, const double wert,const QDate von, const QDate bis, const bool thesa)
{
    qInfo().noquote() << "\n\nZinsberechnung (act/act) von " << von << " bis " << bis << ((thesa) ? (" thesaurierend\n") : ("ausschüttend\n"));
    if( not (von.isValid() and bis.isValid()) or ( von > bis)) {
        qCritical() << "Zinseszins kann nicht berechnet werden - ungültige Parameter";
        return -1.;
    }
    if( bis.year () - von.year () > 1) {
        qCritical() << "Zinszeitraum über mehrere Jahre";
    }
    if( wert <= 0.) return 0.;

    double dpy =QDate::isLeapYear (von.year ()) ? 366 : 365;
    if( von.year() == bis.year()) {

        return r2(TageZwischen_act_act(von, bis)/dpy *zins/100. *wert);
    }
    int TageImErstenJahr = TageBisJahresende_act_act(von); // first day no intrest
    double ZinsImErstenJahr = TageImErstenJahr/dpy *zins/100. *wert;
    double gesamtZins (ZinsImErstenJahr);

    double zwischenWert = (thesa) ? (wert+ZinsImErstenJahr) : (wert);
    double ZinsVolleJahre = 0.;
    int jahre(0);
    for( jahre=0; jahre < bis.year()-von.year()-1; jahre++) {
        double JahresZins = zwischenWert *zins/100.;
        gesamtZins += JahresZins; ZinsVolleJahre += JahresZins;
        zwischenWert = (thesa) ? (zwischenWert+JahresZins) : zwischenWert;
    }
    int dp_final_y =QDate::isLeapYear (bis.year ())? 366 : 365;
    double TageImLetztenJahr = TageSeitJahresAnfang_act_act(bis);
    double ZinsRestjahr = TageImLetztenJahr/dp_final_y *zins/100. *zwischenWert;
    gesamtZins += ZinsRestjahr;
    gesamtZins = r2(gesamtZins);
    qInfo().noquote()
        << "\nverzl.Guthaben:" << wert
        << "\nErstes Jahr : " << ZinsImErstenJahr << "(" << TageImErstenJahr << "/" << dpy << ") Tage)"
        << "\nVolle Jahre : " << ZinsVolleJahre << "(" << jahre << " Jahre)"
        << "\nLetztes Jahr: " << ZinsRestjahr << "(" << TageImLetztenJahr <<  "/" << dp_final_y << " Tage)"
        << "\nGesamtzins  : " << gesamtZins << qsl("\n");
    return r2(gesamtZins);

}

double ZinsesZins(const double zins, const double wert,const QDate von, const QDate bis, const bool thesa)
{
    QString susance =dbConfig::readString (ZINSUSANCE);
    qInfo() << "Zinssusance configured in database: " << susance;
    if(  susance == qsl("act/act"))
        return ::ZinsesZins_act_act (zins, wert, von, bis, thesa);
    else
        return ::ZinsesZins_30_360(zins, wert, von, bis, thesa);
}
