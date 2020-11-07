#include <QVector>
#include <QRandomGenerator>
#include <QTextStream>


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
        contractTable.append(dbfield(qsl("thesaurierend"), QVariant::Bool).setNotNull().setDefault(1));
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
           " Buchungsdatum; Zinssatz; Kreditbetrag; Zins; Endbetrag");
}

// construction
void contract::init()
{
    setId(-1);
    setCreditorId(-1);
    setNoticePeriod(6);
    //setPlannedEndDate(EndOfTheFuckingWorld); - implicit
    setReinvesting(true);
    setConclusionDate(QDate::currentDate());
    setInterestRate(1.50);
    setPlannedInvest(1000000);
}
void contract::initRandom(qlonglong creditorId)
{   //LOG_CALL_W(QString::number(creditorId));

    static QRandomGenerator *rand = QRandomGenerator::system();
    setLabel(proposeContractLabel());
    setCreditorId(creditorId);
    setReinvesting(0 != rand->bounded(100)%5);// 20% auszahlend
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
    }
}
// interface
double contract::value() const
{
    return value(EndOfTheFuckingWorld);
}
double contract::value(QDate d) const
{
    // what is the value of the contract at a given time?
    QString where {qsl("VertragsId=%1 AND Datum <='%2'")};
    where =where.arg(QString::number(id()), d.toString(Qt::ISODate));
    QVariant v = executeSingleValueSql(qsl("SUM(Betrag)"), qsl("Buchungen"), where);
    if( v.isValid())
        return euroFromCt(v.toInt());
    return 0.;
}
booking contract::latestBooking()
{
    if( latest.type != booking::Type::non)
        return latest;
    QSqlRecord rec = executeSingleRecordSql(dkdbstructur[qsl("Buchungen")].Fields(), qsl("VertragsId=") + id_aS(), qsl("Datum DESC LIMIT 1"));
    if( ! rec.isEmpty()) {
        latest.type   =booking::Type(rec.value(qsl("BuchungsArt")).toInt());
        latest.date   =rec.value(qsl("Datum")).toDate();
        latest.amount =euroFromCt(rec.value(qsl("Betrag")).toInt());
    }
    return latest;
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

// contract activation
bool contract::activate(const QDate &actDate, double amount)
{   LOG_CALL;
    Q_ASSERT(id() >= 0);
    QString error;
    if (isActive()) {
        error = qsl("Already active contract can not be activated");
    } else if (!actDate.isValid()) {
        error = qsl("Invalid Date");
    } else if( amount < 0) {
        error =qsl("invalid amount");
    } else if ( ! booking::bookDeposit(id(), actDate, amount)) {
        error = "failed to conduct activation on contract " + id_aS() + qsl(" [") + actDate.toString() + qsl(", ") + QString::number(amount) + qsl("]");
    }
    if (!error.isEmpty()) {
        qCritical() << error;
        return false;
    }
    latest = {id(), booking::Type::deposit, actDate, amount};
    aDate = actDate;
    activated = active;
    qInfo() << "Successfully activated contract " << id() << "[" << actDate << ", " << amount
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
    if( ! isActive())
        return QDate();
    if( aDate.isValid())
        return aDate;
    QString where = qsl("Buchungen.VertragsId=%1");
    return aDate =executeSingleValueSql(qsl("MIN(Datum)"), qsl("Buchungen"), where.arg(id())).toDate();
}
// booking actions
int contract::annualSettlement( int year, bool transactual)
{   LOG_CALL_W(QString::number(year));
    // perform annualSettlement, recursive until 'year'
    // or only once if year is 0

    if( ! isActive()) return 0;

    bool bookingSuccess =false;
    if( transactual) QSqlDatabase::database().transaction();

    while(latestBooking().date.year() <= year) {
        QDate newYearAfterLastBooking = QDate((latestBooking().date.year()+1), 1, 1);
                     //////////
        double zins =ZinsesZins(interestRate(), value(), latestBooking().date, newYearAfterLastBooking);
                     //////////
        if( reinvesting()) {
            if( (bookingSuccess =booking::bookAnnualInterestDeposit(id(), newYearAfterLastBooking, zins)))
                latest = { id(),  booking::Type::annualInterestDeposit, newYearAfterLastBooking, zins };
        } else {
            bookingSuccess =booking::bookAnnualInterestDeposit(id(), newYearAfterLastBooking, zins);
            bookingSuccess &= booking::bookPayout(id(), newYearAfterLastBooking, zins);
            latest = { id(), booking::Type::payout, newYearAfterLastBooking, zins };
        }
        if( bookingSuccess) {
            qInfo() << "Successfull annual settlement: contract id " << id() << ": " << newYearAfterLastBooking << " Zins: " << zins;
            continue;
        } else {
            qDebug() << "failed annual settlement: Vertrag " << id() << ": " << newYearAfterLastBooking << " Zins: " << zins;
            if( transactual) QSqlDatabase::database().rollback();
            return 0;
        }
    }

    if( transactual) QSqlDatabase::database().commit();
    return year;
}
// booking actions
bool contract::bookInterest(QDate nextBookingDate)
{   LOG_CALL;
    // booking interest in case of deposits or payouts
    // performs annualSettlements if necesarry

    QString error;
    if( ! isActive()) error =qsl("interest booking on inactive contract not possible");
    else if( ! nextBookingDate.isValid())  error =qsl("Invalid Date");
    else if( latestBooking().date > nextBookingDate) error =qsl("could not book interest because there are already more recent bookings");
    if( ! error.isEmpty()) {
        qCritical() << error;
        return false;
    }

    if( nextBookingDate.year() > latestBooking().date.year()) {
        qInfo() << "perform annual settlement first";
        if(0 == annualSettlement(nextBookingDate.year() -1, false)) {
            qCritical() << "annual settlement during interest booking failed";
            return false;
        }
    }
                   //////////
    double zins = ZinsesZins(interestRate(), value(), latestBooking().date, nextBookingDate);
                   //////////
    // only annualSettlements can be payed out
    if( booking::bookReInvestInterest(id(), nextBookingDate, zins)) {
        latest = {id(), booking::Type::reInvestInterest, nextBookingDate, zins};
        return true;
    }

    return false;
}
bool contract::deposit(QDate d, double amount)
{   LOG_CALL;
    Q_ASSERT(amount > 0);
    QString error;
    if( ! isActive())
        error = qsl("could not put money on an inactive account");
    else if ( ! d.isValid() || (d.day() == 1 && d.month() == 1))
        error = qsl("Invalid Date") + d.toString();
    else if ( latestBooking().date >= d)
        error = qsl("bookings have to be in a consecutive order. Last booking: ")
                + latestBooking().date.toString() + qsl(" this booking: ") + d.toString();
    if( ! error.isEmpty()) {
        qCritical() << error;
        return false;
    }

    // update interest calculation
    QSqlDatabase::database().transaction();
    if( ! bookInterest(d)) {
        QSqlDatabase::database().rollback();
        return false;
    }
    if( ! booking::bookDeposit(id(), d, amount)) {
        QSqlDatabase::database().rollback();
        return false;
    }
    QSqlDatabase::database().commit();
    latest = { id(),  booking::Type::deposit, d, amount };
    return true;
}
bool contract::payout(QDate d, double amount)
{   LOG_CALL;
    if( amount < 0) amount *= -1.;

    QString error;
    if( amount > value()) error = qsl("Payout impossible. The account has not enough coverage");
    else if( ! d.isValid() || (d.day() == 1 && d.month() == 1)) error =qsl("Invalid Date or new year");
    else if(  latestBooking().date >= d) error = qsl("bookings have to be in a consecutive order. Last booking: ")
                + latestBooking().date.toString() + qsl(" this booking: ") + d.toString();
    if( ! error.isEmpty()) {
        qCritical() << error;
        return false;
    }

    // update interest calculation
    QSqlDatabase::database().transaction();
    if( ! bookInterest(d)) {
        QSqlDatabase::database().rollback();
        return false;
    }
    if( booking::bookPayout(id(), d, amount)) {
        QSqlDatabase::database().commit();
        latest ={id(), booking::Type::payout, d, amount};
        return true;
    }
    QSqlDatabase::database().rollback();
    qCritical() << "booking of payout failed";
    return false;
}
bool contract::cancel(QDate d)
{   LOG_CALL;
    if( ! isActive()) {
        qInfo() << "an inactive contract can not be canceled. It should be deleted.";
        return false;
    }
    QString sql =qsl("UPDATE Vertraege SET LaufzeitEnde=?, Kfrist=? WHERE id=?");
    QVector<QVariant> v {d.toString(Qt::ISODate), -1, id()};
    if( ! executeSql(sql, v)) {
        return false;
    }
    setPlannedEndDate(d);
    return true;
}
bool contract::finalize(bool simulate, const QDate finDate,
                        double& finInterest, double& finPayout)
{   LOG_CALL;
    if( ! finDate.isValid() || finDate < latestBooking().date || id() == -1) {
        qDebug() << "invalid date to finalize contract";
        return false;
    }
    if( ! isActive()){
        qDebug() << "could not finalize inactive contract";
        return false;
    }
    qlonglong id_to_be_deleted = id();
    QSqlDatabase::database().transaction();
    // as we are terminating the contract we have to sum up all interests
    setReinvesting(true);
    if( ! annualSettlement(finDate.year() -1, false)) {
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
        latest = { id(), booking::Type::reInvestInterest, finDate, finInterest };
        if( ! booking::bookPayout(id(), finDate, finPayout)) break;
        latest = { id(), booking::Type::payout, finDate, finPayout };
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

QString contract::toString(QString title)
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
    stream << "Letzte B. " << booking::typeName(latestBooking().type) << qsl(", ")
           << latestBooking().amount << qsl(", ") << latestBooking().date.toString() << Qt::endl;
    return ret;
}

bool contract::storeTerminationDate(QDate d) const
{   LOG_CALL;
    QVector<QVariant> v {d, id()};
    return executeSql(qsl("UPDATE Vertraege SET LaufzeitEnde=? WHERE id=?"), v);
}

bool contract::archive()
{   LOG_CALL;
    // no check isActive() cause this is only called from finalize which does the check already
    // no check value()==0 cause this is done in finalize already
    // secured by the transaction of finalize()

    // move all bookings and the contract to the archive tables
    if( executeSql(qsl("INSERT INTO exVertraege SELECT * FROM Vertraege WHERE id=?"), id()))
     if( executeSql(qsl("INSERT INTO exBuchungen SELECT * FROM Buchungen WHERE VertragsId=?"), id()))
      if( executeSql(qsl("DELETE FROM Buchungen WHERE VertragsId=?"), id()))
       if( executeSql(qsl("DELETE FROM Vertraege WHERE id=?"), id()))
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
