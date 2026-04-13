
#include "contract.h"
#include "dbfield.h"
#include "helper_core.h"
#include "helpersql.h"
#include "helperfin.h"
#include "dbstructure.h"
#include "dkdbhelper.h"

// statics & friends
/*static*/ const QString contract::tnContracts{qsl("Vertraege")};
/*static*/ const QString contract::tnExContracts{qsl("exVertraege")};
/*static*/ const QString contract::fnId {qsl("id")};
/*static*/ const QString contract::fnKreditorId{qsl("KreditorId")};
/*static*/ const QString contract::fnKennung{qsl("Kennung")};
/*static*/ const QString contract::fnAnmerkung{qsl("Anmerkung")};
/*static*/ const QString contract::fnZSatz{qsl("ZSatz")};
/*static*/ const QString contract::fnBetrag{qsl("Betrag")};
/*static*/ const QString contract::fnThesaurierend{qsl("thesaurierend")};
/*static*/ const QString contract::fnVertragsDatum{qsl("Vertragsdatum")};
/*static*/ const QString contract::fnKFrist{qsl("Kfrist")};
/*static*/ const QString contract::fnAnlagenId{qsl("AnlagenId")};
/*static*/ const QString contract::fnLaufzeitEnde{qsl("LaufzeitEnde")};
/*static*/ const QString contract::fnZAktiv{qsl("zActive")};
/*static*/ const QString contract::fnDateCanceled(qsl("KueDatum"));
/*static*/ const QString contract::fnZeitstempel{qsl("Zeitstempel")};

/*static*/ const dbtable& contract::getTableDef()
{
    static dbtable contractTable(tnContracts);
    if (contractTable.Fields().isEmpty ())
    {
        contractTable.append(dbfield(fnId,           QMetaType::LongLong).setAutoInc());
        contractTable.append(dbfield(fnKreditorId,   QMetaType::LongLong).setNotNull());
        contractTable.append(dbForeignKey(contractTable[fnKreditorId],
            dkdbstructur[qsl("Kreditoren")][fnId], ODOU_Action::CASCADE));
        // deleting a creditor will delete inactive contracts but not
        // contracts with existing bookings (=active or terminated contracts)
        contractTable.append(dbfield(fnKennung,       QMetaType::QString).setUnique ());
        contractTable.append(dbfield(fnAnmerkung,     QMetaType::QString).setDefault(""));
        contractTable.append(dbfield(fnZSatz,         QMetaType::Int).setNotNull().setDefault(0)); // 100-stel %; 100 entspricht 1%
        contractTable.append(dbfield(fnBetrag,        QMetaType::Int).setNotNull().setDefault(0)); // ct
        contractTable.append(dbfield(fnThesaurierend, QMetaType::Int).setNotNull().setDefault(1));
        contractTable.append(dbfield(fnVertragsDatum, QMetaType::QDate).setNotNull());
        contractTable.append(dbfield(fnKFrist,        QMetaType::Int).setNotNull().setDefault(6));
        contractTable.append(dbfield(fnAnlagenId,     QMetaType::LongLong));
        contractTable.append(dbForeignKey(contractTable[fnAnlagenId],
            qsl("Geldanlagen"), qsl("rowid"), ODOU_Action::SET_NULL));
        contractTable.append(dbfield(fnLaufzeitEnde,  QMetaType::QDate).setNotNull().setDefault(EndOfTheFuckingWorld_str));
        contractTable.append(dbfield(fnZAktiv,        QMetaType::Bool).setDefault(true));
        contractTable.append(dbfield(fnDateCanceled,  QMetaType::QDate).setNotNull().setDefault(EndOfTheFuckingWorld_str));
        contractTable.append(dbfield(fnZeitstempel,   QMetaType::QDateTime).setDefaultNow());
    }
    return contractTable;
}
/*static*/ const dbtable& contract::getTableDef_deletedContracts()
{
    Q_ASSERT(getTableDef().Fields().size());
    static dbtable exContractTable(tnExContracts);
    if (exContractTable.Fields().isEmpty()) {
        exContractTable.append(dbfield(fnId, QMetaType::LongLong).setPrimaryKey());
        for(int i= 1 /* not 0 */; i < getTableDef().Fields().count(); i++) {
            exContractTable.append(getTableDef().Fields()[i]);
        }
        exContractTable.append(dbForeignKey(exContractTable[fnKreditorId],
                                            dkdbstructur[qsl("Kreditoren")][fnId], ODOU_Action::CASCADE));
    }
    return exContractTable;
}

// construction
contract::contract(contractId_t cId /*=-1*/, bool isTerminated /*=false*/)
    : td(getTableDef())
{
    initContractDefaults();
    if( cId.v < Minimal_contract_id.v)
        return;
    if( isTerminated)
        loadExFromDb( cId);
     else
        loadFromDb( cId);
}

contract::contract(const QSqlRecord &record) : td(getTableDef())
{   LOG_CALL;
    isTerminated =false;
    if (not td.setValues(record))
        qCritical() << "contract could not be created from record";
}

void contract::loadFromDb( contractId_t id)
{   LOG_CALL_W(i2s(id.v)); // id_aS() might not be initialized yet!!
    isTerminated =false;
    QSqlRecord rec = executeSingleRecordSql(getTableDef().Fields(), qsl("id=%1").arg(id.v));
    if (not td.setValues(rec))
        qCritical() << "contract from id could not be created";
}

void contract::loadExFromDb(const contractId_t id)
{   LOG_CALL_W(i2s(id.v));
    isTerminated =true;
    QSqlRecord rec = executeSingleRecordSql(getTableDef_deletedContracts ().Fields(), qsl("id=%1").arg(id.v));
    if (not td.setValues(rec))
        qCritical() << "exContract from id could not be created";
}

