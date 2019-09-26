#ifndef HELPER_H
#define HELPER_H
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

#endif // HELPER_H
