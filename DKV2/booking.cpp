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
/* static */ const dbtable& booking::getTableDef_deletedBookings()
{
    static dbtable deletedBookings("exBuchungen");
    deletedBookings.append(dbfield("id", QVariant::LongLong).setPrimaryKey());
    for( int i =1 /* not 0 */ ; i < getTableDef().Fields().count(); i++) {
        deletedBookings.append(getTableDef().Fields()[i]);
    }

    deletedBookings.append(dbForeignKey(deletedBookings["VertragsId"], dkdbstructur["exVertraege"]["id"], "ON DELETE RESTRICT"));
    return deletedBookings;
}

/* static */ QString booking::typeName(Type t)
{
    switch(t) {
    case booking::non: return "invalid booking";
    case booking::deposit: return "deposit";
    case booking::payout: return "payout";
    case booking::interestDeposit: return "reinvest interest";
    case booking::interestPayout: return "interest payout";
    default: return "ERROR";
    }
}

/* static */ bool booking::doBooking( const booking::Type t, const qlonglong contractId, const QDate date, const double amount)
{   LOG_CALL_W(typeName(t));
    TableDataInserter tdi(booking::getTableDef());
    tdi.setValue("VertragsId", contractId);
    tdi.setValue("BuchungsArt", t);
    tdi.setValue("Betrag", ctFromEuro(amount));
    tdi.setValue("Datum", date);
    if( tdi.InsertData()) {
        qInfo() << "successful booking: " << typeName(t) << " contract#: " << contractId << " Amount: " << ctFromEuro(amount) << " date: " << date;
        return true;
    }
    qCritical() << "booking failed for contract#: " << contractId << " Amount: " << ctFromEuro(amount) << " date: " << date;;
    return false;
}
/* static */ bool booking::makeDeposit(const qlonglong contractId, const QDate date, const double amount)
{
    Q_ASSERT( amount > 0.);
    return doBooking( Type::deposit, contractId, date, amount);
}
/* static */ bool booking::makePayout(qlonglong contractId, QDate date, double amount)
{
    if( amount > 0. ) amount = -1. * amount;
    // contract has to check that a payout is possible
    return doBooking(Type::payout, contractId, date, amount);

}
/* static */ bool booking::investInterest(qlonglong contractId, QDate date, double amount)
{
    Q_ASSERT( amount >= 0.);
    return doBooking(Type::interestDeposit, contractId, date, amount);
}
/* static */ bool booking::payoutInterest(qlonglong contractId, QDate date, double amount)
{
    if( amount > 0) amount *= -1.;
    return doBooking(Type::interestPayout, contractId, date, amount);
}

/* static */ QDate bookings::dateOfnextSettlement()
{   LOG_CALL;
    return  executeSingleValueSql("date", "NextAnnualSettlement").toDate();
}
/* static */ QVector<booking> bookings::getBookings(qlonglong cid, QDate from, QDate to)
{   LOG_CALL;
    QString where = "Buchungen.VertragsId=%1 "
                  "AND Buchungen.Datum >='%2' "
                  "AND Buchungen.Datum <='%3'";
    where = where.arg(QString::number(cid)).arg(from.toString(Qt::ISODate)).arg(to.toString(Qt::ISODate));
    QVector<QSqlRecord> records = executeSql(booking::getTableDef().Fields(), where, "Datum DESC");
    QVector<booking> ret;
    for( auto rec: records) {
        booking::Type t = booking::Type(rec.value("BuchungsArt").toInt());
        QDate d = rec.value("Datum").toDate();
        double amount = euroFromCt(rec.value("Betrag").toInt());
        ret.push_back(booking(t, d, amount));
    }
    return ret;
}
