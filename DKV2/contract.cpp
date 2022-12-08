
#include "helper.h"
#include "helpersql.h"
#include "helperfin.h"
#include "dbstructure.h"
#include "dkdbhelper.h"
#include "contract.h"
#include "booking.h"

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
/*static*/ const QString contract::fnZeitstempel{qsl("Zeitstempel")};

/*static*/ const dbtable& contract::getTableDef()
{
    static dbtable contractTable(tnContracts);
    if (contractTable.Fields().isEmpty ())
    {
        contractTable.append(dbfield(fnId,           QVariant::LongLong).setAutoInc());
        contractTable.append(dbfield(fnKreditorId,   QVariant::LongLong).setNotNull());
        contractTable.append(dbForeignKey(contractTable[fnKreditorId],
            dkdbstructur[qsl("Kreditoren")][fnId], ODOU_Action::CASCADE));
        // deleting a creditor will delete inactive contracts but not
        // contracts with existing bookings (=active or terminated contracts)
        contractTable.append(dbfield(fnKennung,       QVariant::String).setUnique ());
        contractTable.append(dbfield(fnAnmerkung,     QVariant::String).setDefault(""));
        contractTable.append(dbfield(fnZSatz,         QVariant::Int).setNotNull().setDefault(0)); // 100-stel %; 100 entspricht 1%
        contractTable.append(dbfield(fnBetrag,        QVariant::Int).setNotNull().setDefault(0)); // ct
        contractTable.append(dbfield(fnThesaurierend, QVariant::Int).setNotNull().setDefault(1));
        contractTable.append(dbfield(fnVertragsDatum, QVariant::Date).setNotNull());
        contractTable.append(dbfield(fnKFrist,        QVariant::Int).setNotNull().setDefault(6));
        contractTable.append(dbfield(fnAnlagenId,     QVariant::LongLong));
        contractTable.append(dbForeignKey(contractTable[fnAnlagenId],
            qsl("Geldanlagen"), qsl("rowid"), ODOU_Action::SET_NULL));
        contractTable.append(dbfield(fnLaufzeitEnde,  QVariant::Date).setNotNull().setDefault(EndOfTheFuckingWorld_str));
        contractTable.append(dbfield(fnZAktiv,        QVariant::Bool).setDefault(true));
        contractTable.append(dbfield(fnZeitstempel,   QVariant::DateTime).setDefaultNow());
    }
    return contractTable;
}
/*static*/ const dbtable& contract::getTableDef_deletedContracts()
{
    Q_ASSERT(getTableDef().Fields().size());
    static dbtable exContractTable(tnExContracts);
    if (exContractTable.Fields().isEmpty()) {
        exContractTable.append(dbfield(fnId, QVariant::LongLong).setPrimaryKey());
        for(int i= 1 /* not 0 */; i < getTableDef().Fields().count(); i++) {
            exContractTable.append(getTableDef().Fields()[i]);
        }
        exContractTable.append(dbForeignKey(exContractTable[fnKreditorId],
                                            dkdbstructur[qsl("Kreditoren")][fnId], ODOU_Action::CASCADE));
    }
    return exContractTable;
}

// construction
contract::contract(qlonglong contractId /*=-1*/, bool isTerminated /*=false*/)
    : td(getTableDef())
{
    initContractDefaults();
    if( contractId < SQLITE_minimalRowId)
        return;
    if( isTerminated)
        loadExFromDb (contractId);
     else
        loadFromDb(contractId);
}

