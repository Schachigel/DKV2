#include <QVector>

#include "helper.h"
#include "dbfield.h"
#include "sqlhelper.h"
#include "dkdbhelper.h"
#include "vertrag.h"

bool Vertrag::ausDb(int vId, bool mitBelegdaten)
{   LOG_ENTRY_and_EXIT;
    QVector<dbfield>fields;
    fields.append(dkdbstructur["Vertraege"]["id"]);
    fields.append(dkdbstructur["Vertraege"]["KreditorId"]);
    fields.append(dkdbstructur["Vertraege"]["Kennung"]);
    fields.append(dkdbstructur["Vertraege"]["Betrag"]);
    fields.append(dkdbstructur["Vertraege"]["Wert"]);
    fields.append(dkdbstructur["Vertraege"]["ZSatz"]);
    fields.append(dkdbstructur["Vertraege"]["tesaurierend"]);
    fields.append(dkdbstructur["Vertraege"]["Vertragsdatum"]);
    fields.append(dkdbstructur["Vertraege"]["aktiv"]);
    fields.append(dkdbstructur["Vertraege"]["LaufzeitEnde"]);
    fields.append(dkdbstructur["Vertraege"]["LetzteZinsberechnung"]);
    if( mitBelegdaten)
    {
        fields.append(dkdbstructur["Kreditoren"]["id"]);
        fields.append(dkdbstructur["Kreditoren"]["Vorname"]);
        fields.append(dkdbstructur["Kreditoren"]["Nachname"]);
        fields.append(dkdbstructur["Kreditoren"]["Strasse"]);
        fields.append(dkdbstructur["Kreditoren"]["Plz"]);
        fields.append(dkdbstructur["Kreditoren"]["Stadt"]);
        fields.append(dkdbstructur["Kreditoren"]["IBAN"]);
        fields.append(dkdbstructur["Kreditoren"]["BIC"]);
        fields.append(dkdbstructur["Zinssaetze"]["id"]);
        fields.append(dkdbstructur["Zinssaetze"]["Zinssatz"]);
    }

    QSqlRecord rec = ExecuteSingleRecordSql(fields, "[Vertraege].[id]=" +QString::number(vId));
    if( rec.isEmpty())
        return false;
    id                  = rec.value("Vertraege.id").toInt();
    kreditorId          = rec.value("KreditorId").toInt();
    betrag              = rec.value("Betrag").toDouble();
    wert                = rec.value("Wert").toDouble();
    tesaurierend        = rec.value("tesaurierend").toBool();
    active              = rec.value("aktiv").toBool();
    vertragsdatum       = rec.value("Vertragsdatum").toDate();
    laufzeitEnde        = rec.value("LaufzeitEnde").toDate();
    startZinsberechnung = rec.value("LetzteZinsberechnung").toDate();
    if( mitBelegdaten)
    {
        buchungsdatenJson = JsonFromRecord(rec);
        zinsFuss = rec.value("Zinssaetze.Zinssatz").toDouble();
        dkGeber.setValue("Vorname", rec.value("Kreditoren.Vorname"));
        dkGeber.setValue("Nachname", rec.value("Kreditoren.Nachname"));
        dkGeber.setValue("Strasse ", rec.value("Kreditoren.Strasse"));
        dkGeber.setValue("Plz", rec.value("Kreditoren.Plz"));
        dkGeber.setValue("Stadt", rec.value("Kreditoren.Stadt"));
        dkGeber.setValue("Iban", rec.value("Kreditoren.IBAN"));
        dkGeber.setValue("Bic", rec.value("Kreditoren.BIC"));
    }
    return true;
}

bool Vertrag::BelegSpeichern(int BArt, QString msg)
{   LOG_ENTRY_and_EXIT;
    updateAusDb();
    QSqlQuery sqlBuchung;
    sqlBuchung.prepare("INSERT INTO Buchungen (VertragId, Buchungsart, Betrag, Datum, Bemerkung, Buchungsdaten)"
                       " VALUES (:VertragsId, :Buchungsart, :Betrag, :Datum, :Bemerkung, :Buchungsdaten)");
    sqlBuchung.bindValue(":VertragsId", QVariant(id));
    sqlBuchung.bindValue(":Buchungsart", BArt);
    sqlBuchung.bindValue(":Betrag", QVariant(betrag));
    sqlBuchung.bindValue(":Datum", QVariant(QDate::currentDate()));
    sqlBuchung.bindValue(":Bemerkung", QVariant(msg));
    sqlBuchung.bindValue(":Buchungsdaten", QVariant(buchungsdatenJson));
    if( !sqlBuchung.exec())
    {
        qCritical() << "Buchung wurde nicht gesp. Fehler: " << sqlBuchung.lastError();
        return false;
    }
    qDebug().noquote() << msg << "\n" << buchungsdatenJson;
    return true;
}

