#ifndef DKDBHELPER_H
#define DKDBHELPER_H

#include "dbstructure.h"

bool insertDKDB_Views( const QSqlDatabase &db =QSqlDatabase::database());
bool insertDKDB_Indices( const QSqlDatabase& db=QSqlDatabase::database());
bool fill_DkDbDefaultContent(const QSqlDatabase &db = QSqlDatabase::database(), bool includeViews =true, zinssusance sz =zs_30360);

enum version_check_result {
    noVersion =-1,
    lowVersion =0,
    sameVersion =1,
    higherVersion =2
};

int get_db_version(const QString &filename);

bool open_databaseForApplication( const QString &newDbFile="");

bool postDB_UpgradeActions(int /*sourceVersion*/, const QString & dbName);

bool isValidNewContractLabel(const QString& newLabel);
QString proposeContractLabel();
void create_sampleData(int datensaetze =20);

int createNewInvestmentsFromContracts( bool fortlaufend);
int automatchInvestmentsToContracts();
bool createCsvActiveContracts();

struct contractRuntimeDistrib_rowData
{
    QString text;
    QString number;
    QString value;
};
Q_DECLARE_TYPEINFO(contractRuntimeDistrib_rowData, Q_PRIMITIVE_TYPE);
QVector<contractRuntimeDistrib_rowData> contractRuntimeDistribution();

QVector<QStringList> perpetualInvestment_bookings();
QVector<QStringList> perpetualInvestmentByContracts();

struct BookingDateData {
    int count;
    QString type;
    QDate date;
};
Q_DECLARE_TYPEINFO(BookingDateData, Q_PRIMITIVE_TYPE);

// calculations for time series statistics
void getActiveContracsBookingDates( QVector<BookingDateData>& dates);
void getInactiveContractBookingDates( QVector<BookingDateData>& dates);
void getFinishedContractBookingDates( QVector<BookingDateData>& dates);
void getAllContractBookingDates( QVector<BookingDateData>& dates);

// calculations for Uebersichten
double valueOfAllContracts();
QVector<QStringList> overviewShortInfo(const QString& sql);

struct contractEnd_rowData
{
    int year;
    int count;
    double value;
};
Q_DECLARE_TYPEINFO(contractEnd_rowData, Q_PRIMITIVE_TYPE);
void calc_contractEnd(QVector<contractEnd_rowData>& ce);

QString Vor_Nachname_Kreditor(qlonglong id);

struct changeBookingData {
    QString VKennung;
    QString Vorname;
    QString Nachname;
    int BetragInCt;
    QDate Buchungsdatum;
};
bool getChangeBookingData(changeBookingData& cbd, qlonglong bid);

#endif // DKDBHELPER_H
