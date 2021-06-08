#include <QVector>
#include <QRandomGenerator>
#include <QTextStream>
#include <QtMath>

#include "helper.h"
#include "contract.h"
#include "booking.h"

// statics & friends
/*static*/ const dbtable& contract::getTableDef()
{
    static dbtable contractTable(qsl("Vertraege"));
    if (0 == contractTable.Fields().size())
    {
        contractTable.append(dbfield(qsl("id"), QVariant::LongLong).setPrimaryKey().setAutoInc());
        contractTable.append(dbfield(qsl("KreditorId"), QVariant::LongLong).setNotNull());
        contractTable.append(dbForeignKey(contractTable[qsl("KreditorId")],
            dkdbstructur[qsl("Kreditoren")][qsl("id")], qsl("ON DELETE CASCADE")));
        // deleting a creditor will delete inactive contracts but not
        // contracts with existing bookings (=active or terminated contracts)
        contractTable.append(dbfield(qsl("Kennung"), QVariant::String, qsl("UNIQUE")));
        contractTable.append(dbfield(qsl("Anmerkung"), QVariant::String).setDefault(""));
        contractTable.append(dbfield(qsl("ZSatz"), QVariant::Int).setNotNull().setDefault(0)); // 100-stel %; 100 entspricht 1%
        contractTable.append(dbfield(qsl("Betrag"), QVariant::Int).setNotNull().setDefault(0)); // ct
        contractTable.append(dbfield(qsl("thesaurierend"), QVariant::Int).setNotNull().setDefault(1));
        contractTable.append(dbfield(qsl("Vertragsdatum"), QVariant::Date).setNotNull());
        contractTable.append(dbfield(qsl("Kfrist"), QVariant::Int).setNotNull().setDefault(6));
        contractTable.append(dbfield(qsl("AnlagenId"), QVariant::Int));
        contractTable.append(dbForeignKey(contractTable[qsl("AnlagenId")],
            qsl("Geldanlagen"), qsl("rowid"), qsl("ON DELETE SET NULL")));
        contractTable.append(dbfield(qsl("LaufzeitEnde"), QVariant::Date).setNotNull().setDefault(qsl("9999-12-31")));
        contractTable.append(dbfield(qsl("Zeitstempel"), QVariant::DateTime).setDefaultNow());
    }
    return contractTable;
}
/*static*/const dbtable& contract::getTableDef_deletedContracts()
{
    static dbtable exContractTable(qsl("exVertraege"));
    if( 0 not_eq exContractTable.Fields().size())
        return exContractTable;

    exContractTable.append(dbfield(qsl("id"), QVariant::LongLong).setPrimaryKey());
    for(int i= 1 /* not 0 */; i < getTableDef().Fields().count(); i++) {
        exContractTable.append(getTableDef().Fields()[i]);
    }
    exContractTable.append(dbForeignKey(exContractTable[qsl("KreditorId")],
                         dkdbstructur[qsl("Kreditoren")][qsl("id")], qsl("ON DELETE CASCADE")));
    return exContractTable;
}
/*static*/ bool contract::remove(qlonglong id)
{
    QString sql=qsl("DELETE FROM Vertraege WHERE id=") + QString::number(id);
    return executeSql_wNoRecords(sql);
}
/*static*/QString contract::booking_csv_header()
{
    return qsl("Vorname; Nachname; Email; Strasse; Plz; Stadt; IBAN; Kennung; Auszahlend;"
           "Beginn; Buchungsdatum; Zinssatz; Kreditbetrag; Zins; Endbetrag");
}