int Vertrag::speichereNeuenVertrag()
{  LOG_ENTRY_and_EXIT;
    TableDataInserter ti(dkdbstructur["Vertraege"]);
    ti.setValue(dkdbstructur["Vertraege"]["KreditorId"].name(), kreditorId);
    ti.setValue(dkdbstructur["Vertraege"]["Kennung"].name(), kennung);
    ti.setValue(dkdbstructur["Vertraege"]["Betrag"].name(), betrag);
    ti.setValue(dkdbstructur["Vertraege"]["Wert"].name(), wert);
    ti.setValue(dkdbstructur["Vertraege"]["ZSatz"].name(), zinsId);
    ti.setValue(dkdbstructur["Vertraege"]["tesaurierend"].name(), tesaurierend);
    ti.setValue(dkdbstructur["Vertraege"]["Vertragsdatum"].name(), vertragsdatum);
    ti.setValue(dkdbstructur["Vertraege"]["aktiv"].name(), active);
    ti.setValue(dkdbstructur["Vertraege"]["LaufzeitEnde"].name(), laufzeitEnde);
    ti.setValue(dkdbstructur["Vertraege"]["LetzteZinsberechnung"].name(), startZinsberechnung);
    int lastid =ti.InsertData(QSqlDatabase::database());
    if( lastid <0) return -1;
    qDebug() << "Neuer Vertrag wurde eingefügt mit id:" << lastid;
    return lastid;
}

bool Vertrag::speichereBelegNeuerVertrag()
{
    if( buchungsdatenJson.isEmpty())
    {
        ausDb(id, true);
    }
    QString msg("Neuer Vertrag #" +QString::number(id));

    return BelegSpeichern(BuchungsartIdFromArt("Vertrag anlegen"), msg);
}

bool Vertrag::verbucheNeuenVertrag()
{   LOG_ENTRY_and_EXIT;

    QSqlDatabase::database().transaction();
    setVid( speichereNeuenVertrag());
    if( id>0 )
        if( speichereBelegNeuerVertrag())
        {
            QSqlDatabase::database().commit();
            return true;
        }
    qCritical() << "ein neuer Vertrag konnte nicht gespeichert werden";
    QSqlDatabase::database().rollback();
    return false;
}

bool Vertrag::aktiviereVertrag(const QDate& aDate)
{   LOG_ENTRY_and_EXIT;
    QSqlDatabase::database().transaction();

    QSqlQuery updateQ;
    updateQ.prepare("UPDATE Vertraege SET LetzteZinsberechnung = :vdate, aktiv = :true WHERE id = :id");
    updateQ.bindValue(":vdate",QVariant(aDate));
    updateQ.bindValue(":id", QVariant(id));
    updateQ.bindValue(":true", QVariant(true));
    bool ret = updateQ.exec();
    qDebug() << updateQ.lastQuery() << updateQ.lastError();
    if( ret)
    {
        active = true;
        startZinsberechnung = aDate;
    }

    QString BelegNachricht ("Vertrag %1 aktiviert zum %2");
    BelegNachricht = BelegNachricht.arg(QString::number(id), aDate.toString());
    ret &= BelegSpeichern(BuchungsartIdFromArt("Vertrag aktivieren"), BelegNachricht); // BELEG BUCHEN ToDo

    if(ret)
        QSqlDatabase::database().commit();
    else
    {
        qDebug() << "Fehler beim aktivieren eines Vertrags";
        QSqlDatabase::database().rollback();
    }
    return ret;
}

/*static*/ bool Vertrag::passivenVertragLoeschen(const int vId)
{   LOG_ENTRY_and_EXIT;
    if( ExecuteSingleValueSql("SELECT [aktiv] FROM [Vertraege] WHERE id=" +QString::number(vId)).toBool())
    {
        qWarning() << "will not delete active contract w id:" << QString::number(vId);
        return false;
    }
    QSqlQuery deleteQ;
    if( !deleteQ.exec("DELETE FROM Vertraege WHERE id=" + QString::number(vId)))
    {
        qCritical() << "failed to delete Contract: " << deleteQ.lastError() << "\n" << deleteQ.lastQuery();
        return false;
    }
    return true;
}
