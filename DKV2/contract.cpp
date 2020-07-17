#include <QVector>
#include <QRandomGenerator>

#include "helper.h"
#include "contract.h"
#include "booking.h"

// statics & friends
const dbtable& contract::getTableDef()
{
    static dbtable contractTable("Vertraege");
    if( 0 != contractTable.Fields().size())
        return contractTable;

    contractTable.append(dbfield("id",         QVariant::LongLong).setPrimaryKey().setAutoInc());
    contractTable.append(dbfield("KreditorId", QVariant::LongLong).setNotNull());
    contractTable.append(dbForeignKey(contractTable["KreditorId"],
                         dkdbstructur["Kreditoren"]["id"], "ON DELETE CASCADE"));
    // deleting a creditor will delete inactive contracts but not
    // contracts with existing bookings (=active or terminated contracts)
    contractTable.append(dbfield("Kennung",    QVariant::String, "UNIQUE"));
    contractTable.append(dbfield("ZSatz",      QVariant::Int).setNotNull().setDefault(0)); // 100-stel %; 100 entspricht 1%
    contractTable.append(dbfield("Betrag",     QVariant::Int).setNotNull().setDefault(0)); // ct
    contractTable.append(dbfield("thesaurierend", QVariant::Bool).setNotNull().setDefault(1));
    contractTable.append(dbfield("Vertragsdatum", QVariant::Date).setNotNull());
    contractTable.append(dbfield("Kfrist" ,    QVariant::Int).setNotNull().setDefault(6));
    contractTable.append(dbfield("LaufzeitEnde",  QVariant::Date).setNotNull().setDefault("9999-12-31"));

    return contractTable;
}
const dbtable& contract::getTableDef_deletedContracts()
{
    static dbtable exContractTable("exVertraege");
    if( 0 != exContractTable.Fields().size())
        return exContractTable;

    exContractTable.append(dbfield("id", QVariant::LongLong).setPrimaryKey());
    for(int i= 1 /* not 0 */; i < getTableDef().Fields().count(); i++) {
        exContractTable.append(getTableDef().Fields()[i]);
    }
    exContractTable.append(dbForeignKey(exContractTable["KreditorId"],
                         dkdbstructur["Kreditoren"]["id"], "ON DELETE CASCADE"));
    return exContractTable;
}
bool contract::remove(qlonglong id)
{
    QString sql="DELETE FROM Vertraege WHERE id=" + QString::number(id);
    QSqlQuery deleteQ;
    if( deleteQ.exec(sql))
        return true;
    if( "19" == deleteQ.lastError().nativeErrorCode())
        qDebug() << "Delete contract failed due to refer. integrity rules" << endl << deleteQ.lastQuery();
    else
        qCritical() << "Delete contract failed "<< deleteQ.lastError() << endl << deleteQ.lastQuery();
    return false;

}
QString contract::booking_csv_header()
{
    return "Vorname; Nachname; Email; Strasse; Plz; Stadt; IBAN; Kennung; Auszahlend;"
           " Buchungsdatum; Zinssatz; Kreditbetrag; Zins; Endbetrag";
}

