#include <QSqlRecord>
#include <QRegularExpression>
#include "finhelper.h"
#include "helper.h"
#include "sqlhelper.h"
#include "creditor.h"

bool creditor::fromDb( int i)
{   LOG_CALL;

    QSqlRecord rec = ExecuteSingleRecordSql(dkdbstructur["Kreditoren"].Fields(), "id="+QString::number(i));
    if( rec.isEmpty()) return false;
    for(int i=0; i<rec.count(); i++)
    {
        qDebug() << "reading Kreditor from db; Field:" << rec.field(i).name() << "-value:" << rec.field(i).value() << "(" << rec.field(i).value().type() << ")";
        if( dkdbstructur["Kreditoren"][rec.field(i).name()].type() == QVariant::Type::String)
            ti.setValue(rec.field(i).name(), rec.field(i).value().toString());
        else if( dkdbstructur["Kreditoren"][rec.field(i).name()].type() == QVariant::Type::LongLong)
            ti.setValue(rec.field(i).name(), rec.field(i).value().toLongLong());
        else if( dkdbstructur["Kreditoren"][rec.field(i).name()].type() == QVariant::Type::Double)
            ti.setValue(rec.field(i).name(), rec.field(i).value().toDouble());
        else
            ti.setValue(rec.field(i).name(), rec.field(i).value());
    }
    return true;
}

bool creditor::isValid( QString& errortext)
{   LOG_CALL;
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

void creditor::setValue(const QString& n, const QVariant& v)
{   LOG_CALL_W(n);
    ti.setValue(n, v);
}

void creditor::setUniqueDbValue(const QString& n, const QVariant& v)
{   LOG_CALL_W(n);
    if( v.isNull() || !v.isValid() ||
        (v.type() == QVariant::Type::String && v.toString().isEmpty()))
        ti.setValue(n, QVariant(QVariant::String));
    else
        ti.setValue(n, v);
}

int creditor::Speichern(QSqlDatabase db) const
{   LOG_CALL;

    if( ti.getRecord().isEmpty())
        return -1;
    return ti.InsertData(db);
}

int creditor::Update(QSqlDatabase db) const
{   LOG_CALL;
    if( ti.getRecord().isEmpty())
        return -1;
    return ti.UpdateData(db);
}

/* static */ bool creditor::Loeschen(int index)
{   LOG_CALL;
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

void creditor::KreditorenListeMitId(QList<QPair<int,QString>> &entries) const
{   LOG_CALL;
    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare("SELECT id, Vorname, Nachname, Plz, Stadt, Strasse FROM Kreditoren ORDER BY Nachname ASC, Vorname ASC");
    if( !query.exec())
    {
        qCritical() << "Error reading DKGeber while creating a contract: " << QSqlDatabase::database().lastError().text();
        return;
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