void contract::loadFromDb(qlonglong id)
{   LOG_CALL_W(i2s(id)); // id_aS() might not be initialized yet!!
    isTerminated = false;
    QSqlRecord rec = executeSingleRecordSql(getTableDef().Fields(), qsl("id=%1").arg(i2s(id)));
    if (not td.setValues(rec))
        qCritical() << "contract from id could not be created";
}
void contract::loadExFromDb(qlonglong id)
{   LOG_CALL_W(i2s(id));
    isTerminated =true;
    QSqlRecord rec = executeSingleRecordSql(getTableDef_deletedContracts ().Fields(), qsl("id=%1").arg(i2s(id)));
    if (not td.setValues(rec))
        qCritical() << "exContract from id could not be created";
}
void contract::initContractDefaults(const qlonglong CREDITORid /*=-1*/)
{
    setId(-1);
    setCreditorId(CREDITORid);
    setNoticePeriod(6);
    //setPlannedEndDate(EndOfTheFuckingWorld); - implicit
    setInterestModel(interestModel::reinvest);
    setConclusionDate(QDate::currentDate());
    setInterestRate(1.50);
    setPlannedInvest(1000000);
    setInvestment(0);
    setInterestActive(true);
    isTerminated =false;
}
void contract::initRandom(qlonglong creditorId)
{
    static QRandomGenerator *rand = QRandomGenerator::system();
    setLabel(proposeContractLabel());
    setCreditorId(creditorId);
    setInterestModel(interestModelFromInt(rand->bounded(100)%4));
    if( rand->bounded (1000)%15 == 0)
        setInterestRate (0.);
    else
        setInterestRate( rand->bounded(25)* 0.15);
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
    QString where {qsl("%1=%3 AND %2<='%4'").arg(fn_bVertragsId, fn_bDatum)};
    where =where.arg(id_aS (), d.toString(Qt::ISODate));
    QVariant v = executeSingleValueSql(qsl("SUM(Betrag)"), 
        isTerminated ? tn_ExBuchungen : tn_Buchungen, where);
    if( v.isValid())
        return euroFromCt(v.toInt());
    return 0.;
}
double contract::investedValue(const QDate d) const
{
    // how many money was put into the contract by the creditor?
    QString where{ qsl("%1=%5 AND %2<='%6' AND (%3=%7 OR %3=%8) ").arg(fn_bVertragsId, fn_bDatum, fn_bBuchungsArt) };
    where = where.arg(id_aS(), d.toString(Qt::ISODate),
        bookingTypeToNbrString(bookingType::deposit),
        bookingTypeToNbrString(bookingType::payout));
    QVariant v = executeSingleValueSql(qsl("SUM(Betrag)"), tn_Buchungen, where);
    if (v.isValid())
        return euroFromCt(v.toInt());
    return 0.;
}
double contract::interestBearingValue() const
{
    switch (iModel())
    {
    case interestModel::payout:
        return value();
    case interestModel::fixed:
        return investedValue();
    case interestModel::reinvest:
        return value();
    case interestModel::zero:
        return value();
    default:
        Q_ASSERT(false);
        return 0.;
    }
}
const booking contract::latestBooking()
{
    QString sql {qsl("SELECT id, %1, %2, %3, %4 FROM %6 WHERE %1=%5 ORDER BY rowid DESC LIMIT 1").
        arg(fn_bVertragsId, fn_bDatum, fn_bBuchungsArt, fn_bBetrag, id_aS(), isTerminated ? tn_ExBuchungen : tn_Buchungen)};
    QSqlRecord rec = executeSingleRecordSql(sql);
    if( 0 == rec.count()) {
        RETURN_OK( booking(), qsl("latestBooking returns empty value"));
    }
    booking latestB(id(), bookingType(rec.value(fn_bBuchungsArt).toInt()), rec.value(fn_bDatum).toDate(), euroFromCt(rec.value(fn_bBetrag).toInt()));
    RETURN_OK (latestB, qsl("Latest Booking:"), bookingTypeDisplayString(latestB.type), latestB.date.toString (Qt::ISODate),
                         d2euro(latestB.amount), qsl("cId:"), i2s(latestB.contractId));
}

