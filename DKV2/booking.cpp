#include <QSqlQuery>
#include <QtMath>

#include "contract.h"
#include "booking.h"

/* static */ const dbtable& booking::getTableDef()
{
    static dbtable bookingsTable(qsl("Buchungen"));
    if( 0 == bookingsTable.Fields().size()) {
        bookingsTable.append(dbfield(qsl("id"),          QVariant::LongLong).setPrimaryKey().setAutoInc());
        bookingsTable.append(dbfield(qsl("VertragsId"),  QVariant::LongLong).setNotNull());
        bookingsTable.append(dbForeignKey(bookingsTable[qsl("VertragsId")], dkdbstructur[qsl("Vertraege")][qsl("id")], qsl("ON DELETE RESTRICT")));
        bookingsTable.append(dbfield(qsl("Datum"),       QVariant::Date).setDefault(qsl("9999-12-31")).setNotNull());
        bookingsTable.append(dbfield(qsl("BuchungsArt"), QVariant::Int).setNotNull()); // deposit, interestDeposit, outpayment, interestPayment
        bookingsTable.append(dbfield(qsl("Betrag"),      QVariant::Int).setNotNull()); // in cent
        bookingsTable.append(dbfield(qsl("Zeitstempel"),  QVariant::DateTime).setDefaultNow());
    }
    return bookingsTable;
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
/* static */ QString booking::displayString(const Type& t)
{
    switch(t)
    {
    case booking::Type::deposit :
        return qsl("Einzahlung");
    case booking::Type::payout :
        return qsl("Auszahlung");
    case booking::Type::reInvestInterest:
        return qsl("Zinsanrechnung");
    case booking::Type::annualInterestDeposit:
        return qsl("Jahreszins");
    default:
        QString error{qsl("FEHLER: ungültiger Buchungstyp")};
        Q_ASSERT(true);
        return error;
    }
}
/* static */ QString booking::typeName(const Type& t)
{
    switch(t) {
    case booking::Type::non: return qsl("invalid booking");
    case booking::Type::deposit: return qsl("deposit");
    case booking::Type::payout: return qsl("payout");
    case booking::Type::reInvestInterest: return qsl("reinvest interest");
    case booking::Type::annualInterestDeposit: return qsl("annual Interest");
    default: return qsl("ERROR");
    }
}

/* static */ bool booking::doBooking( const booking::Type& t, const qlonglong contractId, const QDate& date, const double amount)
{   LOG_CALL_W(typeName(t));
    TableDataInserter tdi(booking::getTableDef());
    tdi.setValue(qsl("VertragsId"), contractId);
    tdi.setValue(qsl("BuchungsArt"), static_cast<int>(t));
    tdi.setValue(qsl("Betrag"), ctFromEuro(amount));
    tdi.setValue(qsl("Datum"), date);
    tdi.setValue(qsl("Ausführung"), QDateTime(QDate::currentDate(), QTime::currentTime()));
    if( -1 != tdi.InsertData()) {
        qInfo() << "successful booking: " << typeName(t) << " contract#: " << contractId << " Amount: " << ctFromEuro(amount) << " date: " << date;
        return true;
    }
    qCritical() << "booking failed for contract#: " << contractId << " Amount: " << ctFromEuro(amount) << " date: " << date;;
    return false;
}
/* static */ bool booking::bookDeposit(const qlonglong contractId, const QDate& date, const double amount)
{
    Q_ASSERT( amount > 0.);
    return doBooking( Type::deposit, contractId, date, amount);
}
/* static */ bool booking::bookPayout(const qlonglong contractId, const QDate& date, const double amount)
{
    // contract has to check that a payout is possible
    return doBooking(Type::payout, contractId, date, -1*qFabs(amount));
}
/* static */ bool booking::bookReInvestInterest(const qlonglong contractId, const QDate& date, const double amount)
{
    Q_ASSERT( amount >= 0.);
    return doBooking(Type::reInvestInterest, contractId, date, amount);
}
/* static */ bool booking::bookAnnualInterestDeposit(const qlonglong contractId, const QDate& date, const double amount)
{
    Q_ASSERT( amount >= 0.);
    return doBooking(Type::annualInterestDeposit, contractId, date, amount);
}

/* static */ QDate bookings::dateOfnextSettlement()
{   LOG_CALL;
    return  executeSingleValueSql(qsl("date"), qsl("vNextAnnualSettlement")).toDate();
}
/*static */ QVector<booking> bookings::bookingsFromSql(const QString& where, const QString& order)
{
    QVector<QSqlRecord> records = executeSql(booking::getTableDef().Fields(), where, order);
    QVector<booking> vRet;
    for (auto& rec : qAsConst(records)) {
        qlonglong cid = rec.value(qsl("VertragsId")).toLongLong();
        booking::Type t = booking::Type(rec.value(qsl("BuchungsArt")).toInt());
        QDate d = rec.value(qsl("Datum")).toDate();
        double amount = euroFromCt(rec.value(qsl("Betrag")).toInt());
        qInfo() << "Buchung: cid=" << cid << "; type=" << booking::displayString(t) << "; Datum=" << d.toString() << "; Betrag=" << amount;
        vRet.push_back(booking(cid, t, d, amount));
    }
    return vRet;
}

/* static */ QVector<booking> bookings::getBookings(const qlonglong cid, const QDate& from, const QDate& to)
{   LOG_CALL;
    // used in tests
    QString where = qsl("Buchungen.VertragsId=%1 "
                  "AND Buchungen.Datum >='%2' "
                  "AND Buchungen.Datum <='%3'");
    where = where.arg(QString::number(cid), from.toString(Qt::ISODate), to.toString(Qt::ISODate));

    return bookingsFromSql(where, qsl("Datum DESC"));
    //QVector<QSqlRecord> records = executeSql(booking::getTableDef().Fields(), where, qsl("Datum DESC"));
    //QVector<booking> vRet;
    //for( auto& rec: qAsConst(records)) {
    //    booking::Type t = booking::Type(rec.value(qsl("BuchungsArt")).toInt());
    //    QDate d = rec.value(qsl("Datum")).toDate();
    //    double amount = euroFromCt(rec.value(qsl("Betrag")).toInt());
    //    vRet.push_back(booking(t, d, amount));
    //}
    //return vRet;
}
/* static */ QVector<booking> bookings::getAnnualSettelments(const int year)
{   LOG_CALL;
    QString where = qsl("Buchungen.BuchungsArt = %1 AND Buchungen.Datum = '%2'");
    where = where.arg(QString::number(static_cast<int>(booking::Type::annualInterestDeposit)), QDate(year + 1, 1, 1).toString(Qt::ISODate));
    return bookingsFromSql(where);
}
