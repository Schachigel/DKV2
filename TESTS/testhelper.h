#ifndef TESTHELPER_H
#define TESTHELPER_H

#include <QString>
#include <QSqlDatabase>

#include "../DKV2/helper.h"


extern const QString testCon;
extern const QString testDbFilename;
QSqlDatabase testDb();

int tableRecordCount(QString table);
bool dbHasTable(QString tname);
bool dbTableHasField(const QString tname, const QString fname);

#endif // TESTHELPER_H
