#ifndef DKDBHELPER_H
#define DKDBHELPER_H
#include <QDebug>
#include <QSqlDatabase>
#include <QVariant>
#include <QDateTime>
#include <QList>
#include <QString>


#include "dbtable.h"
#include "dbfield.h"
#include "dbstructure.h"
#include "sqlhelper.h"

extern dbstructure dkdbstructur;
extern dbstructure dkdbAddtionalTables;

extern QList<QPair<qlonglong, QString>> Buchungsarten;

void init_DKDBStruct();

bool create_DK_databaseFile(const QString& filename);
bool create_DK_databaseContent(QSqlDatabase db=defaultDb());


bool check_db_version(QSqlDatabase db=defaultDb());
bool isValidDatabase(const QString& filename);
bool isValidDatabase(QSqlDatabase db=defaultDb());

void closeDatabaseConnection(QString connection= QSqlDatabase::defaultConnection);
bool open_databaseForApplication( QString newDbFile="");
QStringList check_DbConsistency( );

bool create_DB_copy(QString targetfn, bool anonym);

QString proposeKennung(QSqlDatabase db=defaultDb());
void create_sampleData(int datensaetze =20);

typedef QPair<int, QString> ZinsAnzeigeMitId;
void interestRates_for_dropdown(QList<ZinsAnzeigeMitId>& Entries);

int BuchungsartIdFromArt(QString s);

bool VertragAktivieren( int ContractId, const QDate& activationDate);
bool passivenVertragLoeschen(const QString& index);
bool VertragsdatenZurLoeschung(const int index, const QDate endD, double& neuerWert, double& davonZins);

QString contractList_SELECT(QVector<dbfield>& f);
QString contractList_FROM();
QString contractList_WHERE(const QString& filter);
QString contractList_SQL(const QVector<dbfield>& f, const QString& filter);

void initMetaInfo( const QString& name, const QString& wert, QSqlDatabase db = defaultDb());
void initNumMetaInfo( const QString& name, const double& wert, QSqlDatabase db = defaultDb());
QString getMetaInfo(const QString& name,QSqlDatabase db = defaultDb());
double getNumMetaInfo(const QString& name, QSqlDatabase db = defaultDb());
void setMetaInfo(const QString& name, const QString& value, QSqlDatabase db = defaultDb());
void setNumMetaInfo(const QString& name, const double Wert, QSqlDatabase db = defaultDb());

bool createCsvActiveContracts();

struct DbSummary
{
    int AnzahlDkGeber;
    int AnzahlDkGeberEin;
    int AnzahlDkGeberZwei;
    int AnzahlDkGeberMehr;
    double DurchschnittZins;
    double MittlererZins;

    int     AnzahlAktive;
    double  BetragAktive;
    double  WertAktive;

    int     AnzahlAuszahlende;
    double  BetragAuszahlende;

    int     AnzahlThesaurierende;
    double  WertThesaurierende;
    double  BetragThesaurierende;

    int     AnzahlPassive;
    double  BetragPassive;
};

struct ContractEnd
{
    int year;
    int count;
    double value;
};
void calc_contractEnd(QVector<ContractEnd>& ce, QSqlDatabase db = defaultDb());
void calculateSummary(DbSummary& dbs, QSqlDatabase db = defaultDb());

struct YZV
{
    int year;
    double intrest;
    int count;
    double sum;
};
void calc_anualInterestDistribution( QVector<YZV>& yzv, QSqlDatabase db = defaultDb());
struct rowData
{
    QString text; QString number; QString value;
};

QVector<rowData> contractRuntimeDistribution(QSqlDatabase db = defaultDb());

#endif // DKDBHELPER_H
