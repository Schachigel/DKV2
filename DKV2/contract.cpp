#include <QVector>

#include "helper.h"
#include "dbfield.h"
#include "sqlhelper.h"
#include "finhelper.h"
#include "dkdbhelper.h"
#include "contract.h"

/* static */ const dbtable& contract::getTableDef()
{
    static dbtable contracttable("Vertraege");
    if( 0 == contracttable.Fields().size())
    {
        contracttable.append(dbfield("id",         QVariant::LongLong, "PRIMARY KEY AUTOINCREMENT"));
        contracttable.append(dbfield("KreditorId", QVariant::LongLong, "", creditor::getTableDef()["id"], dbfield::refIntOption::onDeleteCascade));
        contracttable.append(dbfield("Kennung",    QVariant::String, "UNIQUE"));
        contracttable.append(dbfield("ZSatz",      QVariant::Int, "DEFAULT 0 NOT NULL")); // 100-stel %; 100 entspricht 1%
        contracttable.append(dbfield("Betrag",     QVariant::Int, "DEFAULT '0.0' NOT NULL"));
        contracttable.append(dbfield("thesaurierend", QVariant::Bool, "DEFAULT '1' NOT NULL"));
        contracttable.append(dbfield("Vertragsdatum", QVariant::Date, "DATE  NULL"));
        contracttable.append(dbfield("Kfrist" ,    QVariant::Int, "DEFAULT '6' NOT NULL"));
        contracttable.append(dbfield("LaufzeitEnde",  QVariant::Date, "DEFAULT '9999-12-31' NOT NULL"));
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

contract randomContract(qlonglong creditorId)
{
    static QRandomGenerator *rand = QRandomGenerator::system();

    contract c;
    c.setLabel(proposeKennung());
    c.setCreditorId(creditorId);
    c.setReinvesting(rand->bounded(100)%4);
    c.setInterestRate(1 +rand->bounded(149));
    c.setPlannedInvest(rand->bounded(50)*1000. + rand->bounded(10)*100.);
    if( rand->bounded(100)%3) {
        QDate enddate;
        do {
        enddate = QDate(rand->bounded(4), rand->bounded(12), rand->bounded(31));
        } while( !enddate.isValid());
        c.setPlannedEndDate(enddate);
    }
    else {
        c.setNoticePeriod(3 + rand->bounded(21));
    }
    QDate cd{QDate::currentDate()};
    cd = cd.addYears(-2).addDays(rand->bounded(720));
    c.setConclusionDate(cd);
    return c;
}