// construction
void contract::initContractDefaults(const qlonglong creditorId /*=-1*/)
{
    setId(-1);
    setCreditorId(creditorId);
    setNoticePeriod(6);
    //setPlannedEndDate(EndOfTheFuckingWorld); - implicit
    setInterestModel(interestModel::reinvest);
    setConclusionDate(QDate::currentDate());
    setInterestRate(1.50);
    setPlannedInvest(1000000);
    setInvestment(0);
}
void contract::initRandom(qlonglong creditorId)
{   //LOG_CALL_W(QString::number(creditorId));

    static QRandomGenerator *rand = QRandomGenerator::system();
    setLabel(proposeContractLabel());
    setCreditorId(creditorId);
    setInterestModel(fromInt(rand->bounded(100)%3));
    setInterestRate(1 +rand->bounded(149) /100.);
    setPlannedInvest(    rand->bounded(50)*1000.
                       + rand->bounded(1,3) *500.
                       + rand->bounded(10) *100.);
    setConclusionDate(QDate::currentDate().addYears(-2).addDays(rand->bounded(720)));
    if( rand->bounded(100)%5)
        // in 4 von 5 Fällen
        setNoticePeriod(3 + rand->bounded(21));
    else
        setPlannedEndDate(conclusionDate().addYears(rand->bounded(3)).addMonths(rand->bounded(12)));
}
contract::contract(qlonglong contractId) : td(getTableDef())
{
    if( contractId <= 0) {
        initContractDefaults();
    } else {
        qInfo() << "init contract from DB (id " << contractId << " )";
        QSqlRecord rec = executeSingleRecordSql(getTableDef().Fields(), "id=" + QString::number(contractId));
        if( not td.setValues(rec))
            qCritical() << "contract from id could not be created";
        aDate = activationDate();
        latestB = latestBooking();
    }
}
// interface
double contract::value(const QDate d) const
{
    // what is the value of the contract at a given time?
    QString where {qsl("VertragsId=%1 AND Datum <='%2'")};
    where =where.arg(QString::number(id()), d.toString(Qt::ISODate));
    QVariant v = executeSingleValueSql(qsl("SUM(Betrag)"), qsl("Buchungen"), where);
    if( v.isValid())
        return euroFromCt(v.toInt());
    return 0.;
}

