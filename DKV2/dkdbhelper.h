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

void initDKDBStruktur();
void initAdditionalTables();

void initBuchungsarten();

bool DKDatenbankAnlegen(const QString& filename);
bool DKDatenbankAnlegen(QSqlDatabase db=QSqlDatabase::database());

bool istValideDatenbank(const QString& filename);
bool istValideDatenbank(QSqlDatabase db=QSqlDatabase::database());

void DatenbankverbindungSchliessen(QString connection= QLatin1String(QSqlDatabase::defaultConnection));
void DatenbankZurAnwendungOeffnen( QString newDbFile="");
void CheckDbConsistency( QStringList& msg);

bool ensureTable(const dbtable& table, const QString& connection= QLatin1String(QSqlDatabase::defaultConnection));
bool ensureTable(const dbtable& table, QSqlDatabase& db);

bool createDbCopy(QString targetfn, bool anonym);

QString ProposeKennung();
void BeispieldatenAnlegen(int datensaetze =20);

typedef QPair<int, QString> ZinsAnzeigeMitId;
void ZinssaetzeFuerAuswahlliste(QList<ZinsAnzeigeMitId>& Entries);

int BuchungsartIdFromArt(QString s);

bool VertragAktivieren( int ContractId, const QDate& activationDate);
bool passivenVertragLoeschen(const QString& index);
bool VertragsdatenZurLoeschung(const int index, const QDate endD, double& neuerWert, double& davonZins);

QString ContractList_SELECT(QVector<dbfield>& f);
QString ContractList_FROM();
QString ContractList_WHERE(const QString& filter);
QString ContractList_SQL(const QVector<dbfield>& f, const QString& filter);

void initProperty( const QString& name, const QString& wert, const QString& connection=QLatin1String(QSqlDatabase::defaultConnection));
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
void berechneVertragsenden(QVector<ContractEnd>& ce, QString connection=QLatin1String(QSqlDatabase::defaultConnection));
void berechneZusammenfassung(DbSummary& dbs, QString connection=QLatin1String(QSqlDatabase::defaultConnection));

struct YZV
{
    int year;
    double intrest;
    int count;
    double sum;
};
void berechneJahrZinsVerteilung( QVector<YZV>& yzv, QString connection=QLatin1String(QSqlDatabase::defaultConnection));

QString LaufzeitenVerteilungHtml(QString connection=QLatin1String(QSqlDatabase::defaultConnection));

#endif // DKDBHELPER_H
