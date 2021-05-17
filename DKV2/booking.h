#ifndef BOOKING_H
#define BOOKING_H
#include <iso646.h>

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
    static QString displayString(const Type t);
    inline static int bookingTypeToInt(const booking::Type t) {
        return static_cast<int>(t);
    }

    qlonglong contractId =-1;
    Type type =Type::non;
    QDate date =EndOfTheFuckingWorld;
    double amount =0.;
    // construction
    booking(const qlonglong cId, const booking::Type t = Type::non, const QDate d =EndOfTheFuckingWorld, const double a =0.) : contractId(cId), type(t), date(d), amount(a) {};
    // comparison for tests
    inline friend bool operator==(const booking& lhs, const booking& rhs)
    {
        QString error;
        if( (lhs.type not_eq rhs.type)) error =qsl("comparing bookings: different types");
        if( (lhs.date not_eq rhs.date)) error += qsl(", comparing bookings: different dates");
        if( (lhs.amount not_eq rhs.amount)) error += qsl(", comparing bookings: different amounts");
        if( (lhs.contractId not_eq rhs.contractId)) error += qsl(", comparing bookings: different contractIds");
        if(error.isEmpty())
            return true;
        else {
            qInfo() << error;
            return false;
        }
    }
    // statics
    static const dbtable& getTableDef();
    static const dbtable& getTableDef_deletedBookings();
    static bool bookDeposit(   const qlonglong contractId, QDate date, const double amount);
    static bool bookPayout(    const qlonglong contractId, QDate date, const double amount);
    static bool bookReInvestInterest(const qlonglong contractId, QDate date, const double amount);
    static bool bookAnnualInterestDeposit( const qlonglong contractId, QDate date, const double amount);
    static QString typeName(booking::Type t);
private:
    static bool doBooking( booking::Type , const qlonglong contractId, QDate date, const double amount);
};
Q_DECLARE_TYPEINFO(booking, Q_PRIMITIVE_TYPE );

struct bookings
{
    static QDate dateOfnextSettlement();
    static QVector<booking> bookingsFromSql(const QString& where, const QString& order=QString());
    static QVector<booking> getBookings(const qlonglong cid, const QDate from =BeginingOfTime, const QDate to =EndOfTheFuckingWorld);
    static QVector<booking> getAnnualSettelments(const int year);
};

#endif // BOOKING_H

const QVector<booking>& bookingsFromSql(const QString& where);
