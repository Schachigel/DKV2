#ifndef OPENDATABASE_H
#define OPENDATABASE_H

bool askUserForNextDb();

bool openDB_atStartup();
bool openDB_MRU(const QString path);
bool checkSchema_ConvertIfneeded(const QString &origDbFile);


#endif // OPENDATABASE_H
