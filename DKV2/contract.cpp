#include <QVector>
#include <QRandomGenerator>

#include "helper.h"
#include "contract.h"

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
    ti.setValue(dkdbstructur["Vertraege"]["ZSatz"].name(), interestRate());
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

contract saveRandomContract(qlonglong creditorId)
{   LOG_CALL;
    static QRandomGenerator *rand = QRandomGenerator::system();

    contract c;
    c.setLabel(proposeKennung());
    c.setCreditorId(creditorId);
    c.setReinvesting(rand->bounded(100)%4);
    c.setInterestRate(1 +rand->bounded(149));
    c.setPlannedInvest(rand->bounded(50)*1000. + rand->bounded(10)*100.);
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
    static QRandomGenerator* rand = QRandomGenerator::system();
    Q_ASSERT(count>0);
    QVector<QVariant> creditorIds = executeSingleColumnSql("id", "Kreditoren");
    if( creditorIds.size() == 0) {
        qDebug() << "No Creditors to create contracts for";
    }
    for (int i = 0; i<count; i++)
    {
        saveRandomContract(creditorIds[rand->bounded(creditorIds.size())].toLongLong());
    }
}

