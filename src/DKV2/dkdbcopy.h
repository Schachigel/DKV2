#ifndef DKDBCOPY_H
#define DKDBCOPY_H

#include "helper_core.h"
#include "dbstructure.h"

const QString dbCopyConnection {qsl("db_copy")};
// helper functions - exported for testability
QString moveToPreConversionCopy( const QString& file);

/*
*  copy_database will create a 1 : 1 copy of the currently opened database to a new file
*/
// bool copy_database(const QString& targetFName);
bool copy_Database_fromDefaultConnection( const QString& targetFName);
/*
*  copy_database_anonymous will create a 1 : 1 copy of the currently opened database to a new file
*  with all personal data replaced by random data
*/
bool copy_database_fDC_mangled(const QString& targetfn);

/*
*  convert_database will create a copy of a given database file to a new file using
*  the current schema inserting the data given, leaving any new fields empty / to their default value
*/
QString convert_database_inplace( const QString& targetFilename, const dbstructure& dbs =dkdbstructur);


#endif // DKDBCOPY_H
