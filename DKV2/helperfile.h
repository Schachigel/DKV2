#ifndef FILEHELPER_H
#define FILEHELPER_H
#include <QString>

bool backupFile(const QString& filename, const QString& subfolder="");

void showFileInFolder(const QString &path);

void printHtmlToPdf( QString html, QString fn);

#endif // FILEHELPER_H
