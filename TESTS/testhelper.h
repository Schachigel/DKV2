#ifndef TESTHELPER_H
#define TESTHELPER_H

#include <QTest>
#include "../DKV2/helper.h"

inline const QString testDbFilename {qsl("./data/testdb.sqlite")};
inline const QString testTemplateDb {qsl("./data/template.sqlite")};

void getRidOfFile(QString fn);
void initTestDkDb_InMemory();
void initTestDkDb();
void createTestDkDb_wData();
void createTestDkDbTemplate();
void cleanupTestDkDbTemplate();
void initTestDkDbFromTemplate();

void createTestDb_withRandomData();
void cleanupTestDb_InMemory();
void cleanupTestDkDb();
void openDefaultDbConnection_InMemory();
void openDefaultDbConnection(QString file =testDbFilename);
void closeDefaultDbConnection();
void createEmptyFile(const QString& fn);
//int tableRecordCount(const QString& table, const QSqlDatabase& db =QSqlDatabase::database());
bool dbHasTable(const QString& tname, const QSqlDatabase& db =QSqlDatabase::database());
bool dbTableHasField(const QString& tname, const QString& fname, const QSqlDatabase &db =QSqlDatabase::database());
bool dbsHaveSameTables(const QString &fn1, const QString &fn2);
bool dbsHaveSameTables(const QSqlDatabase& db1, const QSqlDatabase& db2);
bool dbTablesHaveSameFields(const QString& table1, const QString& table2, const QSqlDatabase& db =QSqlDatabase::database());
#endif // TESTHELPER_H
