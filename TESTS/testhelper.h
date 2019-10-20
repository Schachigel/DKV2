#ifndef TESTHELPER_H
#define TESTHELPER_H

#include <QString>
extern const QString testCon;

int tableRecordCount(QString table);
bool dbHasTable(QString tname);
bool dbTableHasField(const QString tname, const QString fname);

#endif // TESTHELPER_H