void contract::initContractDefaults(const creditorId_t credId )
{
    setId(SQLITE_invalidRowId);
    setCreditorId(credId);
    setNoticePeriod(6);
    //setPlannedEndDate(EndOfTheFuckingWorld); - implicit
    setInterestModel(interestModel::reinvest);
    setConclusionDate(QDate::currentDate());
    setInterestRate(1.50);
    setPlannedInvest(1000000);
    setInvestmentId(SQLITE_invalidRowId);
    setInterestActive(true);
    isTerminated =false;
    initCancelationDate();
}

void contract::initRandom(const creditorId_t credId)
{
    static QRandomGenerator *rand = QRandomGenerator::system();
    setLabel(proposeContractLabel());
    setCreditorId(credId);
    setInterestRate( rand->bounded(25)* 0.15);

    setInterestModel(interestModelFromInt(rand->bounded(100)%4));

    setPlannedInvest(    rand->bounded(50)  *1000.
                       + rand->bounded(1,3) *500.
                       + rand->bounded(10)  *100.);
    setConclusionDate(QDate::currentDate().addYears(-2).addDays(rand->bounded(720)));
    if( rand->bounded(100)%5 > 0)
        // in 4 von 5 Fällen
        setNoticePeriod(3 + rand->bounded(21));
    else
        setPlannedEndDate(conclusionDate().addYears(rand->bounded(3)).addMonths(rand->bounded(12)));

    if(rand->bounded(100)%5 == 0)
        setInterestActive(false);
}

// interface
double contract::value(const QDate d) const
{
    // what is the value of the contract at a given time?
    const QString sql = qsl("SELECT SUM(Betrag) FROM %1 WHERE %2=? AND %3<=?")
        .arg(isTerminated ? booking::tn_ExBuchungen : booking::tn_Buchungen,
             booking::fn_bVertragsId,
             booking::fn_bDatum);
    const QVariant v = executeSingleValueSql(sql, QVector<QVariant>{id().v, d.toString(Qt::ISODate)});
    if( v.isValid())
        return euroFromCt(v.toInt());
    // todo: error handling
    return 0.;
}
double contract::investedValue(const QDate d) const
{
    // how many money was put into the contract by the creditor?
    const QString sql = qsl(
        "SELECT SUM(Betrag) FROM %1 "
        "WHERE %2=? AND %3<=? AND (%4=? OR %4=?)")
        .arg(booking::tn_Buchungen,
             booking::fn_bVertragsId,
             booking::fn_bDatum,
             booking::fn_bBuchungsArt);
    const QVariant v = executeSingleValueSql(sql, QVector<QVariant>{
        id().v,
        d.toString(Qt::ISODate),
        int(bookingType::deposit),
        int(bookingType::payout)
    });
    if (v.isValid())
        return euroFromCt(v.toInt());
    return 0.;
}
double contract::interestBearingValue() const
{
    switch (iModel())
    {
    case interestModel::fixed:
        return investedValue();
    case interestModel::payout:
    case interestModel::reinvest:
    case interestModel::zero:
        return value();
    default:
        qCritical() << "invalid interest Model";
        Q_UNREACHABLE();
    }
}
const QDate contract::latestBookingDate()
{
/*
NOTE to self: DKV2 stellt sicher, dass bei Verträgen mit verzögerter Zinszahlung
    eine Aktivierung der Zinszahlung erst nach dem ersten Geldeingang verbuchtwerden kann.
    SONST müsste hier sichergestellt werden, dass die letzte Buchung vom Typ Ein/Auszahlung oder Zinszahlung ist
*/
    const QString sql = qsl("SELECT MAX(%1) FROM %2 WHERE %3=?")
        .arg(booking::fn_bDatum,
             isTerminated ? booking::tn_ExBuchungen : booking::tn_Buchungen,
             booking::fn_bVertragsId);
    const QVariant date = executeSingleValueSql(sql, QVector<QVariant>{id().v});
    if( date.isValid() && date.canConvert<QDate>()) {
        QDate d {date.toDate()};
        if( d.isValid()){
            qInfo() << qsl("latest Booking Date found to be %1").arg(d.toString());
            return d;
        }
    }
    // this is a regular result, if there are no bookings.
    // BUT using "initialPaymentReceived" is the prefrered method to find out, if there are bookings
    qInfo() << "no lastestBookingDate as the contract has no bookings";
    return QDate();
}
const booking contract::latestBooking() const
{
    /*
NOTE to self:
    DKV2 stellt sicher, dass bei Verträgen mit verzögerter Zinszahlung
    eine Aktivierung der Zinszahlung erst nach dem ersten Geldeingang verbuchtwerden kann.
    SONST müsste hier sichergestellt werden, dass die letzte Buchung vom Typ Ein/Auszahlung oder Zinszahlung ist
    */
    const QString sql = qsl(
        "SELECT id, %1, %2, %3, %4 FROM %5 "
        "WHERE %1=? ORDER BY %2 DESC, id DESC LIMIT 1")
        .arg(booking::fn_bVertragsId,
             booking::fn_bDatum,
             booking::fn_bBuchungsArt,
             booking::fn_bBetrag,
             isTerminated ? booking::tn_ExBuchungen : booking::tn_Buchungen);
    QVector<QSqlRecord> records;
    executeSql(sql, QVector<QVariant>{id().v}, records);
    const QSqlRecord rec = (records.size() == 1) ? records[0] : QSqlRecord();
    if( 0 == rec.count()) {
        RETURN_OK( booking(), qsl("latestBooking returns empty value"));
    }
    booking latestB(id(), bookingType(rec.value(booking::fn_bBuchungsArt).toInt()), rec.value(booking::fn_bDatum).toDate(), euroFromCt(rec.value(booking::fn_bBetrag).toInt()));
    RETURN_OK (latestB, qsl("Latest Booking:"), qsl("Typ: "), bookingTypeDisplayString(latestB.type), qsl("/"), latestB.date.toString (Qt::ISODate), qsl("/"),
                         s_d2euro(latestB.amount), qsl("/"), qsl("cId:"), i2s(latestB.contId.v));
}

