#include <QVector>
#include <QRandomGenerator>
#include <QTextStream>
#include <QtMath>

#include "helper.h"
#include "contract.h"
#include "booking.h"

// statics & friends
const dbtable& contract::getTableDef()
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
        contractTable.append(dbfield(qsl("ZSatz"), QVariant::Int).setNotNull().setDefault(0)); // 100-stel %; 100 entspricht 1%
        contractTable.append(dbfield(qsl("Betrag"), QVariant::Int).setNotNull().setDefault(0)); // ct
        contractTable.append(dbfield(qsl("thesaurierend"), QVariant::Int).setNotNull().setDefault(1));
        contractTable.append(dbfield(qsl("Vertragsdatum"), QVariant::Date).setNotNull());
        contractTable.append(dbfield(qsl("Kfrist"), QVariant::Int).setNotNull().setDefault(6));
        contractTable.append(dbfield(qsl("LaufzeitEnde"), QVariant::Date).setNotNull().setDefault(qsl("9999-12-31")));
    }
    return contractTable;
}
const dbtable& contract::getTableDef_deletedContracts()
{
    static dbtable exContractTable(qsl("exVertraege"));
    if( 0 != exContractTable.Fields().size())
        return exContractTable;

    exContractTable.append(dbfield(qsl("id"), QVariant::LongLong).setPrimaryKey());
    for(int i= 1 /* not 0 */; i < getTableDef().Fields().count(); i++) {
        exContractTable.append(getTableDef().Fields()[i]);
    }
    exContractTable.append(dbForeignKey(exContractTable[qsl("KreditorId")],
                         dkdbstructur[qsl("Kreditoren")][qsl("id")], qsl("ON DELETE CASCADE")));
    return exContractTable;
}
bool contract::remove(qlonglong id)
{
    QString sql=qsl("DELETE FROM Vertraege WHERE id=") + QString::number(id);
    QSqlQuery deleteQ;
    if( deleteQ.exec(sql))
        return true;
    if( qsl("19") == deleteQ.lastError().nativeErrorCode())
        qDebug() << qsl("Delete contract failed due to refer. integrity rules") << Qt::endl << deleteQ.lastQuery();
    else
        qCritical() << qsl("Delete contract failed ")<< deleteQ.lastError() << Qt::endl << deleteQ.lastQuery();
    return false;

}
QString contract::booking_csv_header()
{
    return qsl("Vorname; Nachname; Email; Strasse; Plz; Stadt; IBAN; Kennung; Auszahlend;"
           "Beginn; Buchungsdatum; Zinssatz; Kreditbetrag; Zins; Endbetrag");
}

