#ifndef BOOKING_H
#define BOOKING_H

#include "helper_core.h"
#include "helperfin.h"
#include "helpersql.h"
#include "dbtable.h"
#include "idwrapper.h"

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
inline bookingType bookingtypeFromInt(int i)
{
    switch(i)
    {
    case  0: // non / all
    case  1: // deposit
    case  2: // payout
    case  4: // reinvest
    case  8: // annualI.Deposit
    case 16: // setI.Active
        return static_cast<bookingType>(i);
    default:
        qCritical() << "invalid booking type";
        return bookingType::non;
    }
}
inline QString bookingTypeToNbrString( const bookingType t) {return i2s(bookingTypeToInt(t));};

struct booking
{
    contractId_t contId =Invalid_contract_id;
    bookingType type =bookingType::non;
    QDate date =EndOfTheFuckingWorld;
    double amount =0.;
    // construction
    booking(const contractId_t cId =Invalid_contract_id, const bookingType t = bookingType::non,
            const QDate d =EndOfTheFuckingWorld, const double a =0.)
        : contId(cId), type(t), date(d), amount(a) {};
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
         if( (lhs.contId.v not_eq rhs.contId.v)) error += qsl(", comparing bookings: different contractIds");
         if(error.isEmpty())
             return true;
         else
             RETURN_ERR(false, lhs.toString (), rhs.toString ());
     }
};
Q_DECLARE_TYPEINFO(booking, Q_PRIMITIVE_TYPE );

bool writeBookingToDB(bookingType, const contractId_t contrId, QDate date, const double);

bool bookDeposit(   const contractId_t cId, QDate date, const double amount);
bool bookPayout(    const contractId_t cId, QDate date, const double amount);
bool bookReInvestInterest(const contractId_t cId, QDate date, const double amount);
bool bookAnnualInterestDeposit( const contractId_t cId, QDate date, const double amount);
bool bookInterestActive(const contractId_t cId, QDate date);
bool writeBookingUpdate( bookingId_t bookingId, int newValeuInCt);

//QVector<booking> bookingsFromDB(const QString& where, const QString& order ="", bool terminated =false);
QVector<booking> getBookings(   const contractId_t contractId,  const QDate from = BeginingOfTime, const QDate to = EndOfTheFuckingWorld,
                                const QString order = qsl("Datum DESC"), bool terminatedContract =false);

int getNbrOfBookings(const contractId_t contract, const QDate from =BeginingOfTime, const QDate to =EndOfTheFuckingWorld, const bool terminated =false);
int getNbrOfExBookings(const contractId_t contract, const QDate from =BeginingOfTime, const QDate to =EndOfTheFuckingWorld);

double getBookingsSum(QVector<booking> bl, bookingType bt);

QVector<int> yearsWithAnnualBookings();

#endif // BOOKING_H