// write to db
contractId_t contract::saveNewContract()
{
    tableindex_t lastid =td.InsertRecord();
    if( lastid >= Minimal_contract_id.v) {
        setId(lastid);
        if( not advanceContractLabelIndex(label())) {
            qWarning() << "failed to advance contract label index after insert";
        }
        RETURN_OK( contractId_t{lastid}, qsl("contract::saveNewContract: Neuer Vertrag wurde eingefügt mit id: %1").arg(i2s(lastid)));
    }
    RETURN_ERR( Invalid_contract_id, qsl("Fehler beim Einfügen eines neuen Vertrags"));
}
// bool contract::updateDB_Contract()
bool contract::updateLabel( const QString& newLabel)
{   LOG_CALL;
    if( isValidNewContractLabel(newLabel)) {
        return td.updateValue (fnKennung, newLabel, id_l());
    }
    return false;
}
bool contract::updateConclusionDate( const QDate& newD) {
    return td.updateValue(fnVertragsDatum, QVariant(newD), id().v);
}
bool contract::updateInterestActive( const bool active) {
    RETURN_OK( td.updateValue(fnZAktiv, active ? 1 : 0, id().v), qsl("contract zActive updated"));
}
bool contract::updateComment(const QString &c)
{
    RETURN_OK( td.updateValue(fnAnmerkung, c, id().v), qsl("Contract Comment updated"));
}
bool contract::updateInitialPaymentDate(const QDate& newD)
{
// TODO: ?? kontrolliert das UI, dass das ipd VOR jeder anderen Buchung ist?
    const QString sql {qsl(
        "UPDATE Buchungen SET Datum = ? "
        "WHERE id == ("
        "SELECT MIN(rowid) FROM Buchungen WHERE VertragsId = ? AND BuchungsArt = 1)")};
    return executeSql_wNoRecords(sql, {newD.toString(Qt::ISODate), id().v});
}
bool contract::updateTerminationDate(QDate termination, int noticePeriod)
{
    autoRollbackTransaction art;
    bool res =td.updateValue(fnKFrist, noticePeriod, id().v);
    res &= td.updateValue(fnLaufzeitEnde, termination, id().v);
    if( res) {
        if( not art.commit()) {
            qCritical() << "updateTerminationDate: failed to commit transaction";
            RETURN_ERR(res, qsl("Failed to act on transaction"));
        }
        RETURN_OK( res, qsl("Successfully updated termination information"));
    } else {
        RETURN_ERR(res, qsl("Failed to update termination information"));
    }
}
bool contract::updateInvestment(tableindex_t investmentId)
{
    RETURN_OK( td.updateValue(fnAnlagenId, investmentId, id().v), qsl("Updated the contract investment ref"), i2s(investmentId));
}
bool contract::updateSetInterestActive()
{
    // for now we only support activation but not deactivation
    RETURN_OK( td.updateValue(fnZAktiv, true, id().v), qsl("activated interest payment"));
}
bool contract::deleteInactive() {
    return executeSql_wNoRecords(qsl("DELETE FROM Vertraege WHERE id=?"), id().v);
}
// non member helper: only annual settlements or initial payments should be on the last day of the year
// other bookings should move to dec. 30th
QDate avoidYearEndBookings(const QDate d)
{   LOG_CALL;
    if( not d.isValid())
        return d;
    if( d.month() == 12 and d.day() == 31) {
        qInfo() << "no deposit possible on dec. 31st -> switching to 30th";
        return d.addDays(-1);
    }
    return d;
}

// contract activation
// First payment may or may not start interest payment
bool contract::bookInitialPayment(const QDate date, const double amount)
{   LOG_CALL;
    Q_ASSERT(id().v >= 0);
    QString error;

    // QDate actualBookingDate =avoidYearEndBookings(date);
    // INITIAL payments may be on YearEnd, others not (because this is the date for
    // annual settlements ONLY

    if (initialPaymentReceived()) {
        error = qsl("Already active contract can not be activated");
    } else if ( not date.isValid()) {
        error = qsl("Invalid Date");
    } else if (date < conclusionDate()) {
        error = qsl("activate only after conclusion date: %1 vs. %2")
                    .arg(date.toString(), conclusionDate().toString());
    } else if (amount < 0) {
        error =qsl("Invalid amount");
    }
    if ( error.size())
        RETURN_ERR( false, error);

    if ( bookDeposit(id(), date, amount))
            RETURN_OK( true, qsl("Successfully activated contract %1 [%2, %3]").arg( id_aS(), date.toString (Qt::ISODate), s_d2euro(amount)));

    RETURN_ERR( false, qsl("Failed to execut activation on contract "), id_aS(), qsl(" ["), date.toString() , s_d2euro(amount), qsl("]"));
    return false;
}
bool contract::initialPaymentReceived() const
{
    // "true" wenn es Einzahlungen in den Vertrag gibt
    // DKV2 stellt sicher, dass Zinsaktivierung erst nach der Ersteinzahlung gemacht werden kann
    QString where= qsl("%1=%2 AND %3=%4")
            .arg( booking::fn_bVertragsId, id_aS (), booking::fn_bBuchungsArt, bookingTypeToNbrString(bookingType::deposit));
    return (0 < rowCount (booking::tn_Buchungen, where));
}
QDate contract::initialPaymentDate()
{
    const QVariant ipd = executeSingleValueSql(
        qsl("SELECT MIN(Datum) FROM Buchungen WHERE %1 = ? AND %2 = ?")
            .arg(booking::fn_bVertragsId, booking::fn_bBuchungsArt),
        QVector<QVariant>{id().v, int(bookingType::deposit)});
    return ipd.toDate ();
}
bool contract::noBookingButInitial()
{
    // an initial booking is always a deposit
    const QString sql = qsl("SELECT count(*) FROM %1 WHERE %2 = ? AND %3 = ?")
        .arg(booking::tn_Buchungen, booking::fn_bVertragsId, booking::fn_bBuchungsArt);
    return 1 == executeSingleValueSql(sql, QVector<QVariant>{id().v, int(bookingType::deposit)}).toInt();
}

