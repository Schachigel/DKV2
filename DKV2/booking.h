#ifndef BOOKING_H
#define BOOKING_H

#include "helper.h"
#include "tabledatainserter.h"
#include "dkdbhelper.h"


struct booking
{
enum type{
    non,
    deposit = 1,
    payout  = 2,
    interestDeposit = 4,
    interestPayout  = 8
};

    booking();
    static const dbtable& getTableDef();
    static bool makeDeposit(   const qlonglong contractId, const double amount, const QDate date);
    static bool makePayout(    const qlonglong contractId, const double amount, const QDate date);
    static bool investInterest(const qlonglong contractId, const double amount, const QDate date);
    static bool payoutInterest(const qlonglong contractId, const double amount, const QDate date);
    static const QString typeName(booking::type t);
private:
    static bool doBooking( const booking::type, const qlonglong contractId, const double amount, const QDate date);
};

struct bookings
{
    struct data {
        double amount;
        QDate date;
    };

    bookings(qlonglong contractid, booking::type type = booking::type::non)
        : contractId(contractid), type(type){};
    QVector<data> getBookings();
    //QVector<data> getBookings(QDate from, QDate to = EndOfTheFuckingWorld);
    double sumBookings();
    //double sumBookings(QDate from, QDate to = EndOfTheFuckingWorld);
private:
    qlonglong contractId;
    booking::type type;
};

#endif // BOOKING_H
