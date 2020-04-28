#ifndef KREDITOR_H
#define KREDITOR_H

#include <QString>
#include <QPair>
#include <QList>

#include "dkdbhelper.h"

class Kreditor
{
public:
    // constructors
    Kreditor(){ ti.init(dkdbstructur["Kreditoren"]);}
    Kreditor (int i){ fromDb(i);}
    // interface
    bool fromDb(int id);
    void setUniqueDbValue(const QString& n, const QVariant& v);
    void setValue(const QString& n, const QVariant& v);
    QVariant getValue(const QString& f) const { return ti.getValue(f);}
    bool isValid( QString& errortext);
    int Speichern(QSqlDatabase db=QSqlDatabase::database()) const;
    int Update(QSqlDatabase db=QSqlDatabase::database()) const;
    void KreditorenListeMitId(QList<QPair<int,QString>> &entries) const;
    static bool Loeschen(int index);
private:
    TableDataInserter ti;
//    QSqlRecord rec;
};

#endif // KREDITOR_H