BookingResult contract::bookActivateInterest(const QDate d)
{
    if( not initialPaymentReceived())
        return BookingResult::fail({qsl("Der Vertrag kann erst nach dem Geldeingang aktiviert werden")});
    if (d < latestBookingDate())
        return BookingResult::fail({qsl("Die Aktivierung der Zinsanrechnung muss nach der letzten Buchung erfolgen")});
    if( interestActive())
        return BookingResult::fail({qsl("Der Vertrag ist bereits aktiviert")});

    autoRollbackTransaction art;
    // durch diese Zinsbuchung mit Wert 0 wird sichergestellt, dass die nächste Zinsbuchung zum richtigen Zeitpunkt beginnt
    // kein Payout
    if( not bookInterestUntilDate(d))
        return BookingResult::fail({qsl("Die Zinsbuchung für den Zwischenzeitraum konnte nicht erstellt werden")});
    if( updateSetInterestActive() // update contract
            &&
        bookInterestActive(id(), d)) // insert booking
    {
        art.commit();
        qInfo() << qsl("successfully activated interest payment for contract %1").arg( id_aS());
        return BookingResult::success();
    }
    return BookingResult::fail(qsl("Die Aktivierung der Verzinsung für Vertrag %1 ist fehlgeschlagen").arg(id_aS()));
}

inline bool isLastDayOfTheYear(const QDate& d)
{ return (d.month() == 12 && d.day() == 31);}

// booking actions
QDate contract::dateOf_next_AS()
{
    const QDate lastBookingDate=latestBookingDate();

    if( not lastBookingDate.isValid())
        // no booking date -> no initial deposit -> no AS for now...
        return EndOfTheFuckingWorld;

    if( isLastDayOfTheYear (lastBookingDate))
        // last Booking was
        // - an annual settlement
        // - an initial deposit
        // OR an intrest activation w interest booking -> next settlement in one year
        return lastBookingDate.addYears(1);
    else
        // for deposits, payouts next settlement should be end of the same year
        return QDate(lastBookingDate.year(), 12, 31);
}

bool contract::needsAS_before(const QDate intendedNextBooking)
{    
    if (not intendedNextBooking.isValid()) return false;
    if (not latestBookingDate().isValid()) return false;
    const QDate nextAS {dateOf_next_AS()};
    return  nextAS < intendedNextBooking;
}

year contract::annualSettlement( year y)
{   LOG_CALL_W(i2s(y));
    // es werden so lange Jahresabrechnungen durchgeführt, bis das Jahr "year" abgerechent ist
    Q_ASSERT(y);
    qInfo() << "\nJahresabrechnung für Vertrag " << label();

    // no initial booking?
    if( not initialPaymentReceived())
        RETURN_OK(0, qsl("Keine Einzahlung -> keine Jahresabrechnung!"));

    ///// AS SAVEPOINT STARTS HERE
    SavepointGuard sp{qsl("annualSettlement")};

    const QDate requestedSettlementDate(y, 12, 31);
    QDate nextAS {dateOf_next_AS()};

    if( nextAS == EndOfTheFuckingWorld) {
        RETURN_ERR(0, qsl("contract has no booking -> no settlement"));
    }

    QDate lastBD {latestBookingDate()};
    bool didAnything =false;

    while(nextAS <= requestedSettlementDate)
    {
        double baseValue =interestBearingValue();
        ////////// von letzter Buchung bis zum 31.12. des selben Jahres
        double zins =ZinsesZins(actualInterestRate(),
                                baseValue, lastBD, nextAS);
        //////////
        bool ok {false};
        switch(iModel())
        {
        case interestModel::reinvest:
        case interestModel::fixed:
        case interestModel::zero: {
            ok =bookAnnualInterestDeposit(id(), nextAS, zins);
            break;
        }
        case interestModel::payout: {
            ok = bookPayout(id(), nextAS, zins)
                 && bookAnnualInterestDeposit(id(), nextAS, zins);
            break;
        }
        case interestModel::maxId:
        default: {
            qCritical() << "invalid interest Model";
            Q_UNREACHABLE();
        } }

        if( not ok) {
            qCritical() << "Failed annual settlement: Vertrag " << id_aS() << ": " << nextAS << " Zins: " << zins;
            return 0; // savepoint rollback initiated
        }
        qInfo() << "Successfull annual settlement: contract id " << id_aS() << ": " << nextAS << " Zins: " << zins;
        didAnything =true;
        // update latest booking data cause it has changed
        nextAS = dateOf_next_AS();
        lastBD =latestBookingDate();
    }
    if( not didAnything)
    {
        qInfo() << "there was no annual settlement, so we do a rollback";
        return 0;
    }
    //////////
    sp.commit();
    //////////
    return lastBD.year();//nextAnnualSettlementDate.year();
}

