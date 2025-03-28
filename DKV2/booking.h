#ifndef BOOKING_H
#define BOOKING_H
#include <iso646.h>

#include "helper.h"
#include "helperfin.h"
#include "helpersql.h"
#include "dbtable.h"

inline const QString tn_Buchungen   {qsl("Buchungen")};
inline const QString tn_ExBuchungen {qsl("ExBuchungen")};

inline const QString fn_bVertragsId {qsl("VertragsId")};
inline const QString fn_bBuchungsArt{qsl("BuchungsArt")};
inline const QString fn_bBetrag     {qsl("Betrag")};
inline const QString fn_bDatum      {qsl("Datum")};
inline const QString fn_bModifiziert{qsl("Überschrieben")};

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
    case  0: // non
    case  1: // deposit
    case  2: // payout
    case  4: // reinvest
    case  8: // annualI.Deposit
    case 16: // setI.Active
        return static_cast<bookingType>(i);
    default:
        Q_ASSERT(! "invalid booking type");
        qCritical() << "invalid booking type";
        return static_cast<bookingType>(0);
    }
}
inline QString bookingTypeToNbrString( const bookingType t) {return i2s(bookingTypeToInt(t));};

struct booking
{
    tableindex_t contractId =SQLITE_invalidRowId;
    bookingType type =bookingType::non;
    QDate date =EndOfTheFuckingWorld;
    double amount =0.;
    // construction
    booking(const tableindex_t cId =SQLITE_invalidRowId, const bookingType t = bookingType::non,
            const QDate d =EndOfTheFuckingWorld, const double a =0.)
        : contractId(cId), type(t), date(d), amount(a) {};
    // statics
    static const dbtable& getTableDef();
    static const dbtable& getTableDef_deletedBookings();

    QString toString() const;
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
         else
             RETURN_ERR(false, lhs.toString (), rhs.toString ());
     }
};
Q_DECLARE_TYPEINFO(booking, Q_PRIMITIVE_TYPE );

bool bookingToDB(bookingType, const tableindex_t contrId, QDate date, const double);

bool bookDeposit(   const tableindex_t contractId, QDate date, const double amount);
bool bookPayout(    const tableindex_t contractId, QDate date, const double amount);
bool bookReInvestInterest(const tableindex_t contractId, QDate date, const double amount);
bool bookAnnualInterestDeposit( const tableindex_t contractId, QDate date, const double amount);
bool bookInterestActive(const tableindex_t contractId, QDate date);
bool writeBookingUpdate( qlonglong bookingId, int newValeuInCt);

//QVector<booking> bookingsFromDB(const QString& where, const QString& order ="", bool terminated =false);
QVector<booking> getBookings(   const tableindex_t contractId,  const QDate from = BeginingOfTime, const QDate to = EndOfTheFuckingWorld,
                                const QString order = qsl("Datum DESC"), bool terminatedContract =false);
QVector<booking> getExBookings(   const tableindex_t contractId,  const QDate from = BeginingOfTime, const QDate to = EndOfTheFuckingWorld,
                                const QString order = qsl("Datum DESC"));
QDate dateOfnextSettlement();
int getNbrOfBookings(const tableindex_t contract, const QDate from =BeginingOfTime, const QDate to =EndOfTheFuckingWorld, const bool terminated =false);
int getNbrOfExBookings(const tableindex_t contract, const QDate from =BeginingOfTime, const QDate to =EndOfTheFuckingWorld);

double getBookingsSum(QVector<booking> bl, bookingType bt);

QVector<booking> getAnnualSettlements(const int year);
QVector<int> yearsWithAnnualBookings();

#endif // BOOKING_H

