#ifndef OPENDATABASE_H
#define OPENDATABASE_H

// old style
//bool open_Database(const QString& dbFile);

bool askUserForNextDb();

bool openDB_atStartup();
bool openDB_MRU(const QString path);


//bool openDB_interactive();

#endif // OPENDATABASE_H
