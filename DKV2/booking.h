#ifndef BOOKING_H
#define BOOKING_H
#include <iso646.h>

#include "pch.h"

#include "helper.h"
#include "dbtable.h"

enum class bookingType{
    non, // means all
    deposit                = 1,
    payout                 = 2,
    reInvestInterest       = 4,
    annualInterestDeposit  = 8,
    setInterestActive      =16
};
QString bookingTypeDisplayString(const bookingType t);
inline int bookingTypeToInt(const bookingType t) { return static_cast<int>(t);}
inline bookingType fromInt(int i)
{
    switch(i)
    {
    case  0:
    case  1:
    case  2:
    case  4:
    case  8:
    case 16:
        return static_cast<bookingType>(i);
    default:
        Q_ASSERT(! "invalid booking type");
        qCritical() << "invalid booking type";
        return static_cast<bookingType>(0);
    }
}
inline QString bookingTypeToString( const bookingType t) {return QString::number(bookingTypeToInt(t));};

struct booking
{

    qlonglong contractId =-1;
    bookingType type =bookingType::non;
    QDate date =EndOfTheFuckingWorld;
    double amount =0.;
    // construction
    booking(const qlonglong cId =-1, const bookingType t = bookingType::non, const QDate d =EndOfTheFuckingWorld, const double a =0.)
        : contractId(cId), type(t), date(d), amount(a) {};
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

    friend bool bookDeposit(   const qlonglong contractId, QDate date, const double amount);
    friend bool bookPayout(    const qlonglong contractId, QDate date, const double amount);
    friend bool bookReInvestInterest(const qlonglong contractId, QDate date, const double amount);
    friend bool bookAnnualInterestDeposit( const qlonglong contractId, QDate date, const double amount);
    friend bool bookInterestActive(const qlonglong contractId, QDate date);
private:
    friend bool writeBookingToDB( bookingType , const qlonglong contractId, QDate date, const double amount);
};
Q_DECLARE_TYPEINFO(booking, Q_PRIMITIVE_TYPE );

bool bookDeposit(   const qlonglong contractId, QDate date, const double amount);
bool bookPayout(    const qlonglong contractId, QDate date, const double amount);
bool bookReInvestInterest(const qlonglong contractId, QDate date, const double amount);
bool bookAnnualInterestDeposit( const qlonglong contractId, QDate date, const double amount);
bool bookInterestActive(const qlonglong contractId, QDate date);



struct bookings
{
    static QDate dateOfnextSettlement();
    static QVector<booking> bookingsFromSql(const QString& where, const QString& order=QString(), bool terminated =false);
    static QVector<booking> getBookings(const qlonglong cid, const QDate from = BeginingOfTime,
            const QDate to = EndOfTheFuckingWorld, const QString order = qsl("Datum DESC"), bool terminatedContract =false);

    static QVector<booking> getAnnualSettelments(const int year);
    static QVector<int> yearsWithAnnualBookings();
};

#endif // BOOKING_H

