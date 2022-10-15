
#include "contract.h"
#include "dkdbviews.h"
#include "dbstructure.h"
#include "booking.h"

/* static */ const dbtable& booking::getTableDef()
{
    static dbtable bookingsTable(qsl("Buchungen"));
    if( 0 == bookingsTable.Fields().size()) {
        bookingsTable.append(dbfield(qsl("id"),          QVariant::LongLong).setAutoInc());
        bookingsTable.append(dbfield(qsl("VertragsId"),  QVariant::LongLong).setNotNull());
        bookingsTable.append(dbForeignKey(bookingsTable[qsl("VertragsId")], dkdbstructur[contract::tnContracts][contract::fnId], ODOU_Action::RESTRICT));
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
                               dkdbstructur[contract::tnExContracts][contract::fnId],ODOU_Action::RESTRICT));
    }
    return deletedBookings;
}
/* static */ QString bookingTypeDisplayString(const bookingType t)
{
    switch(t)
    {
    case bookingType::deposit :              return qsl("Einzahlung");
    case bookingType::payout :               return qsl("Auszahlung");
    case bookingType::reInvestInterest:      return qsl("Zinsanrechnung");
    case bookingType::annualInterestDeposit: return qsl("Jahreszins");
    case bookingType::setInterestActive:     return qsl("Aktivierung d. Zinszahlung");
    default:
        return qsl("FEHLER: ungültiger Buchungstyp");
    }
}

QString booking::toString( ) const
{
    return qsl("%1 zum Vertrag# %2: Betrag: %3, Datum: %4")
            .arg(bookingTypeDisplayString (type), i2s(contractId), d2euro(amount), date.toString(Qt::ISODate));
}

/////////////// BOOKING functions (friends, not family ;) )
///
bool writeBookingToDB( bookingType t, const qlonglong contractId, QDate date, const double amount)
{
    LOG_CALL_W (booking(contractId, t, date, amount).toString());
    TableDataInserter tdi(booking::getTableDef());
    tdi.setValue(qsl("VertragsId"), contractId);
    tdi.setValue(qsl("BuchungsArt"), static_cast<int>(t));
    tdi.setValue(qsl("Betrag"), ctFromEuro(amount));
    tdi.setValue(qsl("Datum"), date);
    //    "Zeitstempel" will be created by the sql default value =setDefaultNow()
    if( -1 not_eq tdi.InsertRecord()) {
        RETURN_OK( true, qsl("Buchung erfolgreich"));
    }  else
        RETURN_ERR( false, qsl("Buchung gescheitert"));
}
bool bookDeposit(const qlonglong contractId, QDate date, const double amount)
{
    Q_ASSERT( amount > 0.);
    return writeBookingToDB( bookingType::deposit, contractId, date, amount);
}
bool bookPayout(const qlonglong contractId, QDate date, const double amount)
{
    // contract has to check that a payout is possible
    return writeBookingToDB(bookingType::payout, contractId, date, -1*qFabs(amount));
}
bool bookReInvestInterest(const qlonglong contractId, QDate date, const double amount)
{
    Q_ASSERT( amount >= 0.);
    return writeBookingToDB(bookingType::reInvestInterest, contractId, date, amount);
}
bool bookAnnualInterestDeposit(const qlonglong contractId, QDate date, const double amount)
{
    Q_ASSERT( amount >= 0.);
    return writeBookingToDB(bookingType::annualInterestDeposit, contractId, date, amount);
}
bool bookInterestActive(const qlonglong contractId, QDate date)
{
    return writeBookingToDB(bookingType::setInterestActive, contractId, date, 0.);
}

///////////// bookingS start here

/* static */ QDate bookings::dateOfnextSettlement()
{
    /*
     * Man sollte eine Jahresendbuchung auch mehrmals machen können, für den Fall, dass es nachträglich
     * gebuchte Geldeingänge für Neuverträge (=Aktivierungen) gab
    */
    QDate ret =executeSingleValueSql(qsl("SELECT date FROM (%1)").arg(sqlNextAnnualSettlement)).toDate();
    qInfo() << "DateOfnextSettlement: Date of next Settlement was found as " << ret;
    return ret;
}
/*static */ QVector<booking> bookings::bookingsFromSql(const QString& where, const QString& order, bool terminated)
{
    qInfo() << "Where: "<< where << "\n" << "Order: " << order << "terminated: " << (terminated ? "true" : "false");
    QVector<QSqlRecord> records = terminated ?
               executeSql( booking::getTableDef_deletedBookings ().Fields (), where, order)
             : executeSql( booking::getTableDef().Fields(), where, order);

    QVector<booking> vRet;
    for (auto& rec : qAsConst(records)) {
        qlonglong cid = rec.value(qsl("VertragsId")).toLongLong();
        bookingType t = bookingType(rec.value(qsl("BuchungsArt")).toInt());
        QDate d = rec.value(qsl("Datum")).toDate();
        double amount = euroFromCt(rec.value(qsl("Betrag")).toInt());
        qInfo() << "bookingsFromSql: Buchung: cid=" << cid << "; type=" << bookingTypeDisplayString(t) << "; Datum=" << d.toString() << "; Betrag=" << amount;
        vRet.push_back(booking(cid, t, d, amount));
    }
    return vRet;
}
/* static */ QVector<booking> bookings::getBookings(const qlonglong cid, QDate from, const QDate to,
                    QString order, bool terminated)
{
    QString tablename = terminated ? qsl("ExBuchungen") : qsl("Buchungen");
    // used in tests
    QString where = qsl("%4.VertragsId=%1 "
                  "AND %4.Datum >='%2' "
                  "AND %4.Datum <='%3'");
    where = where.arg(QString::number(cid), from.toString(Qt::ISODate), to.toString(Qt::ISODate), tablename);

    return bookingsFromSql(where, order, terminated);
}
/* static */ QVector<booking> bookings::getAnnualSettelments(const int year)
{
    QString where = qsl("Buchungen.BuchungsArt = %1 AND Buchungen.Datum = '%2'");
    where = where.arg(QString::number(static_cast<int>(bookingType::annualInterestDeposit)), QDate(year, 12, 31).toString(Qt::ISODate));
    return bookingsFromSql(where);
}
/*static */ QVector<int> bookings::yearsWithAnnualBookings()
{   LOG_CALL;
    QVector<int> years;
    QString sql{qsl("SELECT DISTINCT SUBSTR(Datum, 1, 4) AS year FROM Buchungen WHERE BuchungsArt =%1 ORDER BY year DESC")
                .arg (bookingTypeToString(bookingType::annualInterestDeposit))};
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
