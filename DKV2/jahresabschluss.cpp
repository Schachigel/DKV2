#include <QSqlQuery>
#include <QSqlError>

#include "helper.h"
#include "helpersql.h"
#include "contract.h"
#include "jahresabschluss.h"

jahresabschluss::jahresabschluss()
{
    Jahr = JahreszahlFuerAbschluss();
}

int jahresabschluss::JahreszahlFuerAbschluss()
{   LOG_CALL;
    QDate aeltesteZinszahlung = executeSingleValueSql("SELECT min(LetzteZinsberechnung) FROM Vertraege WHERE aktiv != 0").toDate();
    if( aeltesteZinszahlung.month()==12 && aeltesteZinszahlung.day() == 31)
        return aeltesteZinszahlung.year() +1;
    return aeltesteZinszahlung.year();
}

bool jahresabschluss::execute()
{   LOG_CALL;
    QSqlQuery sql;
    sql.prepare("SELECT Vertraege.id FROM Vertraege WHERE aktiv != 0");
    if( !(sql.exec() && sql.first()))
    {
        qCritical() << "faild to select contracts: " << sql.lastError() << endl << "in " << sql.lastQuery();
        return false;
    }
    // const QDate YearEnd= QDate(Jahr, 12, 31);
    do
    {
Q_ASSERT(!"repair");
//        Contract v; v.loadContractFromDb(sqlVal<int>(sql, "id"));
//        if( v.bookAnnualInterest(YearEnd))
//        {
//            if( v.Thesaurierend())
//                thesaV.push_back(v);
//            else
//                n_thesaV.append(v);
//        }

    } while(sql.next());

    return true;
}
