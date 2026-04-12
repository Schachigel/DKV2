#include "booking.h"
#include "helper_core.h"
#include "helpersql.h"
#include "contract.h"
#include "dbstructure.h"
#include "qnamespace.h"

/* static */ const QString booking::tn_Buchungen   {qsl("Buchungen")};
/* static */ const QString booking::tn_ExBuchungen {qsl("ExBuchungen")};
/* static */ const QString booking::fn_bVertragsId {qsl("VertragsId")};
/* static */ const QString booking::fn_bBuchungsArt{qsl("BuchungsArt")};
/* static */ const QString booking::fn_bBetrag     {qsl("Betrag")};
/* static */ const QString booking::fn_bDatum      {qsl("Datum")};
/* static */ const QString booking::fn_bModifiziert{qsl("Überschrieben")};

/* static */ const dbtable& booking::getTableDef()
{
    static dbtable bookingsTable(booking::tn_Buchungen);
    if( 0 == bookingsTable.Fields().size()) {
        bookingsTable.append(dbfield(qsl("id"),       QMetaType::LongLong).setAutoInc());
        bookingsTable.append(dbfield(booking::fn_bVertragsId,  QMetaType::LongLong).setNotNull());
        bookingsTable.append(dbForeignKey(bookingsTable[booking::fn_bVertragsId],
                                          dkdbstructur[contract::tnContracts][contract::fnId], ODOU_Action::RESTRICT));
        bookingsTable.append(dbfield(booking::fn_bDatum,       QMetaType::QDate).setDefault(EndOfTheFuckingWorld_str).setNotNull());
        bookingsTable.append(dbfield(booking::fn_bBuchungsArt, QMetaType::Int).setNotNull()); // deposit, interestDeposit, outpayment, interestPayment
        bookingsTable.append(dbfield(booking::fn_bBetrag,      QMetaType::Int).setNotNull()); // in cent
        bookingsTable.append(dbfield(booking::fn_bModifiziert, QMetaType::QDate).setDefault(BeginingOfTime).setNotNull());
        bookingsTable.append(dbfield(qsl("Zeitstempel"),  QMetaType::QDateTime).setDefaultNow());
    }
    return bookingsTable;
}
/* static */ const dbtable& booking::getTableDef_deletedBookings()
{
    static dbtable deletedBookings(booking::tn_ExBuchungen);
    if( 0 == deletedBookings.Fields().size()) {
        deletedBookings.append(dbfield(qsl("id"), QMetaType::LongLong).setPrimaryKey());
        for( int i =1 /* not 0 */; i < getTableDef().Fields().count(); i++) {
            deletedBookings.append(getTableDef().Fields()[i]);
        }
        deletedBookings.append(dbForeignKey(deletedBookings[booking::fn_bVertragsId],
                               dkdbstructur[contract::tnExContracts][contract::fnId],ODOU_Action::RESTRICT));
    }
    return deletedBookings;
}
/* static */ QString bookingTypeDisplayString(const bookingType t)
{
    switch(t)
    {
    case bookingType::non :                  return qsl("Alle Buchungstypen");
    case bookingType::deposit :              return qsl("Einzahlung");
    case bookingType::payout :               return qsl("Auszahlung");
    case bookingType::reInvestInterest:      return qsl("Zinsanrechnung");
    case bookingType::annualInterestDeposit: return qsl("Jahreszins");
    case bookingType::setInterestActive:     return qsl("Aktivierung d. Zinszahlung");
    case bookingType::deferredMidYearInterest:return qsl("Unterjähriger Zins nachgelagert");
    default:
        qCritical() << "Ungültiger Buchungstyp";
        return qsl("-- schwerer Fehler - ungültiger Buchungstyp --");
    }
}

QString booking::toString( ) const
{
    return qsl("%1 zum Vertrag# %2: Betrag: %3, Datum: %4")
            .arg(bookingTypeDisplayString (type), i2s(contId.v), s_d2euro(amount), date.toString(Qt::ISODate));
}

/////////////// BOOKING functions (friends, not family ;) )
///
bool writeBookingToDB(bookingType t, const contractId_t cId, QDate date, const double amount)
{   LOG_CALL_W (booking(cId, t, date, amount).toString());
    if( not date.isValid ())
        RETURN_ERR(false, qsl(">> invalid booking date <<"));
    TableDataInserter tdi(booking::getTableDef());
    tdi.setValue(booking::fn_bVertragsId,  cId.v);
    tdi.setValue(booking::fn_bBuchungsArt, static_cast<int>(t));
    tdi.setValue(booking::fn_bBetrag,      ctFromEuro(amount));
    tdi.setValue(booking::fn_bDatum,       date);
    //    "Zeitstempel" will be created by the sql default value =setDefaultNow()
    if( isValidRowId(tdi.InsertRecord()))
        RETURN_OK( true, qsl(">> Buchung erfolgreich <<"));
    else
        RETURN_ERR( false, qsl(">> Buchung gescheitert <<"));
}
bool bookDeposit(const contractId_t cId, QDate date, const double amount)
{
    if( amount <= 0)
        RETURN_ERR(false, qsl(">> Einzahlungen müssen einen Wert größer als 0 haben <<"));
    return writeBookingToDB( bookingType::deposit, cId, date, amount);
}
bool bookPayout(const contractId_t cId, QDate date, const double amount)
{
    // contract has to check that a payout is possible
    return writeBookingToDB(bookingType::payout, cId, date, -1*qFabs(amount));
}
bool bookReInvestInterest(const contractId_t cId, QDate date, const double amount)
{
    if( amount < 0)
        RETURN_ERR(false, qsl(">> booking ReInvestInterest failed due to negative amount <<"));
    return writeBookingToDB(bookingType::reInvestInterest, cId, date, amount);
}
bool bookAnnualInterestDeposit(const contractId_t cId, QDate date, const double amount)
{
    if( amount < 0)
        RETURN_ERR(false, qsl(">> booking Annual Interest Deposit failed due to negative amount <<"));
    return writeBookingToDB(bookingType::annualInterestDeposit, cId, date, amount);
}
bool bookInterestActive(const contractId_t cId, QDate date)
{
    return writeBookingToDB(bookingType::setInterestActive, cId, date, 0.);
}

