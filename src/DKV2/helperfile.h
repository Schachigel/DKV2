#ifndef FILEHELPER_H
#define FILEHELPER_H

#if defined(Q_OS_WIN)
#include <windows.h>
#include <winnt.h>
#else
#include <stdlib.h>
#endif

QString getUniqueTempFilename(const QString& templateFileName, const QString &purpose);

bool moveToBackup(const QString &fn);

bool backupFile(const QString& filename, const QString& subfolder=QString());

#define showFolder true
#define showFile   false

void showInExplorer(const QString &fullPath, bool fileOrFolder =showFile);

void printHtmlToPdf( const QString &html, const QString& css, const QString &fn);

QString absoluteCanonicalPath(const QString &path);

QString fileToString( const QString& filename);
bool stringToFile( const QString& string, const QString& fullFileName);
QString makeSafeFileName(QString name, int maxSize =8);

//
// open a text file to signal that a database was opened
//
#if defined(Q_OS_WIN)
extern HANDLE openDbSignalnFile;
#else
extern QFile* openDbIndicationFile;
#endif
void createSignalFile(const QString& filename);
void deleteSignalFile();
bool checkSignalFile(const QString& filename);

#endif // FILEHELPER_H
