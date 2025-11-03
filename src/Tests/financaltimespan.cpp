#include "../DKV2/pch.h"

#include "financaltimespan.h"

int t_helper_TageBisJahresende_lookup(const QDate& d)
{
    /*
     * analog zur Excel Funktion
     * DAYS360( date( d.y; d.m; d.d); date( d.y; 12; 31); true)
     *
     */
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

int t_helper_TageSeitJahresAnfang_lookup(const QDate& d)
{
    /*
     * analog zur Excel Funktion
     * DAYS360( date( d.y-1; 12; 31); date( d.y; d.m; d.d); true)
     *
     */
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