// booking actions
bool contract::deposit(const QDate d, double amount, bool payoutInterest, midYearInterestMode midYearInterest)
{   LOG_CALL_W(QString::number(amount));
    double actualAmount = qFabs(amount);
    const booking lastBooking =latestBooking ();
    QString error;
    if( lastBooking.type == bookingType::non)
        error = qsl("could not put money on an inactive account");
    else if ( not d.isValid())
        error = qsl("Year End is a Invalid Date") + d.toString();
    else if ( lastBooking.date >= d)
        error = qsl("bookings have to be in a consecutive order. Last booking: ")
                + lastBooking.date.toString() + qsl(" this booking: ") + d.toString();
    if( error.size()) {
        qCritical().noquote() << error;
        return false;
    }
    QDate actualD =avoidYearEndBookings(d);
    return bookValueChange(actualD, actualAmount, payoutInterest, bookingType::deposit, midYearInterest);
}

bool contract::payout(const QDate d, double amount, bool payoutInterest, midYearInterestMode midYearInterest)
{   LOG_CALL;
    double actualAmount = qFabs(amount);

    const QDate lbd =latestBookingDate();
    QString error;
    if( not d.isValid())
        error =qsl("Invalid Date in payout");
    else if(  lbd >= d)
            error = qsl("Bookings have to be in a consecutive order. Last booking: ")
                + lbd.toString() + qsl(" this booking: ") + d.toString();
    else if( actualAmount > value())
        error = qsl("Payout impossible. The account has not enough coverage");

    if( error.size()) {
        qCritical() << error;
        return false;
    }
    QDate actualD =avoidYearEndBookings(d);
    return bookValueChange(actualD, actualAmount, payoutInterest, bookingType::payout, midYearInterest);
}

contract::midYearInterestMode contract::yearlyMidYearInterestMode(int year)
{
    if (year < 0 || !isValidRowId(id().v))
        return contract::undecided;

    const QString tableName = isTerminated ? booking::tn_ExBuchungen : booking::tn_Buchungen;
    const QString sql = qsl(
        "SELECT "
        "MAX(CASE WHEN BuchungsArt = ? THEN 1 ELSE 0 END) AS deferredMode, "
        "MAX(CASE WHEN BuchungsArt = ? THEN 1 ELSE 0 END) AS immediateMode "
        "FROM %1 "
        "WHERE VertragsId = ? AND substr(Datum, 1, 4) = ?")
        .arg(tableName);

    QVector<QSqlRecord> records;
    if (not executeSql(sql,
                       QVector<QVariant>{int(bookingType::deferredMidYearInterest),
                                         int(bookingType::reInvestInterest),
                                         id().v,
                                         i2s(year)},
                       records) || records.size() != 1) {
        qCritical() << "could not determine mid year interest mode";
        return contract::undecided;
    }

    const QSqlRecord rec = records[0];
    const bool isDeferred = rec.value(qsl("deferredMode")).toInt() > 0;
    const bool isImmediate = rec.value(qsl("immediateMode")).toInt() > 0;

    if (isDeferred && isImmediate) {
        qCritical() << "inconsistent mid-year interest mode markers for contract/year"
                    << id().v << year << "- explicit deferred marker wins";
        return contract::deferred;
    }
    if (isDeferred)
        return contract::deferred;
    if (isImmediate)
        return contract::immediate;
    return contract::undecided;
}

bool contract::cancel(const QDate dPlannedContractEnd, const QDate dCancelation)
{   LOG_CALL;
    if( not initialPaymentReceived()) {
        qInfo() << "an inactive contract can not be canceled. It should be deleted.";
        return false;
    }
    QDate actualD =avoidYearEndBookings(dPlannedContractEnd);
    QString sql =qsl("UPDATE Vertraege SET LaufzeitEnde=?, Kfrist=?, KueDatum=? WHERE id=?");
    QVector<QVariant> v {actualD.toString(Qt::ISODate), -1, dCancelation.toString(Qt::ISODate), id().v};
    if( not executeSql_wNoRecords(sql, v)) {
        qCritical() << "faild to cancel contract ";
        return false;
    }
    setPlannedEndDate(actualD);
    return true;
}
bool contract::finalize(bool simulate, const QDate finDate,
                        double& finInterest, double& finPayout)
{   LOG_CALL;

    QDate lastBookingDate { latestBookingDate()};
    if( not finDate.isValid())
        RETURN_ERR( false, qsl("invalid date to finalize contract"));
    if( finDate < lastBookingDate)
        RETURN_ERR( false, qsl("finalize date before last booking"));
    if( not isValidRowId(id().v))
        RETURN_ERR( false, qsl("invalid contract id"));
    if( not initialPaymentReceived())
        RETURN_ERR( false, qsl("could not finalize inactive contract"));

    const qlonglong id_to_be_deleted { id().v};
    qInfo() << "Finalization startet at value " << interestBearingValue ();

    bool ok {false};
    bool needReload {false};

    { // database transactional savety starts here
    ///////////////
    SavepointGuard sp(qsl("fin"));
    ///////////////
    if( iModel() == interestModel::payout)
        // adjust interest model, as there could be no payouts during finalize
        setInterestModel( interestModel::reinvest);

    if( needsAS_before(finDate)
        and ( 0 == annualSettlement(finDate.year() -1))) {
       // AS failed somehow
        needReload =true;
    } else {
        lastBookingDate =latestBookingDate();
        const double preFinValue { interestBearingValue ()};
        qInfo() << "After last annual settlement: interest bearing / value " << preFinValue << " / " << value();

        finInterest = ZinsesZins(actualInterestRate(), preFinValue, lastBookingDate, finDate);
        finPayout = value() +finInterest;

        if( simulate) {
            qInfo() << "simulation will stop here";
            ok =true;
            needReload =true;
        } else {
            ok = bookReInvestInterest(id(), finDate, finInterest)
            && bookPayout(id(), finDate, finPayout)
            && (value() == 0.)
            && storeTerminationDate(finDate)
            && archive();
            if( ok) sp.commit();
            else {
                needReload =true;
                qCritical() << "annualSettlement failed during finalize";
            }
        }
    }
    }
    if( needReload)
        loadFromDb(id());

    if( not ok) {
        qCritical() << "contract finalize failed";
        return false;
    }

    if( not simulate) {
        reset();
        qInfo() << "successfully finalized and archived contract "
                << id_to_be_deleted;
    }
    return true;

}

