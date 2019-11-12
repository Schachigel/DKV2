#include <QSqlRecord>

#include "helper.h"
#include "sqlhelper.h"
#include "kreditor.h"

bool Kreditor::fromDb( int i)
{   LOG_ENTRY_and_EXIT;

    QSqlRecord rec = ExecuteSingleRecordSql(kTable.Fields(), "id="+QString::number(i));
    if( rec.isEmpty()) return false;
    for(int i=0; i<rec.count(); i++)
    {
        ti.setValue(rec.field(i).name(), rec.field(1).value());
    }
    return true;
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
    query.prepare("SELECT id, Vorname, Nachname, Plz, Strasse FROM Kreditoren ORDER BY Nachname ASC, Vorname ASC");
    if( !query.exec())
    {
        qCritical() << "Error reading DKGeber while creating a contract: " << QSqlDatabase::database().lastError().text();
    }

    while(query.next())
    {
        QString Entry = query.value("Nachname").toString() + QString(", ") + query.value("Vorname").toString() + QString(", ") + query.value("Plz").toString();
        Entry += QString(", ") + query.value("Strasse").toString();
        QList<QPair<int,QString>> entry {{query.value("id").toInt(), Entry}};
        entries.append(entry);
    }
}
