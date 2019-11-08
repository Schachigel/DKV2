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

void initDKDBStruktur();

bool DKDatenbankAnlegen(const QString& filename, QSqlDatabase db=QSqlDatabase());
bool istValideDatenbank(const QString& filename);
void DatenbankverbindungSchliessen();
void DatenbankZurAnwendungOeffnen( QString newDbFile="");

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

struct DbSummary
{
    double aktiveDk;
    double passiveDk;
    double WertAktiveDk;
};

void berechneZusammenfassung(DbSummary& dbs, QString con="");

#endif // DKDBHELPER_H
