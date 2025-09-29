#include "helper.h"
#include "helpersql.h"
#include "contract.h"
#include "dkdbviews.h"
#include "dbstructure.h"
#include "qnamespace.h"
#include "booking.h"

/* static */ const dbtable& booking::getTableDef()
{
    static dbtable bookingsTable(tn_Buchungen);
    if( 0 == bookingsTable.Fields().size()) {
        bookingsTable.append(dbfield(qsl("id"),       QMetaType::LongLong).setAutoInc());
        bookingsTable.append(dbfield(fn_bVertragsId,  QMetaType::LongLong).setNotNull());
        bookingsTable.append(dbForeignKey(bookingsTable[fn_bVertragsId],
                                          dkdbstructur[contract::tnContracts][contract::fnId], ODOU_Action::RESTRICT));
        bookingsTable.append(dbfield(fn_bDatum,       QMetaType::QDate).setDefault(EndOfTheFuckingWorld_str).setNotNull());
        bookingsTable.append(dbfield(fn_bBuchungsArt, QMetaType::Int).setNotNull()); // deposit, interestDeposit, outpayment, interestPayment
        bookingsTable.append(dbfield(fn_bBetrag,      QMetaType::Int).setNotNull()); // in cent
        bookingsTable.append(dbfield(fn_bModifiziert, QMetaType::QDate).setDefault(BeginingOfTime).setNotNull());
        bookingsTable.append(dbfield(qsl("Zeitstempel"),  QMetaType::QDateTime).setDefaultNow());
    }
    return bookingsTable;
}
/* static */ const dbtable& booking::getTableDef_deletedBookings()
{
    static dbtable deletedBookings(tn_ExBuchungen);
    if( 0 == deletedBookings.Fields().size()) {
        deletedBookings.append(dbfield(qsl("id"), QMetaType::LongLong).setPrimaryKey());
        for( int i =1 /* not 0 */; i < getTableDef().Fields().count(); i++) {
            deletedBookings.append(getTableDef().Fields()[i]);
        }
        deletedBookings.append(dbForeignKey(deletedBookings[fn_bVertragsId],
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
        Q_ASSERT(not "Ungültigen Buchungstyp");
        return qsl("FEHLER: ungültiger Buchungstyp");
    }
}

QString booking::toString( ) const
{
    return qsl("%1 zum Vertrag# %2: Betrag: %3, Datum: %4")
            .arg(bookingTypeDisplayString (type), i2s(contractId), s_d2euro(amount), date.toString(Qt::ISODate));
}

/////////////// BOOKING functions (friends, not family ;) )
///
bool bookingToDB(bookingType t, const tableindex_t contractId, QDate date, const double amount)
{   LOG_CALL_W (booking(contractId, t, date, amount).toString());
    if( not date.isValid ())
        RETURN_ERR(false, qsl(">> invalid booking date <<"));
    TableDataInserter tdi(booking::getTableDef());
    tdi.setValue(fn_bVertragsId,  contractId);
    tdi.setValue(fn_bBuchungsArt, static_cast<int>(t));
    tdi.setValue(fn_bBetrag,      ctFromEuro(amount));
    tdi.setValue(fn_bDatum,       date);
    //    "Zeitstempel" will be created by the sql default value =setDefaultNow()
    if( isValidRowId(tdi.InsertRecord()))
        RETURN_OK( true, qsl(">> Buchung erfolgreich <<"));
    else
        RETURN_ERR( false, qsl(">> Buchung gescheitert <<"));
}
bool bookDeposit(const tableindex_t contractId, QDate date, const double amount)
{
    if( amount <= 0)
        RETURN_ERR(false, qsl(">> Einzahlungen müssen einen Wert größer als 0 haben <<"));
    return bookingToDB( bookingType::deposit, contractId, date, amount);
}
bool bookPayout(const tableindex_t contractId, QDate date, const double amount)
{
    // contract has to check that a payout is possible
    return bookingToDB(bookingType::payout, contractId, date, -1*qFabs(amount));
}
bool bookReInvestInterest(const tableindex_t contractId, QDate date, const double amount)
{
    if( amount < 0)
        RETURN_ERR(false, qsl(">> booking ReInvestInterest failed due to negative amount <<"));
    return bookingToDB(bookingType::reInvestInterest, contractId, date, amount);
}
bool bookAnnualInterestDeposit(const tableindex_t contractId, QDate date, const double amount)
{
    if( amount < 0)
        RETURN_ERR(false, qsl(">> booking Annual Interest Deposit failed due to negative amount <<"));
    return bookingToDB(bookingType::annualInterestDeposit, contractId, date, amount);
}
bool bookInterestActive(const tableindex_t contractId, QDate date)
{
    return bookingToDB(bookingType::setInterestActive, contractId, date, 0.);
}

bool writeBookingUpdate( qlonglong bookingId, int newValeuInCt)
{
    QString sql {qsl("UPDATE %0 SET %1=%2, %3='%4' WHERE id=%5")
            .arg(tn_Buchungen, fn_bBetrag, QString::number(newValeuInCt),
                 fn_bModifiziert, QDate::currentDate().toString(Qt::ISODate),
                         QString::number(bookingId))};
    return executeSql_wNoRecords (sql);
}
///////////// bookingS start here

QDate dateOfnextSettlement()
{
    /*
     * Man sollte eine Jahresendbuchung auch mehrmals machen können, für den Fall, dass es nachträglich
     * gebuchte Geldeingänge für Neuverträge (=Aktivierungen) gab
    */
    QVariant ret =executeSingleValueSql(qsl("SELECT date FROM (%1)").arg(sqlNextAnnualSettlement));
    bool canC = ret.convert(QMetaType::QDate);
    QDate retDate = canC ? ret.toDate () : QDate();
    RETURN_OK(retDate, qsl("DateOfnextSettlement: Date of next Settlement was found as %1").arg(retDate.toString (Qt::ISODate)));
}
int getNbrOfBookings(const qlonglong contract, const QDate from, const QDate to, const bool terminated)
{
    QString where {qsl(" VertragsId=%1").arg(contract)};
    if( from > BeginingOfTime)
        where += qsl(" AND Datum >='%1'").arg(from.toString(Qt::ISODate));
    if( to < EndOfTheFuckingWorld)
        where += qsl(" AND Datum <='%1'").arg(to.toString(Qt::ISODate));
    return rowCount((terminated ? tn_ExBuchungen : tn_Buchungen), where);
}
int getNbrOfExBookings(const qlonglong contract, const QDate from, const QDate to)
{
    return getNbrOfBookings(contract, from, to, true);
}

QVector<booking> bookingsFromDB(const QString& where, const QString& order ="", bool terminated =false)
{
    qInfo().noquote () << QString(__FUNCTION__) << qsl(" Where: %1\n").arg( where) << qsl("Order: %1\n").arg( order) << qsl("terminated: %1").arg((terminated ? "true" : "false"));
    QVector<QSqlRecord> records = terminated ?
               executeSql( booking::getTableDef_deletedBookings ().Fields (), where, order)
             : executeSql( booking::getTableDef().Fields(), where, order);

    QVector<booking> vRet;
    for (auto& rec : std::as_const(records)) {
        booking b (rec.value(fn_bVertragsId).toLongLong(),
                   bookingType(rec.value(fn_bBuchungsArt).toInt()),
                   rec.value(fn_bDatum).toDate(),
                   euroFromCt(rec.value(fn_bBetrag).toInt()));
        qInfo() << "bookingFromDB: " << b.toString ();
        vRet.push_back(b);
    }
    return vRet;
}
QVector<booking> getBookings(const tableindex_t contractId, QDate from, const QDate to,
                    QString order, bool terminated)
{
    QString tablename = terminated ? tn_ExBuchungen : tn_Buchungen;
    // used in tests
    QString where = qsl("%9.%1=%6 "
                  "AND %9.%2 >='%7' "
                  "AND %9.%2 <='%8'").arg(fn_bVertragsId, fn_bDatum);
    where = where.arg(i2s(contractId), from.toString(Qt::ISODate), to.toString(Qt::ISODate), tablename);

    return bookingsFromDB(where, order, terminated);
}
QVector<booking> getExBookings(const qlonglong cid, QDate from, const QDate to,
                    QString order)
{
    return getBookings(cid, from, to, order, true);
}
double getBookingsSum(QVector<booking> bl, bookingType bt)
{
    double sum = 0.;
    for (const auto &b : std::as_const(bl)) {
        if (b.type == bt) {
            sum += b.amount;
        }
    }
    return sum;
}

QVector<booking> getAnnualSettlements(const int year)
{
    QString where = qsl("%1.%2 = %4 AND %1.%3 = '%5'").arg(tn_Buchungen, fn_bBuchungsArt, fn_bDatum);
    where = where.arg(bookingTypeToNbrString(bookingType::annualInterestDeposit), QDate(year, 12, 31).toString(Qt::ISODate));
    return bookingsFromDB(where);
}
QVector<int> yearsWithAnnualBookings()
{   LOG_CALL;
    QVector<int> years;
    QString sql{qsl("SELECT DISTINCT SUBSTR(Datum, 1, 4) AS year FROM Buchungen WHERE BuchungsArt =%1 ORDER BY year DESC")
                .arg (bookingTypeToNbrString(bookingType::annualInterestDeposit))};
    QVector<QSqlRecord> vYears;
    if( executeSql (sql, vYears)) {
        for (const QSqlRecord& year : std::as_const(vYears)) {
            years.push_back (year.value (0).toInt ());
        }
    } else {
        qCritical() << "error reading annual settlement years from db";
    }
    return years;
}
