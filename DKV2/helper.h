#ifndef HELPER_H
#define HELPER_H

#include <QString>
#include <QTime>
#include <QDebug>
#include <QFile>

#include "filehelper.h"

class functionlogging {
private:
    QString fname;
public:
    functionlogging(QString x) :fname(x) {
        qDebug() << ">> " << fname;
    }
    ~functionlogging(){
        qDebug() << "<< " << fname << endl;
    }
};

//Define MACRO for easy use for end user.
#define LOG_ENTRY_EXIT_FOR(x)       functionlogging  SomeLongNameThatIsNotLikelyToBeUsedInTheFunctionlogger(x)
#define LOG_ENTRY_and_EXIT              LOG_ENTRY_EXIT_FOR(__func__)

extern QFile* outFile_p;

void logger(QtMsgType type, const QMessageLogContext &context, const QString &msg);
QString logFilePath();


#endif // HELPER_H
