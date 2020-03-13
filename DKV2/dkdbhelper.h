#ifndef DKDBHELPER_H
#define DKDBHELPER_H
#include <qdebug.h>
#include <QSqlDatabase>

#include <QVariant>
#include <QDateTime>
#include <QList>
#include <QString>


#include "dbtable.h"
#include "dbfield.h"
#include "dbstructure.h"

extern dbstructure dkdbstructur;
extern dbstructure dkdbAddtionalTables;

extern QList<QPair<qlonglong, QString>> Buchungsarten;

 enum Buchungsart_i
{
    NOOP =0,
    VERTRAG_ANLEGEN =1,
    VERTRAG_AKTIVIEREN =2,
    PASSIVEN_VERTRAG_LOESCHEN =3,
    VERTRAG_BEENDEN =4,
    ZINSGUTSCHRIFT =5,
    KUENDIGUNG_FRIST =6,
    BART_NEXT =7
};

void init_DKDBStruct();
void init_additionalTables();

void init_bookingTypes();

bool create_DK_database(const QString& filename);
bool create_DK_database(QSqlDatabase db=QSqlDatabase::database());

bool isValidDatabase(const QString& filename);
bool isValidDatabase(QSqlDatabase db=QSqlDatabase::database());

void closeDatabaseConnection(QString connection= QLatin1String(QSqlDatabase::defaultConnection));
void open_databaseForApplication( QString newDbFile="");
QStringList check_DbConsistency( );

bool ensureTable(const dbtable& table, const QString& connection= QLatin1String(QSqlDatabase::defaultConnection));
bool ensureTable(const dbtable& table, QSqlDatabase& db);

bool create_DB_copy(QString targetfn, bool anonym);

QString proposeKennung();
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

void init_property( const QString& name, const QString& wert, const QString& connection=QLatin1String(QSqlDatabase::defaultConnection));
QString getProperty(const QString& name, const QString& connection=QLatin1String(QSqlDatabase::defaultConnection));
void setProperty(const QString& name, const QString& value, const QString& connection=QLatin1String(QSqlDatabase::defaultConnection));

void CsvActiveContracts();

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
void calc_contractEnd(QVector<ContractEnd>& ce, QString connection=QLatin1String(QSqlDatabase::defaultConnection));
void calculateSummary(DbSummary& dbs, QString connection=QLatin1String(QSqlDatabase::defaultConnection));

struct YZV
{
    int year;
    double intrest;
    int count;
    double sum;
};
void calc_anualInterestDistribution( QVector<YZV>& yzv, QString connection=QLatin1String(QSqlDatabase::defaultConnection));

QString contractTerm_distirbution_html(QString connection=QLatin1String(QSqlDatabase::defaultConnection));

#endif // DKDBHELPER_H
