#ifndef TESTHELPER_H
#define TESTHELPER_H

#include <QString>
#include <QSqlDatabase>
#include <QElapsedTimer>

#include "../DKV2/helper.h"

extern const QString testDbFilename;

void initTestDb();
void initTestDb_withData();
void cleanupTestDb();
void closeDbConnection( QSqlDatabase db =QSqlDatabase::database());
void createEmptyFile(QString fn);
int tableRecordCount(const QString table, const QSqlDatabase db =QSqlDatabase::database());
bool dbHasTable(const QString tname, const QSqlDatabase db =QSqlDatabase::database());
bool dbTableHasField(const QString tname, const QString fname, QSqlDatabase db =QSqlDatabase::database());
bool dbsHaveSameTables(const QString fn1, const QString fn2);
bool dbsHaveSameTables(const QSqlDatabase db1, const QSqlDatabase db2);
bool dbTablesHaveSameFields(const QString table1, const QString table2, const QSqlDatabase db =QSqlDatabase::database());
#endif // TESTHELPER_H
