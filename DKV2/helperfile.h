#ifndef FILEHELPER_H
#define FILEHELPER_H

#if defined(Q_OS_WIN)
#include "windows.h"
#else
#include <stdlib.h>
#endif

#include <QString>

QString tempPathTemplateFromPath (const QString& path);
QString getUniqueTempFilename(const QString& templateFileName);

bool moveToBackup(const QString &fn);

bool backupFile(const QString& filename, const QString& subfolder=QString());

static const bool showFolder =true;
static const bool showFile   =false;
void showInExplorer(const QString &fullPath, bool fileOrFolder =showFile);

void printHtmlToPdf( const QString &html, const QString& css, const QString &fn);

QString absoluteCanonicalPath(const QString &path);

QString fileToString( const QString& filename);
bool stringToFile( const QString& string, const QString& fullFileName);

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
