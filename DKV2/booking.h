#ifndef BOOKING_H
#define BOOKING_H

#include <QDate>

#include "dbtable.h"

struct booking
{
enum Type{
    non,
    deposit = 1,
    payout  = 2,
    interestDeposit = 4,
    interestPayout  = 8
};
Type type;
QDate date;
double amount;
qlonglong contractId;
    // construction
    booking(booking::Type t, QDate d, double a) : type(t), date(d), amount(a) {};

    // statics
    static const dbtable& getTableDef();
    static bool makeDeposit(   const qlonglong contractId, const QDate date, const double amount);
    static bool makePayout(    const qlonglong contractId, const QDate date, const double amount);
    static bool investInterest(const qlonglong contractId, const QDate date, const double amount);
    static bool payoutInterest(const qlonglong contractId, const QDate date, const double amount);
    static const QString typeName(booking::Type t);
private:
    static bool doBooking( const booking::Type, const qlonglong contractId, const QDate date, const double amount);
};

struct bookings
{

    bookings(qlonglong contractid, booking::Type type = booking::Type::non)
        : contractId(contractid), type(type){};
    QVector<booking> getBookings();
    //QVector<data> getBookings(QDate from, QDate to = EndOfTheFuckingWorld);
    double sumBookings();
    //double sumBookings(QDate from, QDate to = EndOfTheFuckingWorld);
private:
    qlonglong contractId;
    booking::Type type;
};

#endif // BOOKING_H
