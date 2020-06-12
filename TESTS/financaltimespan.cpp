#include <QDate>

#include "financaltimespan.h"

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