double contract::investedValue(const QDate d) const
{
    // how many money was put into the contract by the creditor?
    QString where{ qsl("VertragsId=%1 AND Datum <='%2' AND (Buchungsart=%3 OR Buchungsart=%4) ") };
    where = where.arg(QString::number(id()), d.toString(Qt::ISODate),
        QString::number(booking::bookingTypeToInt(booking::Type::deposit)),
        QString::number(booking::bookingTypeToInt(booking::Type::payout)));
    QVariant v = executeSingleValueSql(qsl("SUM(Betrag)"), qsl("Buchungen"), where);
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

const booking& contract::latestBooking()
{   LOG_CALL;
    if( latestB.type not_eq booking::Type::non or not isActive())
        return latestB;
    QSqlRecord rec = executeSingleRecordSql(dkdbstructur[qsl("Buchungen")].Fields(), qsl("VertragsId=") + id_aS(), qsl("Datum DESC LIMIT 1"));
    if( not rec.isEmpty()) {
        latestB.type   =booking::Type(rec.value(qsl("BuchungsArt")).toInt());
        latestB.date   =rec.value(qsl("Datum")).toDate();
        latestB.amount =euroFromCt(rec.value(qsl("Betrag")).toInt());
        latestB.contractId = id();
    }
    qInfo() << booking::displayString(latestB.type) << ", " << latestB.date << ", " << latestB.amount << ", cId:" << latestB.contractId;
    return latestB;
}
// write to db
int  contract::saveNewContract()
{   LOG_CALL;
    int lastid =td.InsertData();
    if( lastid >= 0) {
        setId(lastid);
        qDebug() << "Neuer Vertrag wurde eingefügt mit id:" << lastid;
        return lastid;
    }
    qCritical() << "Fehler beim Einfügen eines neuen Vertrags";
    return -1;
}

void contract::updateComment(const QString &c)
{   LOG_CALL;
    setComment(c);
    int i =td.UpdateData();
    if( i >= 0)
        qInfo() << "Contract " << i << " was updated";
    else
        qCritical() << "Contract update failed";
}

bool contract::updateTerminationDate(QDate termination, int noticePeriod)
{
    td.setValue(qsl("Kfrist"), noticePeriod);
    td.setValue(qsl("LaufzeitEnde"), termination);
    return  -1 not_eq td.UpdateData();
}

bool contract::updateInvestment(qlonglong id)
{
    setInvestment( id);
    if( -1 == td.UpdateData())
        return false;
    return true;
}
// helper: only annual settlements should be on the last day of the year
// other bookings should move to dec. 30th
QDate avoidYearEndBookings(const QDate d)
{
    if( not d.isValid()) return d;
    if( d.month() == 12 and d.day() == 31) {
        qInfo() << "no deposit possible on dec. 31st -> switching to 30th";
        return d.addDays(-1);
    }
    return d;
}

// contract activation
bool contract::activate(const QDate date, double amount)
{   LOG_CALL;
    Q_ASSERT(id() >= 0);
    QString error;
    QDate actualDate =avoidYearEndBookings(date);

    if (isActive()) {
        error = qsl("Already active contract can not be activated");
    } else if ( not actualDate.isValid()) {
        error = qsl("Invalid Date");
    } else if( amount < 0) {
        error =qsl("Invalid amount");
    } else if ( not booking::bookDeposit(id(), actualDate, amount)) {
        error = "Failed to execut activation on contract " + id_aS() + qsl(" [") + actualDate.toString() + qsl(", ") + QString::number(amount) + qsl("]");
    }
    if ( not error.isEmpty()) {
        qCritical() << error;
        return false;
    }
    setLatestBooking({id(), booking::Type::deposit, actualDate, amount});
    setActivationDate(actualDate);

    qInfo() << "Successfully activated contract " << id_aS() << "[" << actualDate << ", " << amount
            << " Euro]";
    return true;
}
bool contract::isActive() const
{   if( activated == uninit) {
        QString sql = qsl("SELECT count(*) FROM Buchungen WHERE VertragsId=") + QString::number(id());
        activated = (0 < executeSingleValueSql(sql).toInt()) ? active : passive;
    }
    return activated==active;
}
QDate contract::activationDate() const
{
    if( not isActive())
        return QDate();
    if( aDate.isValid())
        return aDate;
    QString where = qsl("Buchungen.VertragsId=%1");
    return aDate =executeSingleValueSql(qsl("MIN(Datum)"), qsl("Buchungen"), where.arg(id())).toDate();
}
// booking actions
QDate contract::nextDateForAnnualSettlement()
{
    const booking& lastB=latestBooking();
    Q_ASSERT(lastB.date.isValid());

    if( lastB.type == booking::Type::annualInterestDeposit) {
        // the last booking was a annual one - next one should be after 1 year
        Q_ASSERT(lastB.date.month() == 12);
        Q_ASSERT(lastB.date.day() == 31);
        return QDate(lastB.date.year() +1, 12, 31);
    }
    if(lastB.type == booking::Type::payout) {
        // in early versions payouts of annual interests could be after the annualInterestDeposits
        if((lastB.date.month() == 12 and lastB.date.day() == 31))
            return QDate(lastB.date.addYears(1));
        if((lastB.date.month() == 1 and lastB.date.day() == 1))
            return QDate(lastB.date.addDays(-1).addYears(1));
    }
    Q_ASSERT( not ((lastB.date.month() == 12) and (lastB.date.day() == 31)));
    // for deposits, payouts, activations we return year end of the same year
    return QDate(lastB.date.year(), 12, 31);
}

bool contract::needsAnnualSettlement(const QDate intendedNextBooking)
{   LOG_CALL_W(intendedNextBooking.toString());

    if( not isActive()) return false;
    if( latestBooking().date >= intendedNextBooking) {
        qInfo() << "Latest booking date too young for this settlement";
        return false;
    }
    // annualInvestments are at the last day of the year
    // we need a settlement if the latest booking was in the last year
    // or it was the settlement of the last year
    if( latestBooking().date.year() == intendedNextBooking.year())
        return false;
    if( latestBooking().date.addDays(1).year() == intendedNextBooking.year())
        return false;
    return true;
}

int contract::annualSettlement( int year)
{   LOG_CALL_W(QString::number(year));
    // perform annualSettlement, recursive until 'year'
    // or only once if year is 0

    if( not isActive()) return 0;

    executeSql_wNoRecords(qsl("SAVEPOINT as_savepoint"));
    QDate requestedSettlementDate(year, 12, 31);
    QDate nextAnnualSettlementDate =nextDateForAnnualSettlement();

    bool bookingSuccess =false;
    while(nextAnnualSettlementDate <= requestedSettlementDate) {
        double baseValue =interestBearingValue();
                     //////////
        double zins =ZinsesZins(interestRate(), baseValue, latestBooking().date, nextAnnualSettlementDate);
                     //////////
        switch(iModel()) {
        case interestModel::reinvest:
        case interestModel::fixed:
        case interestModel::zero: {
            if( (bookingSuccess =booking::bookAnnualInterestDeposit(id(), nextAnnualSettlementDate, zins)))
                latestB = { id(),  booking::Type::annualInterestDeposit, nextAnnualSettlementDate, zins };
            break;
        }
        case interestModel::payout: {
            bookingSuccess = booking::bookPayout(id(), nextAnnualSettlementDate, zins);
            bookingSuccess &=booking::bookAnnualInterestDeposit(id(), nextAnnualSettlementDate, zins);
            latestB = { id(), booking::Type::annualInterestDeposit, nextAnnualSettlementDate, zins };
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
            continue;
        } else {
            qDebug() << "Failed annual settlement: Vertrag " << id_aS() << ": " << nextAnnualSettlementDate << " Zins: " << zins;
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
bool contract::bookInBetweenInterest(const QDate nextBookingDate)
{   LOG_CALL;
    // booking interest in case of deposits, payouts or finalization
    // performs annualSettlements if necesarry

    QString error;
    if( not isActive()) error =qsl("interest booking on inactive contract not possible");
    else if( not nextBookingDate.isValid())  error =qsl("Invalid Date for interest booking");
    else if( latestBooking().date > nextBookingDate) error =qsl("could not book interest because there are already more recent bookings");
    if( not error.isEmpty()) {
        qCritical() << error;
        return false;
    }
    if( needsAnnualSettlement(nextBookingDate)) {
        qInfo() << "Perform annual settlement first - this is unusual";
        if(0 == annualSettlement(nextBookingDate.year() -1)) {
            qCritical() << "annual settlement during interest booking failed";
            return false;
        }
    }
                   //////////
    double zins = ZinsesZins(interestRate(), interestBearingValue(), latestBooking().date, nextBookingDate);
                   //////////
    // only annualSettlements can be payed out
    if( booking::bookReInvestInterest(id(), nextBookingDate, zins)) {
        latestB = {id(), booking::Type::reInvestInterest, nextBookingDate, zins};
        return true;
    }
    return false;
}

bool contract::deposit(const QDate d, double amount)
{   LOG_CALL;
    double actualAmount = qFabs(amount);
    QString error;
    if( not isActive())
        error = qsl("could not put money on an inactive account");
    else if ( not d.isValid())
        error = qsl("Year End is a Invalid Date") + d.toString();
    else if ( latestBooking().date >= d)
        error = qsl("bookings have to be in a consecutive order. Last booking: ")
                + latestBooking().date.toString() + qsl(" this booking: ") + d.toString();
    if( not error.isEmpty()) {
        qCritical() << error;
        return false;
    }
    QDate actualD =avoidYearEndBookings(d);
    // update interest calculation
    QSqlDatabase::database().transaction();
    if( not bookInBetweenInterest(actualD)) {
        QSqlDatabase::database().rollback();
        return false;
    }
    if( not booking::bookDeposit(id(), actualD, actualAmount)) {
        QSqlDatabase::database().rollback();
        return false;
    }
    QSqlDatabase::database().commit();
    latestB = { id(),  booking::Type::deposit, actualD, actualAmount };
    return true;
}

bool contract::payout(const QDate d, double amount)
{   LOG_CALL;
    double actualAmount = qFabs(amount);

    QString error;
    if( actualAmount > value()) error = qsl("Payout impossible. The account has not enough coverage");
    else if( not d.isValid()) error =qsl("Invalid Date in payout");
    else if(  latestBooking().date >= d) error = qsl("Bookings have to be in a consecutive order. Last booking: ")
                + latestBooking().date.toString() + qsl(" this booking: ") + d.toString();
    if( not error.isEmpty()) {
        qCritical() << error;
        return false;
    }
    QDate actualD =avoidYearEndBookings(d);
    // update interest calculation
    QSqlDatabase::database().transaction();
    if( not bookInBetweenInterest(actualD)) {
        QSqlDatabase::database().rollback();
        return false;
    }
    if( not booking::bookPayout(id(), actualD, actualAmount)) {
        QSqlDatabase::database().rollback();
        qCritical() << "booking of payout failed";
        return false;
    }
    QSqlDatabase::database().commit();
    latestB ={id(), booking::Type::payout, actualD, actualAmount};
    return true;

}

bool contract::cancel(const QDate d)
{   LOG_CALL;
    if( not isActive()) {
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
    if( not finDate.isValid() or finDate < latestBooking().date or id() == -1) {
        qDebug() << "invalid date to finalize contract:" << finDate;
        return false;
    }
    if( not isActive()) {
        qDebug() << "could not finalize inactive contract";
        return false;
    }
    qlonglong id_to_be_deleted = id();
    qInfo() << "Finalization startet at value " << value();
    executeSql_wNoRecords(qsl("SAVEPOINT fin"));
    // as we are terminating the contract we have to sum up all interests
    setInterestModel(interestModel::reinvest);
    if( needsAnnualSettlement(finDate))
        if( not annualSettlement(finDate.year() -1)) {
            executeSql_wNoRecords(qsl("ROLLBACK"));
            return false;
        }
    double preFinValue = value(finDate);
    qInfo() << "Fin. Value after annual settlement:" << preFinValue;
    finInterest = ZinsesZins(interestRate(), preFinValue, latestBooking().date, finDate);
    finPayout = preFinValue +finInterest;
    if( simulate) {
        executeSql_wNoRecords(qsl("ROLLBACK"));
        return  true;
    }
    bool allGood =false;
    do {
        if( not booking::bookReInvestInterest(id(), finDate, finInterest)) break;
        latestB = { id(), booking::Type::reInvestInterest, finDate, finInterest };
        if( not booking::bookPayout(id(), finDate, finPayout)) break;
        latestB = { id(), booking::Type::payout, finDate, finPayout };
        if( value() not_eq 0.) break;
        if( not storeTerminationDate(finDate)) break;
        if( not archive()) break;
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

QString contract::toString(const QString &title) const
{
    QString ret;
    QTextStream stream(&ret);
    stream << title << Qt::endl;
    if( id() <=0)
        stream << qsl("[contract was not saved or loaded from DB]") << Qt::endl;
    else
        stream << qsl("[id:") << id_aS() << qsl("]") << Qt::endl;
    if( not isActive()) {
        stream << "Wert (gepl.):     " << plannedInvest() << Qt::endl;
        stream << "Zinssatz (gepl.): " << interestRate() << Qt::endl;
        return ret;
    }
    stream << "Wert:     " << value() << Qt::endl;
    stream << "Zinssatz: " << interestRate() << Qt::endl;
    stream << "Buchungen:" << bookings::getBookings(id()).count() << Qt::endl;
    stream << "Letzte B. " << booking::typeName(latestB.type) << qsl(", ")
           << latestB.amount << qsl(", ") << latestB.date.toString() << Qt::endl;
    return ret;
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
// test helper
contract saveRandomContract(qlonglong creditorId)
{   LOG_CALL;
    contract c;
    c.initRandom(creditorId);
    c.saveNewContract();
    return c;
}
void saveRandomContracts(int count)
{   LOG_CALL;
    Q_ASSERT(count>0);
    QVector<QVariant> creditorIds = executeSingleColumnSql(dkdbstructur[qsl("Kreditoren")][qsl("id")]);
    if( creditorIds.size() == 0)
        qDebug() << "No Creditors to create contracts for";

    static QRandomGenerator* rand = QRandomGenerator::system();
    for (int i = 0; i<count; i++)
        saveRandomContract(creditorIds[rand->bounded(creditorIds.size())].toLongLong());
}
QDate activateRandomContracts(const int percent)
{   LOG_CALL;
    QDate minimumActivationDate =EndOfTheFuckingWorld; // needed for tests
    if( percent < 0 or percent > 100) return minimumActivationDate;

    QVector<QSqlRecord> contractData = executeSql(contract::getTableDef().Fields());
    int activations = contractData.count() * percent / 100;
    static QRandomGenerator* rand = QRandomGenerator::system();

    for (int i=0; i < activations; i++) {
        // contractData -> from database all amounts are in ct
        double amount = euroFromCt(contractData[i].value(qsl("Betrag")).toInt());
        if( rand->bounded(100)%10 == 0) {
            // some contracts get activated with a different amount
            amount = amount * rand->bounded(90, 110) / 100;
        }
        QDate activationDate(contractData[i].value(qsl("Vertragsdatum")).toDate());
        activationDate = activationDate.addDays(rand->bounded(100));
        if( activationDate < minimumActivationDate)
            minimumActivationDate =activationDate;
        contract c(contractData[i].value(qsl("id")).toInt());
        c.activate(activationDate, amount);
    }
    return minimumActivationDate;
}
