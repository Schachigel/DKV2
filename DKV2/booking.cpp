#include <QtMath>

#include "dkdbviews.h"
#include "contract.h"
#include "booking.h"

/* static */ const dbtable& booking::getTableDef()
{
    static dbtable bookingsTable(qsl("Buchungen"));
    if( 0 == bookingsTable.Fields().size()) {
        bookingsTable.append(dbfield(qsl("id"),          QVariant::LongLong).setPrimaryKey().setAutoInc());
        bookingsTable.append(dbfield(qsl("VertragsId"),  QVariant::LongLong).setNotNull());
        bookingsTable.append(dbForeignKey(bookingsTable[qsl("VertragsId")], dkdbstructur[contract::tnContracts][contract::fnId], qsl("ON DELETE RESTRICT")));
        bookingsTable.append(dbfield(qsl("Datum"),       QVariant::Date).setDefault(EndOfTheFuckingWorld_str).setNotNull());
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
                               dkdbstructur[contract::tnExContracts][contract::fnId], qsl("ON DELETE RESTRICT")));
    }
    return deletedBookings;
}
/* static */ QString booking::displayString(const Type t)
{
    switch(t)
    {
    case booking::Type::deposit : return qsl("Einzahlung");
    case booking::Type::payout : return qsl("Auszahlung");
    case booking::Type::reInvestInterest: return qsl("Zinsanrechnung");
    case booking::Type::annualInterestDeposit: return qsl("Jahreszins");
    case booking::Type::setInterestActive: return qsl("Aktivierung d. Zinszahlung");
    default:
        QString error{qsl("FEHLER: ungültiger Buchungstyp")};
        Q_ASSERT(false);
        return error;
    }
}
/* static */ QString booking::typeName(booking::Type t)
{
    switch(t) {
    case booking::Type::non: return qsl("invalid booking");
    case booking::Type::deposit: return qsl("deposit");
    case booking::Type::payout: return qsl("payout");
    case booking::Type::reInvestInterest: return qsl("reinvest interest");
    case booking::Type::annualInterestDeposit: return qsl("annual interest");
    case booking::Type::setInterestActive: return qsl("interest activation");
    default: return qsl("ERROR");
    }
}

/* static */ bool booking::doBooking( booking::Type t, const qlonglong contractId, QDate date, const double amount)
{   LOG_CALL_W(typeName(t));
    TableDataInserter tdi(booking::getTableDef());
    tdi.setValue(qsl("VertragsId"), contractId);
    tdi.setValue(qsl("BuchungsArt"), static_cast<int>(t));
    tdi.setValue(qsl("Betrag"), ctFromEuro(amount));
    tdi.setValue(qsl("Datum"), date);
//    "Zeitstempel" will be created by default
    if( -1 not_eq tdi.WriteData()) {
        qInfo() << "successful booking: " << typeName(t) << " contract#: " << contractId << " Amount: " << ctFromEuro(amount) << " date: " << date;
        return true;
    }
    qCritical() << "booking failed for contract#: " << contractId << " Amount: " << ctFromEuro(amount) << " date: " << date;;
    return false;
}
/* static */ bool booking::bookDeposit(const qlonglong contractId, QDate date, const double amount)
{
    Q_ASSERT( amount > 0.);
    return doBooking( Type::deposit, contractId, date, amount);
}
/* static */ bool booking::bookPayout(const qlonglong contractId, QDate date, const double amount)
{
    // contract has to check that a payout is possible
    return doBooking(Type::payout, contractId, date, -1*qFabs(amount));
}
/* static */ bool booking::bookReInvestInterest(const qlonglong contractId, QDate date, const double amount)
{
    Q_ASSERT( amount >= 0.);
    return doBooking(Type::reInvestInterest, contractId, date, amount);
}
/* static */ bool booking::bookAnnualInterestDeposit(const qlonglong contractId, QDate date, const double amount)
{
    Q_ASSERT( amount >= 0.);
    return doBooking(Type::annualInterestDeposit, contractId, date, amount);
}

/* static */ bool booking::bookInterestActive(const qlonglong contractId, QDate date)
{
    return doBooking(Type::setInterestActive, contractId, date, 0.);
}

/* static */ QDate bookings::dateOfnextSettlement()
{   LOG_CALL;
    QDate ret =executeSingleValueSql(qsl("SELECT date FROM (%1)").arg(sqlNextAnnualSettlement)).toDate();
    qInfo() << "Date of next Settlement was found as " << ret;
    return ret;
}
/*static */ QVector<booking> bookings::bookingsFromSql(const QString& where, const QString& order, 
        const lifeStatus bookingStatus)
{
    QVector<QSqlRecord> records = executeSql(
        bookingStatus == lifeStatus::InUse ? 
            booking::getTableDef().Fields(): booking::getTableDef_deletedBookings().Fields(),
        where, order);

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

/* static */ QVector<booking> bookings::getBookings(const qlonglong cid, QDate from, const QDate to, 
                    QString order, lifeStatus bookingStatus)
{   LOG_CALL;
    // used in tests
    QString where = qsl("%4.VertragsId=%1 "
                  "AND %4.Datum >='%2' "
                  "AND %4.Datum <='%3'");
    where = where.arg(QString::number(cid), from.toString(Qt::ISODate), to.toString(Qt::ISODate),
            bookingStatus == lifeStatus::InUse ? "Buchungen" : "exBuchungen");

    return bookingsFromSql(where, order, bookingStatus);
}
/* static */ QVector<booking> bookings::getAnnualSettelments(const int year)
{   LOG_CALL;
    QString where = qsl("Buchungen.BuchungsArt = %1 AND Buchungen.Datum = '%2'");
    where = where.arg(QString::number(static_cast<int>(booking::Type::annualInterestDeposit)), QDate(year, 12, 31).toString(Qt::ISODate));
    return bookingsFromSql(where);
}

/*static */ QVector<int> bookings::yearsWithAnnualBookings()
{
    QVector<int> years;
    QString sql{qsl("SELECT DISTINCT SUBSTR(Datum, 1, 4) AS year FROM Buchungen WHERE BuchungsArt =%1 ORDER BY year DESC")
                .arg (booking::bookingTypeToInt(booking::Type::annualInterestDeposit))};
    QVector<QSqlRecord> vYears;
    if( executeSql (sql, QVariant(), vYears)) {
        for (const QSqlRecord& year : qAsConst(vYears)) {
            years.push_back (year.value (0).toInt ());
        }
    } else {
        qCritical() << "error reading annual settlement years from db";
    }
    return years;
}
