
#include <math.h>
#include <QDebug>

#include "finhelper.h"
#include "financaltimespan.h"

double round(const double d, const int stellen)
{
    return qRound(d * pow(10,stellen))/pow(10,stellen);
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
    Q_ASSERT(von.year() == bis.year());
    if( bis.day() == 31)
        return TageZwischen(von, QDate(bis.year(), bis.month(), 30));
    int days =0;
    if(von.month() == bis.month())
    {
        days += bis.day() - von.day();
    }
    else
    {
        days += 30* (bis.month() - von.month()-1);
        days += TageBisMonatsende_exclusiv(von);
        days += TageSeitMonatsAnfang_inclusive(bis);
    }
    return days;
}

int TageBisJahresende_a(const QDate& d)
{
    if( d.day()==31)
        // der 31. wird behandelt wie der 30.
        return TageBisJahresende_a(QDate(d.year(), d.month(),30));
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
    return 30* (d.month()-1) + (d.day()==31 ? 30 : d.day());
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
    if( von > bis)
        qCritical() << "Zinseszins kann nicht berechnet werden - ungültige Parameter";
    if( von.year() == bis.year())
    {
        return TageZwischen(von, bis);
    }
    int tage = TageBisJahresende(von);
    tage += 360* (bis.year() - von.year() -1);
    tage += TageSeitJahresAnfang(bis);
    return tage*zins*wert;
}
