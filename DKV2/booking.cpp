#include <QSqlQuery>

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
        bookings.append(dbfield("Betrag", QVariant::Double, "DEFAULT '0.0' NOT NULL"));
        bookings.append(dbfield("Datum", QVariant::Date, "DEFAULT '9999-12-31' NOT NULL"));
    }
    return bookings;
}

/* static */ const QString booking::typeName( booking::type t)
{
    switch (t)
    {
    case booking::type::deposit:
        return "deposit";
    case booking::type::payout:
        return "payout";
    case booking::type::interestDeposit:
        return "reinvestment";
    case booking::type::interestPayout:
        return "interest payout";
    default:
        return "invalid booking type";
    }
}

/* static */ bool booking::doBooking( const booking::type t, const qlonglong contractId, const double amount, const QDate date)
{   LOG_CALL_W(typeName(t));
    TableDataInserter tdi(dkdbstructur["Buchungen"]);
    tdi.setValue("VertragsId", contractId);
    tdi.setValue("BuchungsArt", t);
    tdi.setValue("Betrag", round2(amount));
    tdi.setValue("Datum", date);
    return tdi.InsertData();
}

/* static */ bool booking::makeDeposit(const qlonglong contractId, const double amount, const QDate date)
{
    Q_ASSERT( amount > 0.);
    return doBooking( type::deposit, contractId, amount, date);
}
/* static */ bool booking::makePayout(qlonglong contractId, double amount, QDate date)
{
    if( amount > 0. ) amount = -1. * amount;
    // todo: check account
    return doBooking(type::payout, contractId, amount, date);

}
/* static */ bool booking::investInterest(qlonglong contractId, double amount, QDate date)
{
    Q_ASSERT( amount > 0.);
    return doBooking(type::interestDeposit, contractId, amount, date);
}
/* static */ bool booking::payoutInterest(qlonglong contractId, double amount, QDate date)
{
    Q_ASSERT( amount > 0.);
    return doBooking(type::interestPayout, contractId, amount, date);
}

QVector<bookings::data> bookings::getBookings()
{   LOG_CALL;
    QSqlQuery q;
    if( !q.exec("SELECT * FROM Buchungen "
              "WHERE VertragsId=" + QString::number(contractId)
              + " AND BuchungsArt=" + QString::number(type)))
    {
        qDebug() << "could not query bookings for " << contractId << " type " << booking::typeName(type);
        return QVector<bookings::data>();
    }
    QVector<bookings::data> result;
    while( q.next())
        result.push_back({q.value("Betrag").toDouble(), q.value("Datum").toDate()});
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
