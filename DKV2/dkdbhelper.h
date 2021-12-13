#ifndef DKDBHELPER_H
#define DKDBHELPER_H

#include <QSqlDatabase>

#include <QDateTime>
#include <QList>
#include <QString>

#include "booking.h"
#include "dbtable.h"
#include "dbfield.h"
#include "dbstructure.h"
#include "helpersql.h"


bool insert_views( const QSqlDatabase &db =QSqlDatabase::database());
bool fill_DkDbDefaultContent(const QSqlDatabase &db = QSqlDatabase::database(), bool includeViews =true, zinssusance sz =zs30360);

enum version_check_result {
    noVersion =-1,
    lowVersion =0,
    sameVersion =1,
    higherVersion =2
};

int get_db_version(const QSqlDatabase &db =QSqlDatabase::database());
int get_db_version(const QString &filename);

void closeAllDatabaseConnections();
bool open_databaseForApplication( const QString &newDbFile=qsl(""));
bool isExistingContractLabel( const QString& newLabel);
bool isExistingExContractLabel( const QString& newLabel);
bool isValidNewContractLabel( const QString& label);
QString proposeContractLabel();
void create_sampleData(int datensaetze =20);

int createNewInvestmentsFromContracts();
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


#endif // DKDBHELPER_H
