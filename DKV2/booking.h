#ifndef BOOKING_H
#define BOOKING_H

#include <QDate>
#include "helper.h"
#include "dbtable.h"

struct booking
{
enum Type{
    non, // means all
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
    // interface
    bool isInterestBooking() const { return isInterestBooking(type);}

    // statics
    static const dbtable& getTableDef();
    static const dbtable& getTableDef_deletedContracts();
    static bool makeDeposit(   const qlonglong contractId, const QDate date, const double amount);
    static bool makePayout(    const qlonglong contractId, const QDate date, const double amount);
    static bool investInterest(const qlonglong contractId, const QDate date, const double amount);
    static bool payoutInterest(const qlonglong contractId, const QDate date, const double amount);
    static const QString typeName(booking::Type t);
    static bool isInterestBooking(Type t);
    bool executeBooking();
private:
    static bool doBooking( const booking::Type, const qlonglong contractId, const QDate date, const double amount);
};

struct bookings
{
    static QDate dateOfnextSettlement();
    static QVector<booking> getBookings(qlonglong cid, QDate from =BeginingOfTime, QDate to =EndOfTheFuckingWorld);
};

#endif // BOOKING_H
