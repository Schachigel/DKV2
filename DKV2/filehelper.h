#ifndef FILEHELPER_H
#define FILEHELPER_H
#include <qstring.h>
#include <qfile.h>

class fileCloser
{   // for use on the stack only
public:
    fileCloser (QFile* f){F=f;}
    fileCloser() = delete;
    ~fileCloser(){F->close();}
private:
    QFile* F;
};

bool backupFile(const QString& filename);

QString logFilePath();

#endif // FILEHELPER_H
