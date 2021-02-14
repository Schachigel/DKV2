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



extern dbstructure dkdbstructur;
// THE structure of our database the single source of truth
void init_DKDBStruct();

bool createNewEmpty_DKDatabaseFile(const QString& filename);
bool insert_views( QSqlDatabase db =QSqlDatabase::database());
bool fill_dbDefaultContent(QSqlDatabase db = QSqlDatabase::database());


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
bool validDbSchema(const QString& filename);
// bool isValidDatabase(QSqlDatabase db =QSqlDatabase::database());

void closeAllDatabaseConnections();
bool open_databaseForApplication( QString newDbFile="");

bool create_DB_copy(QString targetfn, bool deper, QString file);
bool create_DB_copy(QString targetfn, bool anonym, QSqlDatabase db =QSqlDatabase::database());

QString proposeContractLabel();
void create_sampleData(int datensaetze =20);

bool createCsvActiveContracts();

struct PayedInterest
{
    int year =0;
    QString interestTypeDesc;
    double value =0;
};

void calc_payedInterestsByYear(QVector<PayedInterest>& pi);

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
void calc_anualInterestDistribution( QVector<YZV>& yzv);
struct rowData
{
    QString text; QString number; QString value;
};

QVector<rowData> contractRuntimeDistribution();

extern const QString sqlContractView;

#endif // DKDBHELPER_H
