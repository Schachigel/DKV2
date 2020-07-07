#ifndef DKDBHELPER_H
#define DKDBHELPER_H

#include <QSqlDatabase>

#include <QDateTime>
#include <QList>
#include <QString>


#include "dbtable.h"
#include "dbfield.h"
#include "dbstructure.h"
#include "helpersql.h"

extern dbstructure dkdbstructur;
// THE structure of our database the single source of truth
void init_DKDBStruct();

bool create_DK_databaseFile(const QString& filename);
bool create_DK_TablesAndContent(QSqlDatabase db = QSqlDatabase::database());
bool createView(QString name, QString sql, QSqlDatabase db =QSqlDatabase::database());
bool insert_views( QSqlDatabase db =QSqlDatabase::database());

bool check_db_version(QSqlDatabase db = QSqlDatabase::database());
bool isValidDatabase(const QString& filename);
bool isValidDatabase(QSqlDatabase db = QSqlDatabase::database());

void closeDatabaseConnection(QString connection= QSqlDatabase::defaultConnection);
bool open_databaseForApplication( QString newDbFile="");

bool create_DB_copy(QString targetfn, bool anonym);

QString proposeKennung();
void create_sampleData(int datensaetze =20);

int BuchungsartIdFromArt(QString s);

bool VertragAktivieren( int ContractId, const QDate& activationDate);
bool passivenVertragLoeschen(const QString& index);
bool VertragsdatenZurLoeschung(const int index, const QDate endD, double& neuerWert, double& davonZins);

QString contractList_SELECT(QVector<dbfield>& f);
QString contractList_FROM();
QString contractList_WHERE(const QString& filter);
QString contractList_SQL(const QVector<dbfield>& f, const QString& filter);

bool createCsvActiveContracts();

struct DbSummary
{
    int AnzahlDkGeber;
    double AnzahlDkProDkGeber;

    double DurchschnittZins;
    double MittlererZins;

    int     AnzahlAktive;
    double  WertAktive;

    int     AnzahlAuszahlende;
    double  BetragAuszahlende;

    int     AnzahlThesaurierende;
    double  WertThesaurierende;

    int     AnzahlPassive;
    double  BetragPassive;
};

struct ContractEnd
{
    int year;
    int count;
    double value;
};
void calc_contractEnd(QVector<ContractEnd>& ce);
DbSummary calculateSummary();

struct YZV
{
    int year;
    double intrest;
    int count;
    double sum;
};
void calc_anualInterestDistribution( QVector<YZV>& yzv);
struct rowData
{
    QString text; QString number; QString value;
};

QVector<rowData> contractRuntimeDistribution();

#endif // DKDBHELPER_H
