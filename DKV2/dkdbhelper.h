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
    BART_NEXT =6
};

void initDKDBStruktur();
void initAdditionalTables();

void initBuchungsarten();

bool DKDatenbankAnlegen(const QString& filename);
bool DKDatenbankAnlegen(QSqlDatabase db=QSqlDatabase::database());

bool istValideDatenbank(const QString& filename);
bool istValideDatenbank(QSqlDatabase db=QSqlDatabase::database());

void DatenbankverbindungSchliessen(QString con = QLatin1String(QSqlDatabase::defaultConnection));
void DatenbankZurAnwendungOeffnen( QString newDbFile="");
void CheckDbConsistency( QStringList& msg);

bool ensureTable(dbtable table, QSqlDatabase& db);

bool createDbCopy(QString targetfn, bool deper);

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

QVariant Eigenschaft(const QString& name);

void CsvActiveContracts();

struct DbSummary
{
    int AnzahlDkGeber;
    int AnzahlDkGeberEin;
    int AnzahlDkGeberZwei;
    int AnzahlDkGeberMehr;

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
void berechneVertragsenden(QVector<ContractEnd>& ce, QString con="");
void berechneZusammenfassung(DbSummary& dbs, QString con="");

struct YZV
{
    int year;
    double intrest;
    int count;
    double sum;
};
void berechneJahrZinsVerteilung( QVector<YZV>& yzv, QString con ="");

QString LaufzeitenVerteilungHtml(QString con="");

#endif // DKDBHELPER_H
