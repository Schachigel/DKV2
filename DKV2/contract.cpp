#include <QVector>
#include <QRandomGenerator>

#include "helper.h"
#include "contract.h"
#include "booking.h"

/* static */ const dbtable& contract::getTableDef()
{
    static dbtable contracttable("Vertraege");
    if( 0 == contracttable.Fields().size())
    {
        contracttable.append(dbfield("id",         QVariant::LongLong).setPrimaryKey().setAutoInc());
        contracttable.append(dbfield("KreditorId", QVariant::LongLong).setNotNull());
        contracttable.append(dbForeignKey(contracttable["KreditorId"], dkdbstructur["Kreditoren"]["id"], "ON DELETE RESTRICT"));
        contracttable.append(dbfield("Kennung",    QVariant::String, "UNIQUE"));
        contracttable.append(dbfield("ZSatz",      QVariant::Int).setNotNull().setDefault(0)); // 100-stel %; 100 entspricht 1%
        contracttable.append(dbfield("Betrag",     QVariant::Int).setNotNull().setDefault(0)); // Euro
        contracttable.append(dbfield("thesaurierend", QVariant::Bool).setNotNull().setDefault(1));
        contracttable.append(dbfield("Vertragsdatum", QVariant::Date).setNotNull());
        contracttable.append(dbfield("Kfrist" ,    QVariant::Int).setNotNull().setDefault(6));
        contracttable.append(dbfield("LaufzeitEnde",  QVariant::Date).setNotNull().setDefault("9999-12-31"));
    }
    return contracttable;
}

bool contract::fromDb(qlonglong i)
{
    QSqlRecord rec = executeSingleRecordSql(getTableDef().Fields(), "id=" + QString::number(i));
    return td.setValues(rec);
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
int contract::saveNewContract()
{   LOG_CALL;
    TableDataInserter ti(dkdbstructur["Vertraege"]);
    ti.setValue(dkdbstructur["Vertraege"]["KreditorId"].name(), creditorId());
    ti.setValue(dkdbstructur["Vertraege"]["Kennung"].name(), label());
    ti.setValue(dkdbstructur["Vertraege"]["Betrag"].name(), plannedInvest());
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

bool contract::activate(const QDate& aDate, int amount)
{
    if( amount < 0 || ! booking::makeDeposit( id(), aDate, amount)) {
        qCritical() << "failed to conduct activation to contract " << id() << "[" << aDate << ", " << amount << "]";
        return false;
    }
    qInfo() << "successfully activated contract " << id() << "[" << aDate << ", " << amount << "]";
    return true;
}

contract saveRandomContract(qlonglong creditorId)
{   LOG_CALL;
    static QRandomGenerator *rand = QRandomGenerator::system();

    contract c;
    c.setLabel(proposeKennung());
    c.setCreditorId(creditorId);
    c.setReinvesting(rand->bounded(100)%4);
    c.setInterest100th(1 +rand->bounded(149));
    c.setPlannedInvest(rand->bounded(50)*1000. + rand->bounded(1,101)*100.);
    c.setConclusionDate(QDate::currentDate().addYears(-2).addDays(rand->bounded(720)));
    if( rand->bounded(100)%3) {
        c.setPlannedEndDate(c.conclusionDate().addYears(rand->bounded(3)).addMonths(rand->bounded(12)));
    }
    else {
        c.setNoticePeriod(3 + rand->bounded(21));
    }
    c.saveNewContract();
    return c;
}
void saveRandomContracts(int count)
{   LOG_CALL;
    Q_ASSERT(count>0);
    QVector<QVariant> creditorIds = executeSingleColumnSql("id", "Kreditoren");
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
    for (int i = 0; i < activations; i++) {
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
