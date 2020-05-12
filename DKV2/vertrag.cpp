#include <QVector>

#include "helper.h"
#include "dbfield.h"
#include "sqlhelper.h"
#include "finhelper.h"
#include "dkdbhelper.h"
#include "vertrag.h"

bool Contract::loadContractFromDb(qlonglong vId)
{   LOG_CALL;
    QVector<dbfield>fields;
    fields.append(dkdbstructur["Vertraege"]["id"]);
    fields.append(dkdbstructur["Vertraege"]["KreditorId"]);
    fields.append(dkdbstructur["Vertraege"]["Kennung"]);
    fields.append(dkdbstructur["Vertraege"]["Betrag"]);
    fields.append(dkdbstructur["Vertraege"]["Wert"]);
    fields.append(dkdbstructur["Vertraege"]["ZSatz"]);
    fields.append(dkdbstructur["Vertraege"]["thesaurierend"]);
    fields.append(dkdbstructur["Vertraege"]["Vertragsdatum"]);
    fields.append(dkdbstructur["Vertraege"]["aktiv"]);
    fields.append(dkdbstructur["Vertraege"]["LaufzeitEnde"]);
    fields.append(dkdbstructur["Vertraege"]["LetzteZinsberechnung"]);
    fields.append(dkdbstructur["Vertraege"]["Kfrist"]);

    fields.append(dkdbstructur["Kreditoren"]["id"]);
    fields.append(dkdbstructur["Kreditoren"]["Vorname"]);
    fields.append(dkdbstructur["Kreditoren"]["Nachname"]);
    fields.append(dkdbstructur["Kreditoren"]["Strasse"]);
    fields.append(dkdbstructur["Kreditoren"]["Plz"]);
    fields.append(dkdbstructur["Kreditoren"]["Stadt"]);
    fields.append(dkdbstructur["Kreditoren"]["Email"]);
    fields.append(dkdbstructur["Kreditoren"]["IBAN"]);
    fields.append(dkdbstructur["Kreditoren"]["BIC"]);
    fields.append(dkdbstructur["Zinssaetze"]["id"]);
    fields.append(dkdbstructur["Zinssaetze"]["Zinssatz"]);

    QSqlRecord rec = ExecuteSingleRecordSql(fields, "[Vertraege].[id]=" +QString::number(vId));
    if( rec.isEmpty())
        return false;
    id                  = rec.value("Vertraege.id").toInt();
    kreditorId          = rec.value("KreditorId").toInt();
    kennung             = rec.value("Vertraege.Kennung").toString();
    betrag              = rec.value("Betrag").toDouble();
    wert                = rec.value("Wert").toDouble();
    zinsId              = rec.value("Vertraege.ZSatz").toInt();
    thesaurierend       = rec.value("thesaurierend").toBool();
    vertragsdatum       = rec.value("Vertragsdatum").toDate();
    active              = rec.value("aktiv").toBool();
    laufzeitEnde        = rec.value("LaufzeitEnde").toDate();
    startZinsberechnung = rec.value("LetzteZinsberechnung").toDate();
    kFrist              = rec.value("Kfrist").toInt();

    buchungsdatenJson = JsonFromRecord(rec);
    zinsFuss = rec.value("Zinssaetze.Zinssatz").toDouble();
    dkGeber.setValue("Vorname", rec.value("Kreditoren.Vorname"));
    dkGeber.setValue("Nachname", rec.value("Kreditoren.Nachname"));
    dkGeber.setValue("Strasse", rec.value("Kreditoren.Strasse"));
    dkGeber.setValue("Plz", rec.value("Kreditoren.Plz").toString());
    dkGeber.setValue("Stadt", rec.value("Kreditoren.Stadt"));
    dkGeber.setValue("Email", rec.value("Kreditoren.Email"));
    dkGeber.setValue("Iban", rec.value("Kreditoren.IBAN"));
    dkGeber.setValue("Bic", rec.value("Kreditoren.BIC"));

    return true;
}

void Contract::initCreditor()
{   LOG_CALL;
    dkGeber.fromDb(kreditorId);
}

