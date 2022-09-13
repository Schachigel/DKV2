#ifndef OPENDATABASE_H
#define OPENDATABASE_H

#include "pch.h"

bool askUserForNextDb();

bool openDB_atStartup();
bool openDB_MRU(const QString path);

#endif // OPENDATABASE_H