// private
bool contract::ensureYearlyMidYearInterestMode(const QDate bookingDate, midYearInterestMode requestedMode)
{
    if( not interestActive())
        return true;

    const midYearInterestMode existingMode = yearlyMidYearInterestMode(bookingDate.year());
    if( existingMode != contract::undecided) {
        if( requestedMode != contract::undecided && requestedMode != existingMode) {
            qCritical() << "conflicting mid-year interest mode for contract/year"
                        << id().v << bookingDate.year();
            return false;
        }
        return true;
    }

    if( requestedMode == contract::deferred)
        return bookDeferredInBetweenInterest(id(), bookingDate);

    return true;
}

bool contract::bookValueChange(const QDate bookingDate, double amount, bool payoutInterest, bookingType bookingKind, midYearInterestMode midYearInterest)
{
    QSqlDatabase::database().transaction();

    if( not ensureYearlyMidYearInterestMode(bookingDate, midYearInterest)) {
        QSqlDatabase::database().rollback();
        return false;
    }

    if( not bookInterestBeforeValueChange(bookingDate, payoutInterest)) {
        QSqlDatabase::database().rollback();
        return false;
    }

    bool ok = false;
    switch(bookingKind)
    {
    case bookingType::deposit:
        ok = bookDeposit(id(), bookingDate, amount);
        if( not ok)
            qCritical() << "booking deposit failed -> rollback";
        break;
    case bookingType::payout:
        ok = bookPayout(id(), bookingDate, amount);
        if( not ok)
            qCritical() << "booking of payout failed";
        break;
    default:
        qCritical() << "invalid booking type for contract::bookValueChange";
        break;
    }

    if( not ok) {
        QSqlDatabase::database().rollback();
        return false;
    }

    QSqlDatabase::database().commit();
    return true;
}
bool contract::bookInterestUntilDate(const QDate date, bool payout)
{
    return bookInterestToDateImpl(date, payout, interestBookingMode::mustBookNow);
}

bool contract::bookInterestBeforeValueChange(const QDate date, bool payout)
{
    return bookInterestToDateImpl(date, payout, interestBookingMode::mayDeferToYearEnd);
}

bool contract::bookInterestToDateImpl(const QDate nextBookingDate, bool payout, interestBookingMode mode)
{   LOG_CALL;
    // books interest up to a given date and performs annualSettlements if necessary.
    // deposits/payouts may defer this to year end; other business actions must not.

    QDate latestBDate =latestBookingDate();
    QString error;
    if( not latestBDate.isValid())
        error =qsl("interest booking on inactive contract not possible");
    else if( not nextBookingDate.isValid())
        error =qsl("Invalid Date for interest booking");
    else if( latestBDate > nextBookingDate)
        error =qsl("could not book interest because there are already more recent bookings");
    if( error.size()) {
        qCritical() << error;
        return false;
    }
    if( needsAS_before(nextBookingDate)) {
        qInfo() << "Perform annual settlement first - this is unusual";
        if(0 == annualSettlement(nextBookingDate.year() -1)) {
            qCritical() << "annual settlement during interest booking failed";
            return false;
        } else {
            // update lastB, because it was changed by the annual Settlement
            latestBDate= latestBookingDate();
        }
    }

    if( mode == interestBookingMode::mayDeferToYearEnd
            && yearlyMidYearInterestMode(nextBookingDate.year()) == contract::deferred) {
        qInfo() << "mid-year interest booking deferred to year end";
        return true;
    }
                   //////////
    double zins = ZinsesZins(actualInterestRate(), interestBearingValue(), latestBDate, nextBookingDate);
                   //////////
    // only annualSettlements can be payed out
    if( payout)
        bookPayout (id(), nextBookingDate, zins);
    return bookReInvestInterest(id(), nextBookingDate, zins);
}
bool contract::storeTerminationDate(const QDate d) const
{   LOG_CALL;
    QVector<QVariant> v {d, id().v};
    return executeSql_wNoRecords(qsl("UPDATE Vertraege SET LaufzeitEnde=? WHERE id=?"), v);
}
bool contract::archive()
{   LOG_CALL;
    // no check isActive() cause this is only called from finalize which does the check already
    // no check value()==0 cause this is done in finalize already
    // secured by the transaction of finalize()

    // move all bookings and the contract to the archive tables
    qlonglong ContractToDelete =id().v;
    if( executeSql_wNoRecords(qsl("INSERT INTO exVertraege SELECT * FROM Vertraege WHERE id=?"), ContractToDelete))
     if( executeSql_wNoRecords(qsl("INSERT INTO exBuchungen SELECT * FROM Buchungen WHERE VertragsId=?"), ContractToDelete))
      if( executeSql_wNoRecords(qsl("DELETE FROM Buchungen WHERE VertragsId=?"), ContractToDelete))
       if( executeSql_wNoRecords(qsl("DELETE FROM Vertraege WHERE id=?"), ContractToDelete))
          {
             qInfo() << "contract was moved to the contract archive";
             return true;
          }
    qCritical() << "contract could not be moved to the archive";
    return false;
}