// write to db
tableindex_t contract::saveNewContract()
{   LOG_CALL;
    tableindex_t lastid =td.InsertRecord();
    if( lastid >= 0) {
        setId(lastid);
        RETURN_OK( lastid, qsl("Neuer Vertrag wurde eingefügt mit id:"), i2s(lastid));
    }
    RETURN_ERR( SQLITE_invalidRowId, qsl("Fehler beim Einfügen eines neuen Vertrags"));
}
bool contract::updateComment(const QString &c)
{
    RETURN_OK( td.updateValue(fnAnmerkung, c, id()), qsl("Contract Comment updated"));
}
bool contract::updateTerminationDate(QDate termination, int noticePeriod)
{
    autoRollbackTransaction art;
    bool res =td.updateValue(fnKFrist, noticePeriod, id());
    res &= td.updateValue(fnLaufzeitEnde, termination, id());
    if( res) {
        art.commit();
        RETURN_OK( res, qsl("Successfully updated termination information"));
    } else {
        RETURN_ERR(res, qsl("Sailed to update termination information"));
    }
}
bool contract::updateInvestment(qlonglong investmentId)
{
    RETURN_OK( td.updateValue(fnAnlagenId, investmentId, id()), qsl("Updated the contract investment ref"), i2s(investmentId));
}
bool contract::updateSetInterestActive()
{
    // for now we only support activation but not deactivation
    RETURN_OK( td.updateValue(fnZAktiv, true, id()), qsl("activated interest payment"));
}
bool contract::deleteInactive() {
    QString sql=qsl("DELETE FROM Vertraege WHERE id=%1").arg( id_aS ());
    return executeSql_wNoRecords(sql);
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
bool contract::bookInitialPayment(const QDate date, double amount)
{   LOG_CALL;
    Q_ASSERT(id() >= 0);
    QString error;

    // QDate actualBookingDate =avoidYearEndBookings(date);
    // INITIAL payments may be on YearEnd, others not (because this is the date for
    // annual settlements
    QDate actualBookingDate {date};
    if (initialBookingReceived()) {
        error = qsl("Already active contract can not be activated");
    } else if ( not actualBookingDate.isValid()) {
        error = qsl("Invalid Date");
    } else if( amount < 0) {
        error =qsl("Invalid amount");
    }
    if ( error.size())
        RETURN_ERR( false, error);

    if ( bookDeposit(id(), actualBookingDate, amount))
            RETURN_OK( true, qsl("Successfully activated contract "), id_aS(), qsl("["), actualBookingDate.toString (Qt::ISODate), d2euro(amount));

    RETURN_ERR( false, qsl("Failed to execut activation on contract "), id_aS(), qsl(" ["), actualBookingDate.toString() , d2euro(amount), qsl("]"));
    return false;
}
bool contract::initialBookingReceived() const
{
    // there should be one deposits in the bookings list
    // there might be "interest activation" bookings?
    QString where= qsl("%1=%2 AND %3=%4")
            .arg( fn_bVertragsId, id_aS (), fn_bBuchungsArt, bookingTypeToNbrString(bookingType::deposit));
    return (0 < rowCount (tn_Buchungen, where));
}
bool contract::bookActivateInterest(const QDate d)
{
    autoRollbackTransaction art;
    if ( latestBooking().date >= d)
        RETURN_ERR( false, qsl("could not activate interest on same data or after as last booking"));
    if( not bookInBetweenInterest(d))
        RETURN_ERR( false, qsl("could not book inbetween interest on "));
    if( updateSetInterestActive() // update contract
            &&
        bookInterestActive(id(), d)) // insert booking
    {
        art.commit();
        RETURN_OK( true, qsl("successfully activated interest payment for contract "), id_aS());
    }
    RETURN_ERR( false, qsl("failed to activate interest payment for contract "), id_aS());
}

// booking actions
QDate contract::nextDateForAnnualSettlement()
{
    const booking lastB=latestBooking();

    if( lastB.type == bookingType::annualInterestDeposit) {
        // the last booking was a annual one - next one should be after 1 year
        Q_ASSERT(lastB.date.month() == 12);
        Q_ASSERT(lastB.date.day() == 31);
        return QDate(lastB.date.year() +1, 12, 31);
    }
    if( lastB.type == bookingType::deposit
            && getBookings (id()).count () == 1
            && lastB.date.month() == 12
            && lastB.date.day () == 31) {
        {
            qInfo() << "contract activation at years end -> no annualSettlement this year";
            return QDate(lastB.date.year () +1, 12, 31);
        }
    }

    if(lastB.type == bookingType::payout) {
        // in early versions payouts of annual interests could be after the annualInterestDeposits
        if((lastB.date.month() == 12 and lastB.date.day() == 31))
            return QDate(lastB.date.addYears(1));
        if((lastB.date.month() == 1 and lastB.date.day() == 1))
            return QDate(lastB.date.addDays(-1).addYears(1));
    }
    // for deposits, payouts, activations we return year end of the same year
    return QDate(lastB.date.year(), 12, 31);
}
bool contract::needsAnnualSettlement(const QDate intendedNextBooking)
{
    const booking lastB =latestBooking();
    if( lastB.type == bookingType::non) // inactive contract
        RETURN_OK( false, __FUNCTION__, qsl("inactive contract: no annual settlement needed"));

    if(  lastB.date >= intendedNextBooking)
        RETURN_OK( false, __FUNCTION__, qsl("Latest booking date too young for this settlement "),
                   lastB.date.toString (Qt::ISODate));
    // annualInvestments are at the last day of the year
    // we need a settlement if the latest booking was in the last year
    // or it was the settlement of the last year
    if( lastB.date.year() == intendedNextBooking.year())
        RETURN_OK( false, __FUNCTION__, qsl("intended Next booking is not in the same year as next booking "),
                   lastB.date.toString (Qt::ISODate), intendedNextBooking.toString (Qt::ISODate));

    if( lastB.date.addDays(1).year() == intendedNextBooking.year())
        RETURN_OK( false, __FUNCTION__, qsl("intended Next booking is on jan 1st of next year after latest booking "),
                   lastB.date.toString (Qt::ISODate), intendedNextBooking.toString (Qt::ISODate));

    RETURN_OK( true, __FUNCTION__, qsl("OK"));
}
int contract::annualSettlement( int year)
{   LOG_CALL_W(i2s(year));
    // es werden so lange Jahresabrechnungen durchgeführt, bis das Jahr "year" abgerechent ist
    Q_ASSERT(year);

    booking lastB =latestBooking ();
    // no initial booking?
    if( lastB.type == bookingType::non) return 0;

    executeSql_wNoRecords(qsl("SAVEPOINT as_savepoint"));
    QDate requestedSettlementDate(year, 12, 31);
    QDate nextAnnualSettlementDate =nextDateForAnnualSettlement();

    bool bookingSuccess =false;
    while(nextAnnualSettlementDate <= requestedSettlementDate) {
        double baseValue =interestBearingValue();
          ////////// von letzter Buchung bis zum 31.12. des selben Jahres
        double zins =ZinsesZins(actualInterestRate(),
                                baseValue, lastB.date, nextAnnualSettlementDate);
        //////////
        switch(iModel()) {
        case interestModel::reinvest:
        case interestModel::fixed:
        case interestModel::zero: {
            bookingSuccess =bookAnnualInterestDeposit(id(), nextAnnualSettlementDate, zins);
            break;
        }
        case interestModel::payout: {
            bookingSuccess = bookPayout(id(), nextAnnualSettlementDate, zins);
            bookingSuccess &=bookAnnualInterestDeposit(id(), nextAnnualSettlementDate, zins);
            break;
        }
        case interestModel::maxId:
        default: {
            Q_ASSERT(false);
        } }
        if( bookingSuccess) {
            qInfo() << "Successfull annual settlement: contract id " << id_aS() << ": " << nextAnnualSettlementDate << " Zins: " << zins;
            // latest booking has changes, so ...
            nextAnnualSettlementDate = nextDateForAnnualSettlement();
            lastB =latestBooking ();
            continue;
        } else {
            qCritical() << "Failed annual settlement: Vertrag " << id_aS() << ": " << nextAnnualSettlementDate << " Zins: " << zins;
            executeSql_wNoRecords(qsl("ROLLBACK"));
            return 0;
        }
    }
    executeSql_wNoRecords(qsl("RELEASE SAVEPOINT as_savepoint"));
    if( bookingSuccess)
        // there was a booking
        return year;
    else
        return 0;
}

// booking actions
bool contract::deposit(const QDate d, double amount, bool payoutInterest)
{   LOG_CALL;
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
        qCritical() << error;
        return false;
    }
    QDate actualD =avoidYearEndBookings(d);
    // update interest calculation
    QSqlDatabase::database().transaction();
    if( not bookInBetweenInterest(actualD, payoutInterest)) {
        QSqlDatabase::database().rollback();
        return false;
    }
    if( not bookDeposit(id(), actualD, actualAmount)) {
        qCritical() << "booking deposit failed -> rollback";
        QSqlDatabase::database().rollback();
        return false;
    }
    QSqlDatabase::database().commit();
    return true;
}
bool contract::payout(const QDate d, double amount, bool payoutInterest)
{   LOG_CALL;
    double actualAmount = qFabs(amount);

    const QDate lbd =latestBooking ().date;
    QString error;
    if( actualAmount > value()) error = qsl("Payout impossible. The account has not enough coverage");
    else if( not d.isValid()) error =qsl("Invalid Date in payout");
    else if(  lbd >= d) error = qsl("Bookings have to be in a consecutive order. Last booking: ")
                + lbd.toString() + qsl(" this booking: ") + d.toString();
    if( error.size()) {
        qCritical() << error;
        return false;
    }
    QDate actualD =avoidYearEndBookings(d);
    // update interest calculation
    QSqlDatabase::database().transaction();
    if( not bookInBetweenInterest(actualD, payoutInterest)) {
        QSqlDatabase::database().rollback();
        return false;
    }
    if( not bookPayout(id(), actualD, actualAmount)) {
        QSqlDatabase::database().rollback();
        qCritical() << "booking of payout failed";
        return false;
    }
    QSqlDatabase::database().commit();
    return true;
}
bool contract::cancel(const QDate d)
{   LOG_CALL;
    if( not initialBookingReceived()) {
        qInfo() << "an inactive contract can not be canceled. It should be deleted.";
        return false;
    }
    QDate actualD =avoidYearEndBookings(d);
    QString sql =qsl("UPDATE Vertraege SET LaufzeitEnde=?, Kfrist=? WHERE id=?");
    QVector<QVariant> v {actualD.toString(Qt::ISODate), -1, id()};
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
    booking lastB =latestBooking();
    if( not finDate.isValid())
        RETURN_ERR( false, qsl("invalid date to finalize contract"));
    if( finDate < lastB.date)
        RETURN_ERR( false, qsl("finalize date before last booking"));
    if( id() == -1)
        RETURN_ERR( false, qsl("invalid contract id"));

    if( not initialBookingReceived())
        RETURN_ERR( false, qsl("could not finalize inactive contract"));

    qlonglong id_to_be_deleted = id();
    qInfo() << "Finalization startet at value " << interestBearingValue ();
    executeSql_wNoRecords(qsl("SAVEPOINT fin"));

    if( iModel() == interestModel::payout)
        // adjust interest model, as there could be no payouts during finalize
        setInterestModel( interestModel::reinvest);

    if( needsAnnualSettlement(finDate)){
        if( annualSettlement(finDate.year() -1)) {
            lastB =latestBooking ();
        } else {
            executeSql_wNoRecords(qsl("ROLLBACK"));
            return false;
        }
    }
    double preFinValue = interestBearingValue ();
    qInfo() << "After last annual settlement: interest bearing / value " << preFinValue << " / " << value();
    finInterest = ZinsesZins(actualInterestRate(), preFinValue, lastB.date, finDate);
    finPayout = value() +finInterest;
    if( simulate) {
        qInfo() << "sumulation will stop here";
        executeSql_wNoRecords(qsl("ROLLBACK"));
        loadFromDb(id());
        return  true;
    }
    bool allGood =false;
    do {
        if( not bookReInvestInterest(id(), finDate, finInterest)) break;
        if( not bookPayout(id(), finDate, finPayout)) break;
        if( value() not_eq 0.) break;
        if( not storeTerminationDate(finDate)) break;
        if (not archive())
            break;
        allGood = true;
    } while(false);

    if( allGood) {
        executeSql_wNoRecords(qsl("RELEASE fin"));
        reset();
        qInfo() << "successfully finalized and archived contract " << id_to_be_deleted;
        return true;
    } else {
        qCritical() << "contract finalizing failed";
        executeSql_wNoRecords(qsl("ROLLBACK"));
        return false;
    }
}

