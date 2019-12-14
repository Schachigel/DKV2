#ifndef FILEHELPER_H
#define FILEHELPER_H
#include <QString>
#include <QFile>

bool backupFile(const QString& filename);

void showFileInFolder(const QString &path);

#endif // FILEHELPER_H
