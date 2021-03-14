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


bool insert_views( QSqlDatabase db =QSqlDatabase::database());
bool fill_DkDbDefaultContent(QSqlDatabase db = QSqlDatabase::database());

enum version_check_result {
    noVersion =-1,
    lowVersion =0,
    sameVersion =1,
    higherVersion =2
};

version_check_result check_db_version(QSqlDatabase db =QSqlDatabase::database());
version_check_result check_db_version(QString filename);

// bool createView(QString name, QString Sql, QSqlDatabase db = QSqlDatabase::database());
// void updateViews(QSqlDatabase db =QSqlDatabase::database());
// bool isValidDatabase(QSqlDatabase db =QSqlDatabase::database());

void closeAllDatabaseConnections();
bool open_databaseForApplication( QString newDbFile="");
bool isExistingContractLabel( const QString& newLabel);
bool isExistingExContractLabel( const QString& newLabel);
bool isValidNewContractLabel( const QString& label);
QString proposeContractLabel();
void create_sampleData(int datensaetze =20);

int createNewInvestmentsFromContracts();

bool createCsvActiveContracts();

struct ContractEnd
{
    int year;
    int count;
    double value;
};
void calc_contractEnd(QVector<ContractEnd>& ce);

struct YZV
{
    int year;
    double intrest;
    int count;
    double sum;
};
void calc_annualInterestDistribution( QVector<YZV>& yzv);
struct rowData
{
    QString text; QString number; QString value;
};

QVector<rowData> contractRuntimeDistribution();

extern const QString sqlContractView;

#endif // DKDBHELPER_H