// private
bool contract::bookInBetweenInterest(const QDate nextBookingDate, bool payout)
{   LOG_CALL;
    // booking interest in case of deposits, payouts or finalization
    // performs annualSettlements if necesarry

    booking lastB =latestBooking ();
    QString error;
    if( lastB.type == bookingType::non) error =qsl("interest booking on inactive contract not possible");
    else if( not nextBookingDate.isValid())  error =qsl("Invalid Date for interest booking");
    else if( lastB.date > nextBookingDate) error =qsl("could not book interest because there are already more recent bookings");
    if( error.size()) {
        qCritical() << error;
        return false;
    }
    if( needsAnnualSettlement(nextBookingDate)) {
        qInfo() << "Perform annual settlement first - this is unusual";
        if(0 == annualSettlement(nextBookingDate.year() -1)) {
            qCritical() << "annual settlement during interest booking failed";
            return false;
        } else {
            // update lastB, because it was changed by the annual Settlement
            lastB= latestBooking ();
        }
    }
                   //////////
    double zins = ZinsesZins(actualInterestRate(), interestBearingValue(), lastB.date, nextBookingDate);
                   //////////
    // only annualSettlements can be payed out
    if( payout)
        bookPayout (id(), nextBookingDate, zins);
    return bookReInvestInterest(id(), nextBookingDate, zins);
}
bool contract::storeTerminationDate(const QDate d) const
{   LOG_CALL;
    QVector<QVariant> v {d, id()};
    return executeSql_wNoRecords(qsl("UPDATE Vertraege SET LaufzeitEnde=? WHERE id=?"), v);
}
bool contract::archive()
{   LOG_CALL;
    // no check isActive() cause this is only called from finalize which does the check already
    // no check value()==0 cause this is done in finalize already
    // secured by the transaction of finalize()

    // move all bookings and the contract to the archive tables
    qlonglong ContractToDelete =id();
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
    if( id() <=0)
        stream << qsl("[contract was not saved or loaded from DB]") << qsl("\n");
    else
        stream << qsl("[id, cred.Id:") << id_aS() << qsl(", ") << i2s(creditorId()) << qsl("]") << qsl("\n");
    if( not initialBookingReceived()) {
        stream << "Wert (gepl.):     " << plannedInvest() << qsl("\n");
        stream << "Zinssatz (gepl.): " << interestRate() << qsl("\n");
        return ret;
    }
    stream << "Wert:     " << value() << qsl("\n");
    stream << "Zinssatz: " << interestRate() << qsl("\n");
    stream << "Buchungen:" << getNbrOfBookings (id()) << qsl("\n");
    return ret;
}
QVariantMap contract::toVariantMap(QDate fromDate, QDate toDate)
{   LOG_CALL;
    QVariantMap v;
    booking latestB = latestBooking();
    if (fromDate == BeginingOfTime)
        fromDate = td.getValue(fnVertragsDatum).toDate();
    if (toDate == EndOfTheFuckingWorld)
        toDate = latestB.date;
    v["id"] = id();
    v["strId"] = id_aS();
    v["KreditorId"] = i2s(creditorId());
    v["VertragsNr"] = label();
    double d = value(fromDate);
    v["dStartBetrag"] = d;
    v["startBetrag"] = d2euro(d);
    v["startDatum"] = fromDate.toString(qsl("dd.MM.yyyy"));
    d = value(toDate);
    v["dEndBetrag"] = d;
    v["endBetrag"] = d2euro(d);
    v["endDatum"] = toDate.toString(qsl("dd.MM.yyyy"));
    v["Vertragsdatum"] = td.getValue(fnVertragsDatum).toDate().toString(qsl("dd.MM.yyyy"));
    v["Vertragsende"] = hasEndDate() ? td.getValue(fnLaufzeitEnde).toDate().toString(qsl("dd.MM.yyyy")) : "offen";
    v["ZSatz"] = interestRate();
    v["strZSatz"] = prozent2prozent_str (interestRate());
    v["zzAusgesezt"] = QVariant(not interestActive());
    v["Anmerkung"] = comment();
    v["Betrag"] = euroFromCt(td.getValue(fnBetrag).toInt());
    v["strBetrag"] = d2euro(euroFromCt(td.getValue(fnBetrag).toInt()));
    v["Zinsmodell"] = ::interestModelDisplayString(iModel());
    v["KFrist"] = hasEndDate() ? 0 : noticePeriod();
    v["Status"] = isTerminated ? "Beendet" : "Laufend";
    if (isTerminated) {
        v["Beendet"] = "Beendet";
    }
    // get the relevant bookings for period
    QVector<booking> bookVector = getBookings (id(), fromDate, toDate, qsl("Datum ASC"), isTerminated);
    v["dSonstigeZinsen"] = getBookingsSum(bookVector, bookingType::reInvestInterest);
    v["dJahresZinsen"] = getBookingsSum(bookVector, bookingType::annualInterestDeposit);
    v["dAuszahlung"] = 0.;
    if (v["dSonstigeZinsen"] != 0.)
        v["SonstigeZinsen"] = d2euro(v["dSonstigeZinsen"].toDouble());

    if (v["dJahresZinsen"] != 0.) {
        v["JahresZinsen"] = d2euro(v["dJahresZinsen"].toDouble());
        if (iModel() == interestModel::payout) {
            v["dAuszahlung"] = v["dJahresZinsen"];
            v["Auszahlung"] = v["JahresZinsen"];
        }
    }

    QVariantList bl;

    for (const auto &b : qAsConst(bookVector))
    {
        QVariantMap bookMap = {};
        bookMap["Date"] = b.date.toString(qsl("dd.MM.yyyy"));
        bookMap["Text"] = bookingTypeDisplayString(b.type);
        bookMap["Betrag"] = d2euro(b.amount);

        bl.append(bookMap);
    }

    if (bl.size() > 0) {
        v["Buchungen"] = bl;
    }

    return v;
}