bool Contract::validateAndSaveNewContract(QString& meldung)
{   LOG_CALL;
    meldung.clear();

    if( Betrag() <=0)
        meldung = "Der Kreditbetrag muss größer als null sein";
    else if( KreditorId() <= 0 || ZinsId() <= 0)
        meldung = "Wähle den Kreditgeber und die Zinsen aus der gegebenen Auswahl. Ist die Auswahl leer müssen zuerst Kreditgeber und Zinswerte eingegeben werden";
    else if( Kennung() == "")
        meldung= "Du solltest eine eindeutige Kennung vergeben, damit der Kredit besser zugeordnet werden kann";
    if( !meldung.isEmpty())
        return false;

    IbanValidator iv;
    int pos =0;
    QString iban =dkGeber.getValue("IBAN").toString();
    if( iv.validate(iban, pos) != IbanValidator::State::Acceptable)
        meldung = "Die Iban ist vermutlich ungültig. Bitte kontrolliere den Wert in den Daten des Kreditgebers. Der Vertrag wurde trotzdem gewpeichert!";
    bool buchungserfolg = bookNewContract();
    if( !buchungserfolg)
        meldung = "Der Vertrag konnte nicht gespeichert werden. Ist die Kennung des Vertrags eindeutig?";
    return buchungserfolg;
}

int Contract::saveNewContract() const
{   LOG_CALL;
    TableDataInserter ti(dkdbstructur["Vertraege"]);
    ti.setValue(dkdbstructur["Vertraege"]["KreditorId"].name(), kreditorId);
    ti.setValue(dkdbstructur["Vertraege"]["Kennung"].name(), kennung);
    ti.setValue(dkdbstructur["Vertraege"]["Betrag"].name(), betrag);
    ti.setValue(dkdbstructur["Vertraege"]["Wert"].name(), thesaurierend?wert:0.);
    ti.setValue(dkdbstructur["Vertraege"]["ZSatz"].name(), zinsId);
    ti.setValue(dkdbstructur["Vertraege"]["thesaurierend"].name(), thesaurierend);
    ti.setValue(dkdbstructur["Vertraege"]["Vertragsdatum"].name(), vertragsdatum);
    ti.setValue(dkdbstructur["Vertraege"]["aktiv"].name(), active);
    ti.setValue(dkdbstructur["Vertraege"]["LaufzeitEnde"].name(), laufzeitEnde.isValid()? laufzeitEnde : EndOfTheFuckingWorld);
    ti.setValue(dkdbstructur["Vertraege"]["LetzteZinsberechnung"].name(), startZinsberechnung);
    ti.setValue(dkdbstructur["Vertraege"]["Kfrist"].name(), kFrist);
    int lastid =ti.InsertData(QSqlDatabase::database());
    if( lastid >= 0)
    {
        qDebug() << "Neuer Vertrag wurde eingefügt mit id:" << lastid;
        return lastid;
    }
    qCritical() << "Fehler beim Einfügen eines neuen Vertrags";
    return -1;
}

bool Contract::bookNewContract()
{   LOG_CALL;

    QSqlDatabase::database().transaction();
    int nextId =saveNewContract();
    if( nextId>0 )
    {
        setVid( nextId);
        QSqlDatabase::database().commit();
        return true;
    }
    qCritical() << "ein neuer Vertrag konnte nicht gespeichert werden";
    QSqlDatabase::database().rollback();
    return false;
}

bool Contract::activateContract(const QDate& aDate)
{   LOG_CALL;

    QSqlDatabase::database().transaction();

    QSqlQuery updateQ;
    updateQ.prepare("UPDATE Vertraege SET LetzteZinsberechnung = :vdate, aktiv = :true WHERE id = :id");
    updateQ.bindValue(":vdate",QVariant(aDate));
    updateQ.bindValue(":id", QVariant(id));
    updateQ.bindValue(":true", QVariant(true));

    bool ret = true;
    if( !updateQ.exec())
    {
        qCritical() << "Vertrag Aktivierung fehlgeschlagen " << updateQ.lastQuery() << updateQ.lastError();
        ret = false;
    }
    if(ret)
    {
        active = true;
        startZinsberechnung = aDate;
        QSqlDatabase::database().commit();
    }
    else
    {
        qCritical() << "Aktivieren des Vertrags " << id << " fehlgeschlagen";
        QSqlDatabase::database().rollback();
    }
    return ret;
}

