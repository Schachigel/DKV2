#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QtCore>

#include "dkdbhelper.h"

bool createTables( const QSqlDatabase& db)
{

    QSqlQuery q(db);
    bool ret (q.exec(sqls_createDkGeber));
    ret &= q.exec(sqls_createDkVertrag);
    ret &= q.exec(sqls_createBuchungen);
    ret &= q.exec(sqls_createZinssaetze);
    return ret;
}

bool insertInterestRates(const QSqlDatabase& db)
{
    QString sqlZinssaetze ("INSERT INTO DKZinssaetze (Zinssatz, Bemerkung) VALUES ");

    sqlZinssaetze += "(" + QString::number(0.) + ",'Helden')";
    for (double zins=0.1; zins < .6; zins+=0.1)
        sqlZinssaetze += ", (" + QString::number(zins) + ",'Freunde')";
    for (double zins=.6; zins < 1.1; zins+=0.1)
        sqlZinssaetze += ", (" + QString::number(zins) + ",'Förderer')";
    for (double zins=1.1; zins < 2.; zins+=0.1)
        sqlZinssaetze += ", (" + QString::number(zins) + ",'Investoren')";

    QSqlQuery q(db);
    return q.exec(sqlZinssaetze);
}

bool createDKDB(const QString& filename)
{
    if( QFile(filename).exists())
    {
        QFile::remove(filename +".old");
        if( !QFile::rename(filename, filename +".old"))
            return false;
    }
    bool ret = true;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(filename);
    ret &= db.open();
    if( !ret) return ret;

    dbCloser c(&db);
    ret &= createTables(db);
    ret &= insertInterestRates(db);
    return ret;
}