// construction
void contract::init()
{
    setId(-1);
    //setPlannedEndDate(EndOfTheFuckingWorld);
    setNoticePeriod(6);
    setReinvesting(true);
    setConclusionDate(QDate::currentDate());
    setInterest100th(150);
    setPlannedInvest(1000000);
}
contract::contract(qlonglong i) : td(getTableDef())
{
    if( i >0) {
        QSqlRecord rec = executeSingleRecordSql(getTableDef().Fields(), "id=" + QString::number(i));
        if( td.setValues(rec))
            return;
        else
            qCritical() << "contract from id could not be created";
    }
    init();
}
// interface
double contract::value() const
{
    return value(EndOfTheFuckingWorld);
}
double contract::value(QDate d) const
{
    // what is the value of the contract at a given time?
    QString where = "VertragsId=%1 AND BuchungsArt!=%2 AND Datum <='%3'";
    where =where.arg(id());
    where =where.arg(booking::Type::interestPayout);
    where =where.arg(d.toString(Qt::ISODate));
    QVariant v = executeSingleValueSql("SUM(Betrag)", "Buchungen", where);
    if( v.isValid())
        return euroFromCt(v.toInt());
    return 0.;
}
booking contract::
latestBooking()
{
    if( latest.type != booking::Type::non)
        return latest;
    QSqlRecord rec = executeSingleRecordSql(dkdbstructur["Buchungen"].Fields(), "VertragsId=" + id_aS(), "Datum DESC LIMIT 1");
    if( ! rec.isEmpty()) {
        latest.type   =booking::Type(rec.value("BuchungsArt").toInt());
        latest.date   =rec.value("Datum").toDate();
        latest.amount =euroFromCt(rec.value("Betrag").toInt());
    }
    return latest;
}
// write to db
int  contract::saveNewContract()
{   LOG_CALL;
    TableDataInserter ti(dkdbstructur["Vertraege"]);
    ti.setValue(dkdbstructur["Vertraege"]["KreditorId"].name(), creditorId());
    ti.setValue(dkdbstructur["Vertraege"]["Kennung"].name(), label());
    ti.setValue(dkdbstructur["Vertraege"]["Betrag"].name(), ctFromEuro(plannedInvest()));
    ti.setValue(dkdbstructur["Vertraege"]["ZSatz"].name(), interestRate()*100);
    ti.setValue(dkdbstructur["Vertraege"]["thesaurierend"].name(), reinvesting());
    ti.setValue(dkdbstructur["Vertraege"]["Vertragsdatum"].name(), conclusionDate());
    ti.setValue(dkdbstructur["Vertraege"]["LaufzeitEnde"].name(), plannedEndDate().isValid() ? plannedEndDate() : EndOfTheFuckingWorld);
    ti.setValue(dkdbstructur["Vertraege"]["Kfrist"].name(), noticePeriod());
    int lastid =ti.InsertData();
    if( lastid >= 0)
    {
        setId(lastid);
        qDebug() << "Neuer Vertrag wurde eingefügt mit id:" << lastid;
        return lastid;
    }
    qCritical() << "Fehler beim Einfügen eines neuen Vertrags";
    return -1;
}
bool contract::validateAndSaveNewContract(QString& meldung)
{   LOG_CALL;
    meldung.clear();
    if( plannedInvest() <=0)
        meldung = "Der Kreditbetrag muss größer als null sein";
    else if( creditorId() <= 0 )
        meldung = "Wähle den Kreditgeber. Ist die Auswahl leer muss zuerst ein Kreditor angelegt werden";
    else if( label() == "")
        meldung= "Du solltest eine eindeutige Kennung vergeben, damit der Kredit besser zugeordnet werden kann";
    if( !meldung.isEmpty())
        return false;

    bool buchungserfolg = saveNewContract();
    if( !buchungserfolg)
        meldung = "Der Vertrag konnte nicht gespeichert werden. Ist die Kennung des Vertrags eindeutig?";
    return buchungserfolg;
}
// contract activation
bool contract::activate( const QDate& aDate, double amount)
{
    LOG_CALL;
     Q_ASSERT (id()>=0);
     if( isActive()) {
         qCritical() << "Already active contract can not be activated";
         return false;
     }
     if( ! aDate.isValid()) {
         qCritical() << "Invalid Date";
         return false;
     }
     if( amount < 0 || ! booking::makeDeposit( id(), aDate, amount )) {
         qCritical() << "failed to conduct activation on contract " << id() << "[" << aDate << ", " << amount << "]";
         return false;
     }
     latest ={booking::Type::deposit, aDate, amount};
     qInfo() << "Successfully activated contract " << id() << "[" << aDate << ", " << amount << " Euro]";
     return true;
}
bool contract::isActive() const
{
    QString sql = "SELECT count(*) FROM Buchungen WHERE VertragsId=" + QString::number(id());
    return 0 < executeSingleValueSql(sql).toInt();
}
QDate contract::activationDate() const
{
    static QDate aDate;
    if( aDate.isValid())
        return aDate;
    QString where = "Buchungen.VertragsId=%1";
    return aDate =executeSingleValueSql("MIN(Datum)", "Buchungen", where.arg(id())).toDate();
}
// booking actions
int contract::annualSettlement( int year)
{   LOG_CALL;
    if( ! isActive()) return 0;
    QDate nextTarget = QDate(latestBooking().date.year() +1, 1, 1);
    QDate target = (year == 0) ? nextTarget : QDate(year+1, 1, 1);

    bool bookingSuccess =false;
    while(latestBooking().date < target) {
        double zins =ZinsesZins(interestRate(), value(), latestBooking().date, nextTarget);
        if( reinvesting()) {
            if( (bookingSuccess =booking::investInterest(id(), nextTarget, zins)))
                latest ={booking::Type::interestDeposit, nextTarget, zins};
        } else {
            zins *= -1.;
            if( (bookingSuccess =booking::payoutInterest(id(), nextTarget, zins)))
                latest ={booking::Type::interestPayout, nextTarget, zins};
        }
        if( bookingSuccess) {
            qInfo() << "Successfull annual settlement: contract id " << id() << ": " << nextTarget << " Zins: " << zins;
            continue;
        } else {
            qDebug() << "failed annual settlement: Vertrag " << id() << ": " << nextTarget << " Zins: " << zins;
            return 0;
        }
    }
    return bookingSuccess ? target.year() : 0;
}
// booking actions
bool contract::bookInterest(QDate d, bool transactual)
{   LOG_CALL;
    if( ! isActive()) {
        qCritical() << "interest booking on inactive contract not possible";
        return false;
    }
    // transactual =false allows to use this funciton with an outer transaction (sqlite does not support nested transaction)
    if( ! d.isValid()) {
        qCritical() << "Invalid Date";
        return false;
    }
    if( latestBooking().date > d) {
        qCritical() << "could not book interest because there are already more recent bookings";
        return false;
    }
    if( transactual)
        QSqlDatabase::database().transaction();
    while( d.year() > latestBooking().date.year()) {
        qInfo() << "perform annual settlement first";
        if(0 == annualSettlement()) {
            qCritical() << "annual settlement during interest booking failed";
            if( transactual)
                QSqlDatabase::database().rollback();
            return false;
        }
    }
    double zins = ZinsesZins(interestRate(), value(), latestBooking().date, d);
    if( reinvesting()
        ? booking::investInterest(id(), d, zins)
        : booking::payoutInterest(id(), d, -1.*zins))
    {
        if( transactual)
            QSqlDatabase::database().commit();
        latest = { reinvesting() ? booking::Type::interestDeposit : booking::Type::interestPayout,
                   d,
                   reinvesting() ? zins : -1.*zins};
        return true;
    }
    if( transactual)
        QSqlDatabase::database().rollback();
    return false;
}
bool contract::deposit(QDate d, double amount)
{   LOG_CALL;
    Q_ASSERT(amount > 0);
    if( ! isActive()) {
        qCritical() << "could not put money on an inactive account";
        return false;
    }
    if( ! d.isValid() || (d.day() == 1 && d.month() == 1)) {
        qCritical() << "Invalid Date" << d;
        return false;
    }
    if( latestBooking().date >= d) {
        qCritical() << "bookings have to be in a consecutive order. Last booking: " << latestBooking().date
                    << " this booking: " << d;
        return false;
    }
    // update interest calculation
    if( ! bookInterest(d, false)) {
        return false;
    }
    if( ! booking::makeDeposit(id(), d, amount)) {
        return false;
    }
    latest ={booking::Type::deposit, d, amount};
    return true;
}
bool contract::payout(QDate d, double amount)
{   LOG_CALL;
    if( amount < 0) amount *= -1.;
    if( amount > value()) {
        // so we do not need to check if contract is active
        qCritical() << "Payout impossible. The account has not enough coverage";
        return false;
    }
    if( ! d.isValid() || (d.day() == 1 && d.month() == 1)) {
        qCritical() << "Invalid Date";
        return false;
    }
    if( latestBooking().date >= d) {
        qCritical() << "bookings have to be in a consecutive order. Last booking: " << latestBooking().date
                    << " this booking: " << d;
        return false;
    }
    // update interest calculation
    if( ! bookInterest(d)) {
        return false;
    }
    if( booking::makePayout(id(), d, amount)) {
        latest ={booking::Type::payout, d, amount};
        return true;
    }
    qCritical() << "booking of payout failed";
    return false;
}
bool contract::cancel(QDate d)
{   LOG_CALL;
    if( ! isActive()) {
        qInfo() << "an inactive contract can not be canceled. It should be deleted.";
        return false;
    }
    QString sql ="UPDATE Vertraege SET LaufzeitEnde=?, Kfrist=? WHERE id=?";
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
    while( latestBooking().date.year() < finDate.year()) {
        if( ! annualSettlement()) {
            QSqlDatabase::database().rollback();
            return false;
        }
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
        if( ! booking::investInterest(id(), finDate, finInterest)) break;
        latest ={booking::Type::interestDeposit, finDate, finInterest};
        if( ! booking::makePayout(id(), finDate, finPayout)) break;
        latest ={booking::Type::payout, finDate, finPayout};
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

bool contract::storeTerminationDate(QDate d) const
{   LOG_CALL;
    QVector<QVariant> v {d, id()};
    return executeSql("UPDATE Vertraege SET LaufzeitEnde=? WHERE id=?", v);
}

bool contract::archive()
{   LOG_CALL;
    // no check isActive() cause this is only called from finalize which does the check already
    // no check value()==0 cause this is done in finalize already
    // secured by the transaction of finalize()

    // move all bookings and the contract to the archive tables
    bool res = false;
    do {
        if( ! executeSql("INSERT INTO exVertraege SELECT * FROM Vertraege WHERE id=?", id())) break;
        if( ! executeSql("INSERT INTO exBuchungen SELECT * FROM Buchungen WHERE VertragsId=?", id())) break;
        if( ! executeSql("DELETE FROM Buchungen WHERE VertragsId=?", id())) break;
        if( ! executeSql("DELETE FROM Vertraege WHERE id=?", id())) break;
        res =true;
    } while( false);
    if(res) {
        qInfo() << "contract was moved to the contract archive";
        return true;
    }
    qCritical() << "contract could not be moved to the archive";
    return false;
}
// test helper
contract saveRandomContract(qlonglong creditorId)
{   LOG_CALL;
    static QRandomGenerator *rand = QRandomGenerator::system();

    contract c;
    c.setLabel(proposeKennung());
    c.setCreditorId(creditorId);
    c.setReinvesting(rand->bounded(100)%6);// 16% auszahlend
    c.setInterest100th(1 +rand->bounded(149));
    c.setPlannedInvest(    rand->bounded(50)*1000.
                           + rand->bounded(1,3) *500.
                           + rand->bounded(10) *100);
    c.setConclusionDate(QDate::currentDate().addYears(-2).addDays(rand->bounded(720)));
    if( rand->bounded(100)%5)
        // in 4 von 5 Fällen
        c.setNoticePeriod(3 + rand->bounded(21));
    else
        c.setPlannedEndDate(c.conclusionDate().addYears(rand->bounded(3)).addMonths(rand->bounded(12)));

    c.saveNewContract();
    return c;
}
void saveRandomContracts(int count)
{   LOG_CALL;
    Q_ASSERT(count>0);
    QVector<QVariant> creditorIds = executeSingleColumnSql(dkdbstructur["Kreditoren"]["id"]);
    if( creditorIds.size() == 0)
        qDebug() << "No Creditors to create contracts for";

    static QRandomGenerator* rand = QRandomGenerator::system();
    for (int i = 0; i<count; i++)
        saveRandomContract(creditorIds[rand->bounded(creditorIds.size())].toLongLong());
}
void activateRandomContracts(int percent)
{   LOG_CALL;
    if( percent < 0 || percent > 100) return;

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
        QDate activationDate(contractData[i].value("Vertragsdatum").toDate());
        activationDate = activationDate.addDays(rand->bounded(50));

        contract c(contractData[i].value("id").toInt());
        c.activate(activationDate, amount);
    }
}
