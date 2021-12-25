#ifndef DKDBCOPY_H
#define DKDBCOPY_H

#include <QString>
#include <QSqlDatabase>

#include "dbstructure.h"
#include "helper.h"

const QString dbCopyConnection {qsl("db_copy")};
// helper functions - exported for testability
QString moveToPreConversionCopy( const QString& file);

/*
*  copy_database will create a 1 : 1 copy of the currently opened database to a new file
*/
bool copy_database( const QString& sourceFName, const QString& targetFName, const dbstructure& dbs =dkdbstructur);
bool copy_dkdb_database( const QString& sourceFName, const QString& targetFName);
/*
*  copy_database_anonymous will create a 1 : 1 copy of the currently opened database to a new file
*  with all personal data replaced by random data
*/
bool copy_database_mangled(const QString &targetfn, const QString &file);
bool copy_database_mangled(const QString& targetfn, const QSqlDatabase& db =QSqlDatabase::database());

/*
*  convert_database will create a copy of a given database file to a new file using
*  the current schema inserting the data given, leaving any new fields empty / to their default value
*/
QString convert_database_inplace( const QString& targetFilename, const dbstructure& dbs =dkdbstructur);


#endif // DKDBCOPY_H
