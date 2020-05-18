#include "contract.h"
#include "booking.h"

booking::booking()
{
}

/* static */ const dbtable& booking::getTableDef()
{
    static dbtable bookings("Buchungen");
    if( 0 == bookings.Fields().size())
    {
        bookings.append(dbfield("id",  QVariant::LongLong, "PRIMARY KEY AUTOINCREMENT"));
        bookings.append(dbfield("VertragsId", QVariant::LongLong, "", contract::getTableDef()["id"], dbfield::refIntOption::onDeleteCascade));
        bookings.append(dbfield("BuchungsArt", QVariant::Int, "DEFAULT 0 NOT NULL")); // deposit, interestDeposit, outpayment, interestPayment
        bookings.append(dbfield("Datum", QVariant::Date, "DEFAULT '9999-12-31' NOT NULL"));
        bookings.append(dbfield("Betrag", QVariant::Double, "DEFAULT '0.0' NOT NULL"));
    }
    return bookings;
}
