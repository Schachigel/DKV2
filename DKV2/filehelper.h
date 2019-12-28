#ifndef FILEHELPER_H
#define FILEHELPER_H
#include <QString>
#include <QFile>

bool backupFile(const QString& filename);

void showFileInFolder(const QString &path);

void printHtmlToPdf( QString html, QString fn);

QString getDbFolder();

#endif // FILEHELPER_H