// construction
void contract::init()
{
    setId(-1);
    setCreditorId(-1);
    setNoticePeriod(6);
    //setPlannedEndDate(EndOfTheFuckingWorld); - implicit
    setInterestModel(interestModel::accumulating);
    setConclusionDate(QDate::currentDate());
    setInterestRate(1.50);
    setPlannedInvest(1000000);
}
void contract::initRandom(qlonglong creditorId)
{   //LOG_CALL_W(QString::number(creditorId));

    static QRandomGenerator *rand = QRandomGenerator::system();
    setLabel(proposeContractLabel());
    setCreditorId(creditorId);
    setInterestModel(InterestModelfromInt(0 != rand->bounded(100)%5));// 20% auszahlend
    setInterestRate(1 +rand->bounded(149) /100.);
    setPlannedInvest(    rand->bounded(50)*1000.
                       + rand->bounded(1,3) *500.
                       + rand->bounded(10) *100);
    setConclusionDate(QDate::currentDate().addYears(-2).addDays(rand->bounded(720)));
    if( rand->bounded(100)%5)
        // in 4 von 5 Fällen
        setNoticePeriod(3 + rand->bounded(21));
    else
        setPlannedEndDate(conclusionDate().addYears(rand->bounded(3)).addMonths(rand->bounded(12)));
//    qDebug() << toString();
}
contract::contract(qlonglong i) : td(getTableDef())
{
    if( i <= 0) {
        init();
    } else {
        QSqlRecord rec = executeSingleRecordSql(getTableDef().Fields(), "id=" + QString::number(i));
        if( ! td.setValues(rec))
            qCritical() << "contract from id could not be created";
        aDate = activationDate();
        latestB = latestBooking();
    }
}
// interface
double contract::value() const
{
    return value(EndOfTheFuckingWorld);
}
double contract::value(const QDate& d) const
{
    // what is the value of the contract at a given time?
    QString where {qsl("VertragsId=%1 AND Datum <='%2'")};
    where =where.arg(QString::number(id()), d.toString(Qt::ISODate));
    QVariant v = executeSingleValueSql(qsl("SUM(Betrag)"), qsl("Buchungen"), where);
    if( v.isValid())
        return euroFromCt(v.toInt());
    return 0.;
}
const booking& contract::latestBooking()
{
    if( latestB.type != booking::Type::non)
        return latestB;
    QSqlRecord rec = executeSingleRecordSql(dkdbstructur[qsl("Buchungen")].Fields(), qsl("VertragsId=") + id_aS(), qsl("Datum DESC LIMIT 1"));
    if( ! rec.isEmpty()) {
        latestB.type   =booking::Type(rec.value(qsl("BuchungsArt")).toInt());
        latestB.date   =rec.value(qsl("Datum")).toDate();
        latestB.amount =euroFromCt(rec.value(qsl("Betrag")).toInt());
        latestB.contractId = id();
    }
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

// helper: only annual settlements should be on the last day of the year
// other bookings should move to dec. 30th
QDate avoidYearEndBookings(const QDate& d)
{
    if( d.month() == 12 && d.day() == 31) {
        qInfo() << "no deposit possible on dec. 31st -> switching to 30th";
        return d.addDays(-1);
    }
    return d;
}

// contract activation
bool contract::activate(const QDate &date, const double& amount)
{   LOG_CALL;
    Q_ASSERT(id() >= 0);
    QString error;
    QDate actualDate =avoidYearEndBookings(date);

    if (isActive()) {
        error = qsl("Already active contract can not be activated");
    } else if (!date.isValid()) {
        error = qsl("Invalid Date");
    } else if( amount < 0) {
        error =qsl("Invalid amount");
    } else if ( ! booking::bookDeposit(id(), actualDate, amount)) {
        error = "Failed to execut eactivation on contract " + id_aS() + qsl(" [") + actualDate.toString() + qsl(", ") + QString::number(amount) + qsl("]");
    }
    if (!error.isEmpty()) {
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
    if( !isActive())
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
    if(lastB.type == booking::Type::payout
        && lastB.date.month() == 12
        && lastB.date.day() == 31) {
        // in early versions payouts of annual interests could be after the annualInterestDeposits
        return QDate(lastB.date.addYears(1));
    }
    Q_ASSERT(!((lastB.date.month() == 12) && (lastB.date.day() == 31)));
    // for deposits, payouts, activations we return year end of the same year
    return QDate(lastB.date.year(), 12, 31);
}

bool contract::needsAnnualSettlement(const QDate& date)
{   LOG_CALL_W(date.toString());
    if( !isActive()) return false;
    if( latestBooking().date >= date) {
        qInfo() << "Latest booking date too young for this settlement";
        return false;
    }
    // annualInvestments are at the last day of the year
    // we need a settlement if the latest booking was in the last year
    // or it was the settlement of the last year
    if( latestBooking().date.addDays(1).year() == date.year())
        return false;

    return true;
}

int contract::annualSettlement( int year, bool transactual)
{   LOG_CALL_W(QString::number(year));
    // perform annualSettlement, recursive until 'year'
    // or only once if year is 0

    if( ! isActive()) return 0;

    if( transactual) QSqlDatabase::database().transaction();
    QDate requestedSettlementDate(year, 12, 31);
    QDate nextAnnualSettlementDate =nextDateForAnnualSettlement();

    bool bookingSuccess =false;
    while(nextAnnualSettlementDate <= requestedSettlementDate) {
                     //////////
        double zins =ZinsesZins(interestRate(), value(), latestBooking().date, nextAnnualSettlementDate);
                     //////////
        if( interestModel()== interestModel::accumulating) {
            if( (bookingSuccess =booking::bookAnnualInterestDeposit(id(), nextAnnualSettlementDate, zins)))
                latestB = { id(),  booking::Type::annualInterestDeposit, nextAnnualSettlementDate, zins };
        } else {
            bookingSuccess = booking::bookPayout(id(), nextAnnualSettlementDate, zins);
            bookingSuccess &=booking::bookAnnualInterestDeposit(id(), nextAnnualSettlementDate, zins);
            latestB = { id(), booking::Type::annualInterestDeposit, nextAnnualSettlementDate, zins };
        }
        if( bookingSuccess) {
            qInfo() << "Successfull annual settlement: contract id " << id_aS() << ": " << nextAnnualSettlementDate << " Zins: " << zins;
            // latest booking has changes, so ...
            nextAnnualSettlementDate = nextDateForAnnualSettlement();
            continue;
        } else {
            qDebug() << "Failed annual settlement: Vertrag " << id_aS() << ": " << nextAnnualSettlementDate << " Zins: " << zins;
            if( transactual) QSqlDatabase::database().rollback();
            return 0;
        }
    }
    if( transactual) QSqlDatabase::database().commit();
    if( bookingSuccess)
        // there was a booking
        return year;
    else
        return 0;
}

// booking actions
bool contract::bookInBetweenInterest(const QDate& nextBookingDate)
{   LOG_CALL;
    // booking interest in case of deposits, payouts or finalization
    // performs annualSettlements if necesarry

    QString error;
    if( ! isActive()) error =qsl("interest booking on inactive contract not possible");
    else if( ! nextBookingDate.isValid())  error =qsl("Invalid Date for interest booking");
    else if( latestBooking().date > nextBookingDate) error =qsl("could not book interest because there are already more recent bookings");
    if( ! error.isEmpty()) {
        qCritical() << error;
        return false;
    }

    if( needsAnnualSettlement(nextBookingDate)) {
        qInfo() << "Perform annual settlement first - this is unusual";
// set temporarily to 'reinvesting' otherwise the payout might be forgotton
//        bool reinv = reinvesting();
//        setReinvesting();
        if(0 == annualSettlement(nextBookingDate.year() -1, false)) {
//            setReinvesting(reinv);
            qCritical() << "annual settlement during interest booking failed";
            return false;
        }
//        setReinvesting(reinv);
    }
                   //////////
    double zins = ZinsesZins(interestRate(), value(), latestBooking().date, nextBookingDate);
                   //////////
    // only annualSettlements can be payed out
    if( booking::bookReInvestInterest(id(), nextBookingDate, zins)) {
        latestB = {id(), booking::Type::reInvestInterest, nextBookingDate, zins};
        return true;
    }

    return false;
}

bool contract::deposit(const QDate& d, const double& amount)
{   LOG_CALL;
    Q_ASSERT(amount > 0);
    QString error;
    if( ! isActive())
        error = qsl("could not put money on an inactive account");
    else if ( !d.isValid() || (d.day() == 1 && d.month() == 1))
        error = qsl("Invalid Date") + d.toString();
    else if ( latestBooking().date >= d)
        error = qsl("bookings have to be in a consecutive order. Last booking: ")
                + latestBooking().date.toString() + qsl(" this booking: ") + d.toString();
    if( ! error.isEmpty()) {
        qCritical() << error;
        return false;
    }
    QDate actualD =avoidYearEndBookings(d);
    // update interest calculation
    QSqlDatabase::database().transaction();
    if( !bookInBetweenInterest(actualD)) {
        QSqlDatabase::database().rollback();
        return false;
    }
    if( ! booking::bookDeposit(id(), actualD, amount)) {
        QSqlDatabase::database().rollback();
        return false;
    }
    QSqlDatabase::database().commit();
    latestB = { id(),  booking::Type::deposit, actualD, amount };
    return true;
}

bool contract::payout(const QDate& d, const double& amount)
{   LOG_CALL;
    double actualAmount = qFabs(amount);

    QString error;
    if( actualAmount > value()) error = qsl("Payout impossible. The account has not enough coverage");
    else if( ! d.isValid() || (d.day() == 1 && d.month() == 1)) error =qsl("Invalid Date or new year");
    else if(  latestBooking().date >= d) error = qsl("Bookings have to be in a consecutive order. Last booking: ")
                + latestBooking().date.toString() + qsl(" this booking: ") + d.toString();
    if( ! error.isEmpty()) {
        qCritical() << error;
        return false;
    }
    QDate actualD =avoidYearEndBookings(d);
    // update interest calculation
    QSqlDatabase::database().transaction();
    if( !bookInBetweenInterest(actualD)) {
        QSqlDatabase::database().rollback();
        return false;
    }
    if( booking::bookPayout(id(), actualD, actualAmount)) {
        QSqlDatabase::database().commit();
        latestB ={id(), booking::Type::payout, actualD, actualAmount};
        return true;
    }
    QSqlDatabase::database().rollback();
    qCritical() << "booking of payout failed";
    return false;
}

bool contract::cancel(const QDate& d)
{   LOG_CALL;
    if( ! isActive()) {
        qInfo() << "an inactive contract can not be canceled. It should be deleted.";
        return false;
    }
    QDate actualD =avoidYearEndBookings(d);
    QString sql =qsl("UPDATE Vertraege SET LaufzeitEnde=?, Kfrist=? WHERE id=?");
    QVector<QVariant> v {actualD.toString(Qt::ISODate), -1, id()};
    if( ! executeSql_wNoRecords(sql, v)) {
        return false;
    }
    setPlannedEndDate(actualD);
    return true;
}
bool contract::finalize(bool simulate, const QDate& finDate,
                        double& finInterest, double& finPayout)
{   LOG_CALL;
    if( ! finDate.isValid() || finDate < latestBooking().date || id() == -1) {
        qDebug() << "invalid date to finalize contract:" << finDate;
        return false;
    }
    if( ! isActive()){
        qDebug() << "could not finalize inactive contract";
        return false;
    }
    qlonglong id_to_be_deleted = id();
    QSqlDatabase::database().transaction();
    // as we are terminating the contract we have to sum up all interests
    setInterestModel(interestModel::accumulating);
    if( needsAnnualSettlement(finDate))
        if( !annualSettlement(finDate.year() -1, false)) {
            QSqlDatabase::database().rollback();
            return false;
        }
    double cv = value(finDate);
    finInterest = ZinsesZins(interestRate(), cv, latestBooking().date, finDate);
    finPayout = cv +finInterest;
    if( simulate) {
        QSqlDatabase::database().rollback();
        return  true;
    }
    bool allGood =false;
    do {
        if( ! booking::bookReInvestInterest(id(), finDate, finInterest)) break;
        latestB = { id(), booking::Type::reInvestInterest, finDate, finInterest };
        if( ! booking::bookPayout(id(), finDate, finPayout)) break;
        latestB = { id(), booking::Type::payout, finDate, finPayout };
        if( value() != 0.) break;
        if( ! storeTerminationDate(finDate)) break;
        if( ! archive()) break;
        allGood = true;
    } while(false);

    if( allGood) {
        QSqlDatabase::database().commit();
        reset();
        qInfo() << "successfully finalized and archived contract " << id_to_be_deleted;
        return true;
    } else {
        qCritical() << "contract finalizing failed";
        QSqlDatabase::database().rollback();
        return false;
    }
}

QString contract::toString(QString title) const
{
    QString ret;
    QTextStream stream(&ret);
    stream << title << Qt::endl;
    if( id() <=0)
        stream << qsl("[contract was not saved or loaded from DB]") << Qt::endl;
    else
        stream << qsl("[id:") << id_aS() << qsl("]") << Qt::endl;
    if( bookings::getBookings(id()).count() == 0) {
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

bool contract::storeTerminationDate(const QDate& d) const
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
    if( executeSql_wNoRecords(qsl("INSERT INTO exVertraege SELECT * FROM Vertraege WHERE id=?"), id()))
     if( executeSql_wNoRecords(qsl("INSERT INTO exBuchungen SELECT * FROM Buchungen WHERE VertragsId=?"), id()))
      if( executeSql_wNoRecords(qsl("DELETE FROM Buchungen WHERE VertragsId=?"), id()))
       if( executeSql_wNoRecords(qsl("DELETE FROM Vertraege WHERE id=?"), id()))
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
QDate activateRandomContracts(int percent)
{   LOG_CALL;
    QDate minimumActivationDate =EndOfTheFuckingWorld; // needed for tests
    if( percent < 0 || percent > 100) return minimumActivationDate;

    QVector<QSqlRecord> contractData = executeSql(contract::getTableDef().Fields());
    int activations = contractData.count() * percent / 100;
    static QRandomGenerator* rand = QRandomGenerator::system();

    for (int i=0; i < activations; i++) {
        // contractData -> from database all amounts are in ct
        double amount = euroFromCt(contractData[i].value("Betrag").toInt());
        if( rand->bounded(100)%10 == 0) {
            // some contracts get activated with a different amount
            amount = amount * rand->bounded(90, 110) / 100;
        }
        QDate activationDate(contractData[i].value(qsl("Vertragsdatum")).toDate());
        activationDate = activationDate.addDays(rand->bounded(50));
        if( activationDate < minimumActivationDate)
            minimumActivationDate =activationDate;
        contract c(contractData[i].value(qsl("id")).toInt());
        c.activate(activationDate, amount);
    }
    return minimumActivationDate;
}
