#include <QSqlQuery>

#include "contract.h"
#include "booking.h"

/* static */ const dbtable& booking::getTableDef()
{
    static dbtable bookings("Buchungen");
    if( 0 == bookings.Fields().size())
    {
        bookings.append(dbfield("id",          QVariant::LongLong).setPrimaryKey().setAutoInc());
        bookings.append(dbfield("VertragsId",  QVariant::LongLong).setDefault(0).setNotNull());
        bookings.append(dbForeignKey(bookings["VertragsId"], dkdbstructur["Vertraege"]["id"], "ON DELETE RESTRICT"));
        bookings.append(dbfield("BuchungsArt", QVariant::Int).setDefault(0).setNotNull()); // deposit, interestDeposit, outpayment, interestPayment
        bookings.append(dbfield("Betrag",      QVariant::Int).setDefault(0).setNotNull()); // in cent
        bookings.append(dbfield("Datum",       QVariant::Date).setDefault("9999-12-31").setNotNull());
    }
    return bookings;
}

/* static */ const QString booking::typeName( booking::Type t)
{
    switch (t)
    {
    case booking::Type::deposit:
        return "deposit";
    case booking::Type::payout:
        return "payout";
    case booking::Type::interestDeposit:
        return "reinvestment";
    case booking::Type::interestPayout:
        return "interest payout";
    default:
        return "invalid booking type";
    }
}

/* static */ bool booking::doBooking( const booking::Type t, const qlonglong contractId, const QDate date, const double amount)
{   LOG_CALL_W(typeName(t));
    TableDataInserter tdi(dkdbstructur["Buchungen"]);
    tdi.setValue("VertragsId", contractId);
    tdi.setValue("BuchungsArt", t);
    tdi.setValue("Betrag", round2(amount));
    tdi.setValue("Datum", date);
    return tdi.InsertData();
}

/* static */ bool booking::makeDeposit(const qlonglong contractId, const QDate date, const double amount)
{
    Q_ASSERT( amount > 0.);
    return doBooking( Type::deposit, contractId, date, amount);
}
/* static */ bool booking::makePayout(qlonglong contractId, QDate date, double amount)
{
    if( amount > 0. ) amount = -1. * amount;
    // todo: check account
    return doBooking(Type::payout, contractId, date, amount);

}
/* static */ bool booking::investInterest(qlonglong contractId, QDate date, double amount)
{
    Q_ASSERT( amount > 0.);
    return doBooking(Type::interestDeposit, contractId, date, amount);
}
/* static */ bool booking::payoutInterest(qlonglong contractId, QDate date, double amount)
{
    Q_ASSERT( amount > 0.);
    return doBooking(Type::interestPayout, contractId, date, amount);
}

/*
 * BookingS is about getting collections of bookings
*/

QVector<booking> bookings::getBookings()
{   LOG_CALL;
    QVector<QSqlRecord> rec = executeSql(dkdbstructur["Buchungen"].Fields(),
            "VertragsId=" + QString::number(contractId) + " AND BuchungsArt=" + QString::number(type));
    if( rec.isEmpty()) {
        qDebug() << "could not query bookings for " << contractId << " type " << booking::typeName(type);
        return QVector<booking>();
    }
    QVector<booking> result;
    for( auto r : rec) {
        result.push_back(booking(type, r.value("Datum").toDate(), r.value("Betrag").toDouble()));
    }
    return result;
}

double bookings::sumBookings()
{   LOG_CALL;
    QSqlQuery q;
    if( !q.exec("SELECT SUM(Betrag) FROM Buchungen "
                "WHERE VertragsId=" + QString::number(contractId)
                + " AND BuchungsArt=" + QString::number(type)))
    {
        qDebug() << "could not query sum of bookings for " << contractId << " type " << booking::typeName(type);
        return 0.;
    }
    q.first();
    return q.value(0).toDouble();
}
