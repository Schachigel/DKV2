#include <QVector>

#include "helper.h"
#include "dbfield.h"
#include "sqlhelper.h"
#include "finhelper.h"
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
        dkGeber.setValue("Strasse", rec.value("Kreditoren.Strasse"));
        dkGeber.setValue("Plz", rec.value("Kreditoren.Plz").toString());
        dkGeber.setValue("Stadt", rec.value("Kreditoren.Stadt"));
        dkGeber.setValue("Iban", rec.value("Kreditoren.IBAN"));
        dkGeber.setValue("Bic", rec.value("Kreditoren.BIC"));
    }
    return true;
}

bool Vertrag::BelegSpeichern(const int BArt, const QString& msg)
{   LOG_ENTRY_and_EXIT;

    updateAusDb();
    TableDataInserter ti(dkdbstructur["Buchungen"]);
    ti.setValue("VertragId", id);
    ti.setValue("Buchungsart", BArt);
    ti.setValue("Betrag", betrag);
    ti.setValue("Datum", QDate::currentDate());
    ti.setValue("Bemerkung", msg);
    ti.setValue("Buchungsdaten", buchungsdatenJson);
    ti.InsertData();

    qDebug().noquote() << msg << endl << buchungsdatenJson;
    return true;
}

int Vertrag::speichereNeuenVertrag() const
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

    return BelegSpeichern(VERTRAG_ANLEGEN, msg);
}

bool Vertrag::verbucheNeuenVertrag()
{   LOG_ENTRY_and_EXIT;

    QSqlDatabase::database().transaction();
    int nextId =speichereNeuenVertrag();
    if( nextId>0 )
    {
        setVid( nextId);
        if( speichereBelegNeuerVertrag())
        {
            QSqlDatabase::database().commit();
            return true;
        }
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
    ret &= BelegSpeichern(VERTRAG_AKTIVIEREN, BelegNachricht);

    if(ret)
        QSqlDatabase::database().commit();
    else
    {
        qDebug() << "Fehler beim aktivieren eines Vertrags";
        QSqlDatabase::database().rollback();
    }
    return ret;
}

bool Vertrag::passivenVertragLoeschen()
{   LOG_ENTRY_and_EXIT;
    if( ExecuteSingleValueSql("SELECT [aktiv] FROM [Vertraege] WHERE id=" +QString::number(id)).toBool())
    {
        qWarning() << "will not delete active contract w id:" << id;
        return false;
    }

    QSqlDatabase::database().transaction();
    // wg der ref. integrit. muss ERST die Buchung gemacht werden, dann der Vertrag gelöscht
    QString Belegnachricht("Passiver Vertrag %1 gelöscht");
    Belegnachricht = Belegnachricht.arg(QString::number(id));
    if( !BelegSpeichern( PASSIVEN_VERTRAG_LOESCHEN, Belegnachricht))
    {
        qCritical() << "Belegdaten konnten nicht gespeichert werden";
        QSqlDatabase::database().rollback();
        return false;
    }
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

bool Vertrag::aktivenVertragLoeschen( const QDate& termin)
{LOG_ENTRY_and_EXIT;
    if( !ExecuteSingleValueSql("SELECT [aktiv] FROM [Vertraege] WHERE id=" +QString::number(id)).toBool())
    {
        qWarning() << "will not delete non-activ contract w id:" << id;
        return false;
    }
    // abschluss Wert berechnen
    double davonZins =ZinsesZins(Zinsfuss(), Wert(), StartZinsberechnung(), termin, Tesaurierend());
    wert += davonZins;

    QSqlDatabase::database().transaction();
    QString Belegnachricht("Aktiven Vertrag " + QString::number(id) + " beenden. ");
    Belegnachricht += QString::number(Wert()) + "Euro (" + QString::number(davonZins) + "Euro Zins)";
    if( !BelegSpeichern(VERTRAG_BEENDEN, Belegnachricht))
    {
        qCritical() << "Belegdaten konnten nicht gespeichert werden";
        QSqlDatabase::database().rollback();
        return false;
    }

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

bool Vertrag::speichereJahresabschluss(const QDate& end)
{
    letzteZinsgutschrift = ZinsesZins(Zinsfuss(), Wert(), StartZinsberechnung(), end);
    double neuerWert = 0.;
    bool ret =true;
    QString where = "id = " + QString::number(id);
    if( Tesaurierend())
    {
        neuerWert = round(wert +letzteZinsgutschrift);
        ret &= ExecuteUpdateSql( "Vertraege", "Wert", QVariant(neuerWert), where);
        if( ret)
            wert = neuerWert;
    }
    ret &= ExecuteUpdateSql( "Vertraege", "LetzteZinsberechnung", QVariant(end), where);

    if( ret)
    {   // update this contract in Memory
        startZinsberechnung = end;
    }

    return ret;
}

bool Vertrag::speichereBelegJahresabschluss(const QDate &end)
{
    QString msg = QString::number(letzteZinsgutschrift) + " Euro "
                    "Zinsgutschrift zum " + end.toString();
    return BelegSpeichern(ZINSGUTSCHRIFT, msg);
}

bool Vertrag::verbucheJahreszins(const QDate& end)
{
    if( end < StartZinsberechnung())
    {
        qDebug() << "Begin der Zinsberechnung ist NACH dem Jahresabschlussdatum -> keine Abrechnung";
        return false;
    }
    QSqlDatabase::database().transaction();
    if( !speichereJahresabschluss(end))
    {
        qCritical() << "Jahresabschluss wurde nicht gespeichert";
        QSqlDatabase::database().rollback();
        return false;
    }
    if( !speichereBelegJahresabschluss(end))
    {
        qCritical() << "Jahresabschluss Beleg wurde nicht gespeichert";
        QSqlDatabase::database().rollback();
        return false;
    }
    QSqlDatabase::database().commit();
    // Briefe Drucken
    return true;

}
