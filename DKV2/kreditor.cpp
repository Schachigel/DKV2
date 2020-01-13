#include <QSqlRecord>
#include <QRegularExpression>
#include "finhelper.h"
#include "helper.h"
#include "sqlhelper.h"
#include "kreditor.h"

bool Kreditor::fromDb( int i)
{   LOG_ENTRY_and_EXIT;

    QSqlRecord rec = ExecuteSingleRecordSql(dkdbstructur["Kreditoren"].Fields(), "id="+QString::number(i));
    if( rec.isEmpty()) return false;
    for(int i=0; i<rec.count(); i++)
    {
        ti.setValue(rec.field(i).name(), rec.field(i).value());
    }
    return true;
}

bool Kreditor::isValid( QString& errortext)
{
    errortext.clear();
    if( (ti.getValue("Vorname").toString().isEmpty() && ti.getValue("Vorname").toString().isEmpty())
         ||
        ti.getValue("Strasse").toString().isEmpty()
         ||
        ti.getValue("Plz").toString().isEmpty()
         ||
        ti.getValue("Stadt").toString().isEmpty())
        errortext = "Die Adressdaten sind unvollständig";

    QString email = ti.getValue("Email").toString();
    if( !email.isEmpty() || email == "NULL_STRING")
    {
        QRegularExpression rx("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b",
                              QRegularExpression::CaseInsensitiveOption);
        if( !rx.match(email).hasMatch())
            errortext = "Das Format der e-mail Adresse ist ungültig";
    }

    IbanValidator iv; int pos = 0;
    QString iban = ti.getValue("IBAN").toString();
    if( !iban.isEmpty())
        if( iv.validate(iban, pos) != IbanValidator::State::Acceptable)
            errortext = "Das Format der IBAN ist nicht korrekt";

    if( errortext.isEmpty())
        return true;
    return false;
}

void Kreditor::setValue(const QString& n, const QVariant& v)
{
    ti.setValue(n, v);
}

void Kreditor::setUniqueDbValue(const QString& n, const QVariant& v)
{
    if( v.isNull() || !v.isValid() ||
        (v.type() == QVariant::Type::String && v.toString().isEmpty()))
        ti.setValue(n, QVariant(QVariant::String));
    else
        ti.setValue(n, v);
}

int Kreditor::Speichern(QSqlDatabase db) const
{   LOG_ENTRY_and_EXIT;

    if( ti.getRecord().isEmpty())
        return -1;
    return ti.InsertData(db);
}

int Kreditor::Update(QSqlDatabase db) const
{LOG_ENTRY_and_EXIT;
    if( ti.getRecord().isEmpty())
        return -1;
    return ti.UpdateData(db);
}

/* static */ bool Kreditor::Loeschen(int index)
{   LOG_ENTRY_and_EXIT;

    // referential integrity will delete the contracts
    QSqlQuery deleteQ;
    if( !deleteQ.exec("DELETE FROM [Kreditoren] WHERE [Id]=" +QString::number(index)))
    {
        qCritical() << "Delete Kreditor failed "<< deleteQ.lastError() << endl << deleteQ.lastQuery();
        return false;
    }
    else
        return true;
}

void Kreditor::KreditorenMitId(QList<QPair<int,QString>> &entries) const
{

    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare("SELECT id, Vorname, Nachname, Plz, Stadt, Strasse FROM Kreditoren ORDER BY Nachname ASC, Vorname ASC");
    if( !query.exec())
    {
        qCritical() << "Error reading DKGeber while creating a contract: " << QSqlDatabase::database().lastError().text();
    }

    while(query.next())
    {
        QString Entry = query.value("Nachname").toString() + QString(", ") + query.value("Vorname").toString();
        Entry += QString(", ") + query.value("Plz").toString() + "-" +query.value("Stadt").toString();
        Entry += QString(", ") + query.value("Strasse").toString();
        QList<QPair<int,QString>> entry {{query.value("id").toInt(), Entry}};
        entries.append(entry);
    }
}
