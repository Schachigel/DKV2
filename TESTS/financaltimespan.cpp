#include <QDate>

#include "financaltimespan.h"

int TageBisJahresende_lookup(const QDate& d)
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

int TageSeitJahresAnfang_lookup(const QDate& d)
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


