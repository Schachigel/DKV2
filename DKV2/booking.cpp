#include <QSqlQuery>

#include "contract.h"
#include "booking.h"

/* static */ const dbtable& booking::getTableDef()
{
    static dbtable bookings(qsl("Buchungen"));
    if( 0 == bookings.Fields().size()) {
        bookings.append(dbfield(qsl("id"),          QVariant::LongLong).setPrimaryKey().setAutoInc());
        bookings.append(dbfield(qsl("VertragsId"),  QVariant::LongLong).setDefault(0).setNotNull());
        bookings.append(dbForeignKey(bookings[qsl("VertragsId")], dkdbstructur[qsl("Vertraege")][qsl("id")], qsl("ON DELETE RESTRICT")));
        bookings.append(dbfield(qsl("Datum"),       QVariant::Date).setDefault(qsl("9999-12-31")).setNotNull());
        bookings.append(dbfield(qsl("BuchungsArt"), QVariant::Int).setDefault(0).setNotNull()); // deposit, interestDeposit, outpayment, interestPayment
        bookings.append(dbfield(qsl("Betrag"),      QVariant::Int).setDefault(0).setNotNull()); // in cent
    }
    return bookings;
}
/* static */ const dbtable& booking::getTableDef_deletedBookings()
{
    static dbtable deletedBookings(qsl("exBuchungen"));
    if( 0 == deletedBookings.Fields().size()) {
        deletedBookings.append(dbfield(qsl("id"), QVariant::LongLong).setPrimaryKey());
        for( int i =1 /* not 0 */ ; i < getTableDef().Fields().count(); i++) {
            deletedBookings.append(getTableDef().Fields()[i]);
        }
        deletedBookings.append(dbForeignKey(deletedBookings[qsl("VertragsId")],
                               dkdbstructur[qsl("exVertraege")][qsl("id")], qsl("ON DELETE RESTRICT")));
    }
    return deletedBookings;
}
/* static */ QString booking::displayString(Type t)
{
    switch(t)
    {
    case booking::Type::deposit :
        return qsl("Einzahlung");
    case booking::Type::payout :
        return qsl("Auszahlung");
    case booking::Type::interestDeposit:
        return qsl("Zinsanrechnung");
    case booking::Type::annualInterestDeposit:
        return qsl("Jahreszins");
    default:
        QString error{qsl("FEHLER: ungültiger Buchungstyp")};
        Q_ASSERT(true);
        return error;
    }
}
/* static */ QString booking::typeName(Type t)
{
    switch(t) {
    case booking::non: return qsl("invalid booking");
    case booking::deposit: return qsl("deposit");
    case booking::payout: return qsl("payout");
    case booking::interestDeposit: return qsl("reinvest interest");
    default: return qsl("ERROR");
    }
}

/* static */ bool booking::doBooking( const booking::Type t, const qlonglong contractId, const QDate date, const double amount)
{   LOG_CALL_W(typeName(t));
    TableDataInserter tdi(booking::getTableDef());
    tdi.setValue(qsl("VertragsId"), contractId);
    tdi.setValue(qsl("BuchungsArt"), t);
    tdi.setValue(qsl("Betrag"), ctFromEuro(amount));
    tdi.setValue(qsl("Datum"), date);
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

/* static */ QDate bookings::dateOfnextSettlement()
{   LOG_CALL;
    return  executeSingleValueSql(qsl("date"), qsl("NextAnnualSettlement")).toDate();
}
/* static */ QVector<booking> bookings::getBookings(qlonglong cid, QDate from, QDate to)
{   LOG_CALL;
    // used in tests
    QString where = qsl("Buchungen.VertragsId=%1 "
                  "AND Buchungen.Datum >='%2' "
                  "AND Buchungen.Datum <='%3'");
    where = where.arg(QString::number(cid), from.toString(Qt::ISODate), to.toString(Qt::ISODate));
    QVector<QSqlRecord> records = executeSql(booking::getTableDef().Fields(), where, qsl("Datum DESC"));
    QVector<booking> ret;
    for( auto& rec: qAsConst(records)) {
        booking::Type t = booking::Type(rec.value(qsl("BuchungsArt")).toInt());
        QDate d = rec.value(qsl("Datum")).toDate();
        double amount = euroFromCt(rec.value(qsl("Betrag")).toInt());
        ret.push_back(booking(t, d, amount));
    }
    return ret;
}
