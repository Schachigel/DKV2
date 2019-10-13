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

bool DKDatenbankAnlegen(const QString& filename);
bool istValideDatenbank(const QString& filename);
void DatenbankverbindungSchliessen();
void DatenbankZurAnwendungOeffnen( QString newDbFile="");

void BeispieldatenAnlegen(int datensaetze =20);

struct KreditorDaten
{
//    int id;
    QString Vorname;
    QString Nachname;
    QString Strasse;
    QString Plz;
    QString Stadt;
    QString Iban;
    QString Bic;
};
int KreditorDatenSpeichern(const KreditorDaten& p);
bool KreditorLoeschen(QString index);
typedef QPair<int, QString> KreditorAnzeigeMitId;
void KreditorenFuerAuswahlliste(QList<KreditorAnzeigeMitId>& Entries);

typedef QPair<int, QString> ZinsAnzeigeMitId;
void ZinssaetzeFuerAuswahlliste(QList<ZinsAnzeigeMitId>& Entries);

struct VertragsDaten
{
//    int id;
    QVariant KreditorId;
    QVariant Kennung;
    QVariant Betrag;
    QVariant Wert;
    QVariant Zins;
    QVariant tesaurierend;
    QVariant active;
    QVariant Vertragsdatum;
    QVariant LaufzeitEnde;
    QVariant StartZinsberechnung;
    VertragsDaten() :
        KreditorId(-1),
        Betrag(0.), Wert(0.), Zins(0.),
        tesaurierend(true), active(false),
        Vertragsdatum(QDate::currentDate()),
        LaufzeitEnde(QDate(9999, 12, 31)),
        StartZinsberechnung(QDate(9999, 12, 31)) {}
    bool verbucheVertrag();
    int speichereVertrag();
    bool BelegZuNeuemVertragSpeichern(const int vid);
};
// bool VertragVerbuchen(const VertragsDaten& c);
bool VertragAktivieren( int ContractId, QDate activationDate);
bool VertragLoeschen(QString index);

QString ContractList_SELECT(QVector<dbfield> f);
QString ContractList_FROM();
QString ContractList_WHERE(QString filter);
QString ContractList_SQL(QVector<dbfield> f, QString filter);

struct DbSummary
{
    double aktiveDk;
    double passiveDk;
    double WertAktiveDk;
};
void berechneZusammenfassung(DbSummary& dbs);


#endif // DKDBHELPER_H