// other helper
QString contract::toString(const QString &title) const
{
    QString ret;
    QTextStream stream(&ret);
    stream << title << qsl("\n");
    if( id().v <=0)
        stream << qsl("[contract was not saved or loaded from DB]\n");
    else
        stream << qsl("[id, cred.Id:") << id_aS() << qsl(", ") << i2s(credId().v) << qsl("]\n");
    if( not initialPaymentReceived()) {
        stream << qsl("Wert (gepl.):     %1\n").arg(plannedInvest()) << qsl("\n");
        stream << qsl("Zinssatz (gepl.): %1\n").arg(interestRate());
        return ret;
    }
    stream << "Wert:     " << value() << qsl("\n");
    stream << "Zinssatz: " << interestRate() << qsl("\n");
    stream << "verz.Zz   " << interestActive() << qsl("\n");
    stream << "Buchungen:" << getNbrOfBookings (id()) << qsl("\n");
    return ret;
}
QVariantMap contract::toVariantMap(QDate fromDate, QDate toDate) const
{   LOG_CALL;
    QVariantMap v;
    booking latestB = latestBooking();
    if (fromDate == BeginingOfTime)
        fromDate = td.getValue(fnVertragsDatum).toDate();
    if (toDate == EndOfTheFuckingWorld)
        toDate = latestB.date;
    v["id"] = id().v;
    v["strId"] = id_aS();
    v["KreditorId"] = i2s(credId().v);
    v["VertragsNr"] = label();

    // "Start" should include last settlement before the period, but exclude bookings on fromDate.
    const QDate startValueDate = fromDate.addDays(-1);
    double d = value(startValueDate);
    v["dStartBetrag"] = d;
    v["startBetrag"] = s_d2euro(d);
    v["startDatum"] = fromDate.toString(qsl("dd.MM.yyyy"));
    v["showStartSaldo"] = td.getValue(fnVertragsDatum).toDate() < fromDate;

    d = value(toDate);
    v["dEndBetrag"] = d;
    v["endBetrag"] = s_d2euro(d);
    v["endDatum"] = toDate.toString(qsl("dd.MM.yyyy"));
    v["showEndSaldo"] = !isTerminated;

    v["Vertragsdatum"] = td.getValue(fnVertragsDatum).toDate().toString(qsl("dd.MM.yyyy"));
    v["Vertragsende"] = hasEndDate() ? td.getValue(fnLaufzeitEnde).toDate().toString(qsl("dd.MM.yyyy")) : "offen";
    v["ZSatz"] = interestRate();
    v["strZSatz"] = prozent2prozent_str (interestRate());
    v["zzAusgesezt"] = QVariant(not interestActive());
    v["Anmerkung"] = comment();
    v["Betrag"] = euroFromCt(td.getValue(fnBetrag).toInt());
    v["strBetrag"] = s_d2euro(euroFromCt(td.getValue(fnBetrag).toInt()));
    v["Zinsmodell"] = ::interestModelDisplayString(iModel());
    v["KFrist"] = hasEndDate() ? 0 : noticePeriod();
    v["Status"] = isTerminated ? "Beendet" : "Laufend";
    if (isTerminated) {
        v["Beendet"] = "Beendet";
    }

    QVector<booking> yearBookings = getBookings (id(), fromDate, toDate, qsl("Datum ASC"), isTerminated);
    v["dAuszahlung"] = 0.;
    v["dZinsgutschrift"] = 0.;
    v["dJahresZinsen"] = 0.;
    v["dSonstigeZinsen"] = 0.;

    /* special handling for contracts terminated in requested period to
       calculate the overall payed interest */
    if (isTerminated && plannedEndDate() >= fromDate && plannedEndDate() <= toDate)
    {
        QVector<booking> allBookings = getBookings(id(), BeginingOfTime, toDate, qsl("Datum ASC"), isTerminated);

        if (iModel() == interestModel::reinvest || iModel() == interestModel::fixed)
        {
      v["dSonstigeZinsen"] = getBookingsSum(allBookings, bookingType::reInvestInterest) +
                             getBookingsSum(allBookings, bookingType::annualInterestDeposit);
        }
        else
        {
      v["dSonstigeZinsen"] = getBookingsSum(yearBookings, bookingType::reInvestInterest) +
                             getBookingsSum(yearBookings, bookingType::annualInterestDeposit);
        }
    }
    else {
        // get the relevant bookings for period
        v["dJahresZinsen"] = getBookingsSum(yearBookings, bookingType::annualInterestDeposit) +
                                getBookingsSum(yearBookings, bookingType::reInvestInterest);
    }

    if (v["dSonstigeZinsen"] != 0.)
        v["SonstigeZinsen"] = s_d2euro(v["dSonstigeZinsen"].toDouble());

    if (v["dJahresZinsen"] != 0.) {
        v["JahresZinsen"] = s_d2euro(v["dJahresZinsen"].toDouble());
        if (iModel() == interestModel::payout) {
            v["dAuszahlung"] = v["dJahresZinsen"];
            v["Auszahlung"] = v["JahresZinsen"];
        }
        else {
            v["dZinsgutschrift"] = v["dJahresZinsen"];
            v["Zinsgutschrift"] = v["JahresZinsen"];
        }
    }

    // Only include booking list if there is more than just the AS booking.
    bool includeBookingList = !yearBookings.isEmpty();
    if (yearBookings.size() == 1 &&
        yearBookings[0].type == bookingType::annualInterestDeposit) {
        includeBookingList = false;
    }
    if (includeBookingList) {
        QVariantList bl;
        for (const auto &b : std::as_const(yearBookings))
        {
            QVariantMap bookMap = {};
            bookMap["Date"] = b.date.toString(qsl("dd.MM.yyyy"));
            QString bookingText = bookingTypeDisplayString(b.type);
            if (isTerminated && b.type == bookingType::payout) {
                bookingText = qsl("Finale Auszahlung");
            }
            bookMap["Text"] = bookingText;
            bookMap["Betrag"] = s_d2euro(b.amount);

            bl.append(bookMap);
        }
        if (!bl.isEmpty()) {
            v["Buchungen"] = bl;
        }
    }

    return v;
}

