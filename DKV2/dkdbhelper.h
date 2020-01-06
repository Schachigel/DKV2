#ifndef DKDBHELPER_H
#define DKDBHELPER_H
#include <qdebug.h>
#include <QSqlDatabase>
#include <qsqlquery.h>
#include <qsqlerror.h>
#include <qsqlrecord.h>
#include <qsqlfield.h>
#include <qvariant.h>
#include <qdatetime.h>
#include <qlist.h>
#include <qstring.h>

#include "dbtable.h"
#include "dbfield.h"
#include "dbstructure.h"

extern dbstructure dkdbstructur;
extern QList<QPair<int, QString>> Buchungsarten;

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
void initBuchungsarten();

bool DKDatenbankAnlegen(const QString& filename, QSqlDatabase db=QSqlDatabase());
bool istValideDatenbank(const QString& filename);
void DatenbankverbindungSchliessen();
void DatenbankZurAnwendungOeffnen( QString newDbFile="");
void CheckDbConsistency( QStringList& msg);

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