bool Contract::deleteInactiveContract()
{   LOG_CALL;
    if( ExecuteSingleValueSql("SELECT [aktiv] FROM [Vertraege] WHERE id=" +QString::number(id)).toBool())
    {
        qWarning() << "will not delete active contract w id:" << id;
        return false;
    }

    QSqlDatabase::database().transaction();
    QSqlQuery deleteQ;
    if( !deleteQ.exec("DELETE FROM Vertraege WHERE id=" + QString::number(id)))
    {
        qCritical() << "failed to delete Contract: " << deleteQ.lastError() << endl << deleteQ.lastQuery();
        QSqlDatabase::database().rollback();
        return false;
    }
    QSqlDatabase::database().commit();
    qDebug() << "passiver Vertrag " << id << "gelöscht";
    return true;
}

bool Contract::cancelActiveContract(const QDate& kTermin)
{   LOG_CALL;

    QSqlQuery updateQ;
    updateQ.prepare("UPDATE Vertraege SET Kfrist = '-1', LaufzeitEnde = '" + kTermin.toString(Qt::ISODate) + "' WHERE id = :id");
    updateQ.bindValue(":id", QVariant(id));
    bool ret = updateQ.exec();
    if( !ret)
    {
        qCritical() << "Error in Kuendingung " << updateQ.lastError() << endl << updateQ.lastQuery();
        return false;
    }
    return true;
}

bool Contract::terminateActiveContract( const QDate& termin)
{   LOG_CALL;
    if( !ExecuteSingleValueSql("SELECT [aktiv] FROM [Vertraege] WHERE id=" +QString::number(id)).toBool())
    {
        qWarning() << "will not delete non-activ contract w id:" << id;
        return false;
    }
    // abschluss Wert berechnen
    double davonZins =ZinsesZins(Zinsfuss(), Wert(), StartZinsberechnung(), termin, Thesaurierend());
    wert += davonZins;

    QSqlDatabase::database().transaction();
    QSqlQuery deleteQ;
    if( !deleteQ.exec("DELETE FROM Vertraege WHERE id=" + QString::number(id)))
    {
        qCritical() << "failed to delete active Contract: " << deleteQ.lastError() << endl << deleteQ.lastQuery();
        QSqlDatabase::database().rollback();
        return false;
    }
    QSqlDatabase::database().commit();
    qDebug() << "aktiver Vertrag " << id << "beendet";
    return true;


}

bool Contract::saveAnnualPayment(const QDate& end)
{   LOG_CALL;
    letzteZinsgutschrift = ZinsesZins(Zinsfuss(), thesaurierend?Wert():Betrag(), StartZinsberechnung(), end, thesaurierend);
    qDebug() << "JA: berechneter Zins: " << letzteZinsgutschrift;

    double neuerWert =  Thesaurierend() ? auf2Stellen(wert +letzteZinsgutschrift) : 0.;
    qDebug() << "JA: aktualisierter Wert: " << neuerWert;

    QString where = "id = " + QString::number(id);
    bool ret =true;
    ret &= -1 != ExecuteUpdateSql( "Vertraege", "Wert", QVariant(neuerWert), where);
    if( ret) wert = neuerWert;       // update this contract obj. in Memory

    ret &= -1 != ExecuteUpdateSql( "Vertraege", "LetzteZinsberechnung", QVariant(end), where);
    if( ret) startZinsberechnung = end;       // update this contract obj. in Memory

    return ret;
}

bool Contract::bookAnnualInterest(const QDate& end)
{   LOG_CALL_W( QString::number(end.year()));
    if( end < StartZinsberechnung())
    {   // Verträge, für die der Jahresabschluß bereits gemacht worden ist
        qDebug() << "Begin der Zinsberechnung ist NACH dem Jahresabschlussdatum -> keine Abrechnung";
        return false;
    }
    QSqlDatabase::database().transaction();
    if( !saveAnnualPayment(end))
    {
        qCritical() << "Jahresabschluss wurde nicht gespeichert";
        QSqlDatabase::database().rollback();
        return false;
    }

    QSqlDatabase::database().commit();
    // Briefe Drucken
    return true;
}
