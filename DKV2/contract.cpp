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
QDate contract::latestBooking() const
{
    return executeSingleValueSql("MAX(Datum)", "Buchungen", "VertragsId="+QString::number(id())).toDate();
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
bool contract::activate(const QDate& aDate, int amount_ct) const
{   LOG_CALL;
    Q_ASSERT (id()>=0);
    if( isActive()) {
        qCritical() << "Already active contract can not be activated";
        return false;
    }
    if( ! aDate.isValid()) {
        qCritical() << "Invalid Date";
        return false;
    }
    if( amount_ct < 0 || ! booking::makeDeposit( id(), aDate, euroFromCt(amount_ct))) {
        qCritical() << "failed to conduct activation on contract " << id() << "[" << aDate << ", " << amount_ct << "]";
        return false;
    }
    qInfo() << "Successfully activated contract " << id() << "[" << aDate << ", " << amount_ct << " ct]";
    return true;
}
bool contract::activate( const QDate& aDate, double amount) const
{
    int ct = ctFromEuro(amount);
    return activate(aDate, ct);
}
bool contract::isActive() const
{
    QString sql = "SELECT count(*) FROM Buchungen WHERE VertragsId=" + QString::number(id());
    return 0 < executeSingleValueSql(sql).toInt();
}
QDate contract::activationDate() const
{
    QString where = "Buchungen.VertragsId=%1";
    return executeSingleValueSql("MIN(Datum)", "Buchungen", where.arg(id())).toDate();
}
// booking actions
int contract::annualSettlement() const
{   LOG_CALL;
    if( ! isActive()) return 0;
    QDate lastBooking =latestBooking();
    QDate target =QDate(lastBooking.year() +1, 1, 1);
    double zins =ZinsesZins(interestRate(), value(), lastBooking, target);
    if( reinvesting()
            ? booking::investInterest(id(), target, zins)
            : booking::payoutInterest(id(), target, -1*zins)) {
        qInfo() << "successfull annual settlement: Vertrag " << id() << ": " << target << " Zins: " << zins;
        return lastBooking.year() +1;
    } else {
        qDebug() << "failed annual settlement: Vertrag " << id() << ": " << target << " Zins: " << zins;
        return 0;
    }
}
bool contract::bookInterest(QDate d) const
{   LOG_CALL;
    if( ! d.isValid()) {
        qCritical() << "Invalid Date";
        return false;
    }
    QDate lastBooking =latestBooking();
    if( lastBooking > d) {
        qCritical() << "could not book interest because there are already more recent bookings";
        return false;
    }
    QSqlDatabase::database().transaction();
    while( d.year() > lastBooking.year()) {
        qInfo() << "perform annual settlement first";
        if(0 == annualSettlement()) {
            qCritical() << "annual settlement during interest booking failed";
            QSqlDatabase::database().rollback();
            return false;
        }
        lastBooking =latestBooking();
    }
    double zins = ZinsesZins(interestRate(), value(), lastBooking, d);
    if( reinvesting()
        ? booking::investInterest(id(), d, zins)
        : booking::payoutInterest(id(), d, -1.*zins))
    {
        qInfo() << "Successfull interest booking: " << id() << d << zins;
        QSqlDatabase::database().commit();
        return true;
    }
    qCritical() << "interest booking failed" << id() << d << zins;
    QSqlDatabase::database().rollback();
    return false;
}
bool contract::deposit(QDate d, double amount) const
{   LOG_CALL;
    Q_ASSERT(amount > 0);
    if( ! d.isValid() || (d.day() == 1 && d.month() == 1)) {
        qCritical() << "Invalid Date" << d;
        return false;
    }
    if( ! isActive()) {
        qCritical() << "could not put money on an inactive account";
        return false;
    }
    if( latestBooking() >= d) {
        qCritical() << "bookings have to be in a consecutive order. Last booking: " << latestBooking()
                    << " this booking: " << d;
        return false;
    }
    // update interest calculation
    if( ! bookInterest(d)) {
        qCritical() << "intrest booking failed";
        return false;
    }
    if( ! booking::makeDeposit(id(), d, amount)) {
        qCritical() << "deposit booking failed";
        return false;
    }
    return true;
}
bool contract::payout(QDate d, double amount) const
{   LOG_CALL;
    if( amount < 0) amount *= -1.;
    if( amount > value()) {
        qCritical() << "Payout impossible. The account has not enough coverage";
        return false;
    }
    if( ! d.isValid() || (d.day() == 1 && d.month() == 1)) {
        qCritical() << "Invalid Date";
        return false;
    }
    QDate last=latestBooking();
    if( last >= d) {
        qCritical() << "bookings have to be in a consecutive order. Last booking: " << last
                    << " this booking: " << d;
        return false;
    }
    // update interest calculation
    if( ! bookInterest(d)) {
        qCritical() << "intrest booking failed";
        return false;
    }
    return booking::makePayout(id(), d, amount);
}
bool contract::cancel(QDate d)
{   LOG_CALL;
    if( ! id()) {
        qInfo() << "an invalid contract can not be canceled";
        return false;
    }
    QString sql ="UPDATE Vertraege SET LaufzeitEnde='%1', Kfrist=%2 WHERE id=%3";
    sql =sql.arg(d.toString(Qt::ISODate)).arg(-1).arg(id());
    if( ! executeSql(sql)) {
        qDebug() << "contract update to cancel can not be done. ";
        return false;
    }
    setPlannedEndDate(d);
    return true;
}

bool contract::finalize(bool simulate, const QDate finDate,
                        double& finInterest, double& finPayout)
{   LOG_CALL;
    if( ! finDate.isValid() || finDate < latestBooking()) {
        qDebug() << "invalid date to finalize contract";
        return false;
    }
    while( latestBooking().year() < finDate.year())
    {
        // as we are terminating the contract we have to sum up all interests
        setReinvesting(true);
        if( ! annualSettlement()) {
            qDebug() << "error with annual settlement during contract termination";
            return false;
        }
    }
    double cv = value(finDate);
    finInterest = ZinsesZins(interestRate(), cv, latestBooking(), finDate);
    finPayout = cv +finInterest;
    if( simulate)
        return  true;
    QSqlDatabase::database().transaction();
    if( ! booking::investInterest(id(), finDate, finInterest)) {
        qDebug() << "faild to book final interest";
        QSqlDatabase::database().rollback();
        return false;
    }
    if( ! booking::makePayout(id(), finDate, value())) {
        qDebug() << "faild to book final payout";
        QSqlDatabase::database().rollback();
        return false;
    }
    if( value() != 0.) {
        qDebug() << "final payout did not equlize the contract";
        QSqlDatabase::database().rollback();
        return false;
    }
    QSqlDatabase::database().commit();
    return true;
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

    QVector<QSqlRecord> contracts = executeSql(contract::getTableDef().Fields());
    int activations = contracts.count() * percent / 100;
    static QRandomGenerator* rand = QRandomGenerator::system();
    for (int i=0; i < activations; i++) {
        int amount = contracts[i].value("Betrag").toInt();
        if( rand->bounded(100)%10 == 0) {
            // some contracts get activated with a different amount
            amount = amount * rand->bounded(90, 110) / 100;
        }
        QDate activationDate(contracts[i].value("Vertragsdatum").toDate());
        activationDate = activationDate.addDays(rand->bounded(50));

        contract c(contracts[i].value("id").toInt());
        c.activate(activationDate, amount);
    }
}
