#ifndef BOOKING_H
#define BOOKING_H

#include <QDate>
#include "helper.h"
#include "dbtable.h"

struct booking
{
enum class Type{
    non, // means all
    deposit = 1, payout  = 2,
    reInvestInterest = 4,
    annualInterestDeposit = 8
    };
static QString displayString(Type t);

Type type =Type::non;
QDate date =EndOfTheFuckingWorld;
double amount =0.;
qlonglong contractId =-1;
    // construction
    booking(booking::Type t = Type::non, QDate d =EndOfTheFuckingWorld, double a =0.) : type(t), date(d), amount(a) {};

    // statics
    static const dbtable& getTableDef();
    static const dbtable& getTableDef_deletedBookings();
    static bool bookDeposit(   const qlonglong contractId, const QDate date, const double amount);
    static bool bookPayout(    const qlonglong contractId, const QDate date, const double amount);
    static bool bookReInvestInterest(const qlonglong contractId, const QDate date, const double amount);
    static bool bookAnnualInterestDeposit( const qlonglong contractId, const QDate date, const double amount);
    static QString typeName(Type t);
private:
    static bool doBooking( const booking::Type, const qlonglong contractId, const QDate date, const double amount);
};

struct bookings
{
    static QDate dateOfnextSettlement();
    static QVector<booking> bookingsFromSql(QString where, QString order=QString());
    static QVector<booking> getBookings(qlonglong cid, QDate from =BeginingOfTime, QDate to =EndOfTheFuckingWorld);
    static QVector<booking> getAnnualSettelments(int year);
};

#endif // BOOKING_H

const QVector<booking>& bookingsFromSql(QString& where);