// helper for letters
double contract::payedInterestAtTermination()
{
    if( not isTerminated) return 0.;
    QString sql(qsl("SELECT Betrag FROM exBuchungen WHERE VertragsId=%1 AND BuchungsArt=%2 ORDER BY id DESC LIMIT 1"));
    sql =sql.arg(id_aS (), bookingTypeToNbrString(bookingType::reInvestInterest));
    return euroFromCt(executeSingleValueSql(sql).toInt());
}
double contract::getAnnualInterest(int year, bookingType interestType)
{
    if( iModel() not_eq interestModel::payout)
        return 0;

    QString where{qsl("VertragsId=%1 AND BuchungsArt=%2 AND SUBSTR(Buchungen.Datum, 1, 4)=%3")};
    where =where.arg(DbInsertableString (id()), DbInsertableString (bookingTypeToNbrString(interestType)),
                     DbInsertableString (i2s(year))); // conversion to string is needed as this is not an integer but part of a date string

    return euroFromCt(executeSingleValueSql (qsl("SUM(Betrag)"), tn_Buchungen, where).toInt());
}

// NON MEMBER FUNCTIONS

// test helper
// Vergleichsoperatoren für TESTS !!
bool operator==(const contract& lhs, const contract& rhs)
{   // friend functions - even in the class definition - are not member
    bool ret =true;
    if( lhs.td.getRecord().count() not_eq rhs.td.getRecord().count()) {
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
           (lhs.td.getValue(fname).type () == rhs.td.getValue(fname).type ()))
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

contract saveRandomContract(qlonglong creditorId)
{   LOG_CALL;
    contract c;
    c.initRandom(creditorId);
    c.saveNewContract();
    return c;
}

void saveRandomContractPerCreditor()
{
    QVector<QVariant> creditorIds = executeSingleColumnSql(dkdbstructur[qsl("Kreditoren")][contract::fnId]);
    for( const QVariant& creditorId: creditorIds) {
        saveRandomContract (creditorId.toLongLong ());
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
        saveRandomContract(creditorIds[rand->bounded(creditorIds.size())].toLongLong());
}
// test helper ?!
int activateAllContracts(int year)
{
    QVector<dbfield> idField {contract::getTableDef().Fields()[0]};
    QVector<QSqlRecord> ids= executeSql(idField);
    int res =0;
    for( const auto& id : ids) {
        contract c(id.value (0).toLongLong ());
        if( not c.interestActive ())
            c.bookActivateInterest (QDate(year, 1, 1).addYears (-1));
        c.bookInitialPayment (QDate(year, 1, 1).addYears (-1).addDays (10), c.plannedInvest ());
        if(year == c.annualSettlement (year))  res++;
    }
    return res;
}

// test helper ^

QDate activateRandomContracts(const int percent)
{   LOG_CALL;
    QDate minimumActivationDate =EndOfTheFuckingWorld; // needed for tests
    if( percent < 0 or percent > 100) return minimumActivationDate;

    QVector<QSqlRecord> contractData = executeSql(contract::getTableDef().Fields());
// todo: read only the ids!
// all data will be availabel in the loop from the contract object!!
    int activations = contractData.count() * percent / 100;
    static QRandomGenerator* rand = QRandomGenerator::system();

    for (int i=0; i < activations; i++) {
        // contractData -> from database all amounts are in ct
        double amount = euroFromCt(contractData[i].value(contract::fnBetrag).toInt());
        if( rand->bounded(100)%10 == 0) {
            // some contracts get activated with a different amount
            amount = amount * rand->bounded(90, 110) / 100;
        }
        QDate activationDate(contractData[i].value(contract::fnVertragsDatum).toDate());
        activationDate = activationDate.addDays(rand->bounded(100));
        if( activationDate < minimumActivationDate)
            minimumActivationDate =activationDate;
        contract c(contractData[i].value(contract::fnId).toInt());
        c.bookInitialPayment(activationDate, amount);
    }
    return minimumActivationDate;
}