bool writeBookingUpdate( bookingId_t bookingId, int newValeuInCt)
{
    const QString sql = qsl("UPDATE %1 SET %2=?, %3=? WHERE id=?")
            .arg(booking::tn_Buchungen, booking::fn_bBetrag, booking::fn_bModifiziert);
    return executeSql_wNoRecords(sql, QVector<QVariant>{newValeuInCt, QDate::currentDate(), bookingId.v});
}

///////////// bookingS start here
int getNbrOfBookings(const contractId_t contract, const QDate from, const QDate to, const bool terminated)
{   // for testing mainly
    const QString tableName = terminated ? booking::tn_ExBuchungen : booking::tn_Buchungen;
    QString sql = qsl("SELECT count(*) FROM %1 WHERE %2=?")
            .arg(tableName, booking::fn_bVertragsId);
    QVector<QVariant> params{contract.v};

    if( from > BeginingOfTime) {
        sql += qsl(" AND %1>=?").arg(booking::fn_bDatum);
        params.push_back(from.toString(Qt::ISODate));
    }
    if( to < EndOfTheFuckingWorld) {
        sql += qsl(" AND %1<=?").arg(booking::fn_bDatum);
        params.push_back(to.toString(Qt::ISODate));
    }
    return executeSingleValueSql(sql, params).toInt();
}
int getNbrOfExBookings(const contractId_t contract, const QDate from, const QDate to)
{
    return getNbrOfBookings(contract, from, to, true);
}
namespace {
QVector<booking> bookingsFromDB(const QString& sql, const QVector<QVariant>& params, bool terminated =false)
{
    qInfo().noquote() << QString(__FUNCTION__)
                      << qsl(" Sql: %1\n").arg(sql)
                      << qsl("terminated: %1").arg((terminated ? "true" : "false"));
    QVector<QSqlRecord> records;
    if (not executeSql(sql, params, records)) {
        qCritical() << "bookingsFromDB: query failed";
        return {};
    }

    QVector<booking> vRet;
    for (const auto& rec : std::as_const(records)) {
        const contractId_t cid{rec.value(booking::fn_bVertragsId).toLongLong()};
        const booking b(cid,
                  bookingType(rec.value(booking::fn_bBuchungsArt).toInt()),
                  rec.value(booking::fn_bDatum).toDate(),
                  euroFromCt(rec.value(booking::fn_bBetrag).toInt()));
        qInfo() << "bookingFromDB: " << b.toString();
        vRet.push_back(b);
    }
    return vRet;
}
} // namespace

// annual settlement letter creation
QVector<booking> getBookings(const contractId_t contractId, QDate inclFrom, const QDate inclTo,
                    QString order, bool terminated)
{
    QString tablename = terminated ? booking::tn_ExBuchungen : booking::tn_Buchungen;
    QString sql = qsl("SELECT * FROM %1 WHERE %2 >= ? AND %2 <= ?")
                      .arg(tablename, booking::fn_bDatum);
    QVector<QVariant> params {inclFrom.toString(Qt::ISODate), inclTo.toString(Qt::ISODate)};

    if (isValidRowId(contractId.v)) {
        sql += qsl(" AND %1 = ?").arg(booking::fn_bVertragsId);
        params.push_back(QVariant::fromValue<qlonglong>(contractId.v));
    }

    if (not order.isEmpty()) {
        sql += qsl(" ORDER BY %1").arg(order);
    }

    return bookingsFromDB(sql, params, terminated);
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

QVector<int> yearsWithAnnualBookings()
{   LOG_CALL;
    QVector<int> years;
    const QString sql{qsl("SELECT DISTINCT SUBSTR(Datum, 1, 4) AS year FROM Buchungen WHERE BuchungsArt = ? ORDER BY year DESC")};
    QVector<QSqlRecord> vYears;
    if( executeSql(sql, QVector<QVariant>{int(bookingType::annualInterestDeposit)}, vYears)) {
        for (const QSqlRecord& year : std::as_const(vYears)) {
            years.push_back (year.value (0).toInt ());
        }
    } else {
        qCritical() << "error reading annual settlement years from db";
    }
    return years;
}