// helper for letters
double contract::payedInterestAtTermination()
{
    if( not isTerminated) return 0.;
    const QString sql = qsl(
        "SELECT Betrag FROM exBuchungen WHERE VertragsId=? AND BuchungsArt=? "
        "ORDER BY id DESC LIMIT 1");
    return euroFromCt(executeSingleValueSql(
        sql,
        QVector<QVariant>{id().v, int(bookingType::reInvestInterest)}).toInt());
}
double contract::getAnnualInterest(year y, bookingType interestType)
{
    if( iModel() != interestModel::payout)
        return 0;

    const QString sql = qsl(
        "SELECT SUM(Betrag) FROM %1 WHERE VertragsId=? AND BuchungsArt=? AND SUBSTR(%2, 1, 4)=?")
        .arg(booking::tn_Buchungen, booking::fn_bDatum);
    return euroFromCt(executeSingleValueSql(
        sql,
        QVector<QVariant>{id().v, int(interestType), i2s(y)}).toInt());
}

// test helper
// Vergleichsoperatoren für TESTS !!
bool operator==(const contract& lhs, const contract& rhs)
{   // friend functions - even in the class definition - are not member
    bool ret =true;
    if( lhs.td.getRecord().count() != rhs.td.getRecord().count()) {
        qInfo() << "contract comparison: field count mismatch " << lhs.td.getRecord().count() << " / " << rhs.td.getRecord().count();
        ret =false;
    }
    dbtable table =contract::getTableDef();
    for( int i =0; i < table.Fields().count(); i++){
        QString fname =table.Fields().value(i).name();
        if( fname == qsl("Zeitstempel"))
            continue;
        if( (lhs.td.getValue(fname) == rhs.td.getValue(fname))
                &&
           (lhs.td.getValue(fname).metaType () == rhs.td.getValue(fname).metaType ()))
            // QVariant comparison might convert QString to numbers
            continue;
        else {
            qInfo() << "contract field missmatch " << fname << ": " << lhs.td.getValue(fname) << " / " << rhs.td.getValue(fname);
            ret = false;
        }
    }
    return ret;
}
bool operator!=(const contract& lhs, const contract& rhs)
{
    return not (lhs==rhs);
}

void contract::initCancelationDate() {
    td.setValue (qsl("KueDatum"), EndOfTheFuckingWorld);
}

contract saveRandomContract(const creditorId_t credId)
{   LOG_CALL;
    contract c;
    c.initRandom(credId);
    c.saveNewContract();
    return c;
}

void saveRandomContractPerCreditor()
{
    QVector<QVariant> creditorIds = executeSingleColumnSql(dkdbstructur[qsl("Kreditoren")][contract::fnId]);
    for( const QVariant& credId: std::as_const(creditorIds)) {
        saveRandomContract ( creditorId_t{ credId.toLongLong()});
    }
}

void saveRandomContracts(int count)
{   LOG_CALL;
    Q_ASSERT(count>0);
    QVector<QVariant> creditorIds = executeSingleColumnSql(dkdbstructur[qsl("Kreditoren")][contract::fnId]);
    if( creditorIds.isEmpty ())
        qCritical() << "No Creditors to create contracts for";

    static QRandomGenerator* rand = QRandomGenerator::system();
    for (int i = 0; i<count; i++)
        saveRandomContract(creditorId_t{creditorIds[rand->bounded(creditorIds.size())].toLongLong()});
}
// test helper ?!
int activateAllContracts(year y)
{
    QVector<dbfield> idField {contract::getTableDef().Fields()[0]};
    QVector<QSqlRecord> ids= executeSql(idField);
    int res =0;
    for( const auto& id : std::as_const(ids)) {
        contract c(contractId_t{id.value (0).toLongLong ()});
        if( not c.interestActive ())
            c.bookActivateInterest (QDate(y, 1, 1).addYears (-1));
        c.bookInitialPayment (QDate(y, 1, 1).addYears (-1).addDays (10), c.plannedInvest ());
        if(y == c.annualSettlement (y))  res++;
    }
    return res;
}

// test helper ^

QDate activateRandomContracts(const int percent)
{   LOG_CALL;
    QDate minimumActivationDate =EndOfTheFuckingWorld; // needed for tests
    if( percent < 0 or percent > 100) {
        qCritical() << "invalid parameter for activateRandomContracts";
        return minimumActivationDate;
    }

    QVector<QVariant> contractData = executeSingleColumnSql (contract::getTableDef ().Fields ()[0]);

    qsizetype activations = contractData.count() * percent / 100l;
    static QRandomGenerator* rand = QRandomGenerator::system();

    for (int i=0; i < activations; i++) {
        contract c(contractId_t {contractData[i].toLongLong ()});

        double amount = c.plannedInvest ();
        if( rand->bounded(100)%10 == 0) {
            // some contracts get activated with a different amount
            amount = amount * rand->bounded(90, 110) / 100;
        }
        // Activation must be after conclusion date; avoid random 0-day offset.
        QDate activationDate(c.conclusionDate().addDays(rand->bounded(1, 51)));
        if( c.bookInitialPayment(activationDate, amount)) {
            if( activationDate < minimumActivationDate)
                minimumActivationDate =activationDate;
        } else {
            qInfo() << "failed contract activation on " << c.toString ();
        }
    }
    RETURN_OK( minimumActivationDate, minimumActivationDate.toString());
}
void contract::setCreditorId(const creditorId_t kid) {
    td.setValue(fnKreditorId, kid.v);
}
