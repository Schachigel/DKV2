#ifndef HELPER_H
#define HELPER_H

#include <QString>
#include <QTime>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QElapsedTimer>

#include "filehelper.h"


class dbgTimer
{
    QElapsedTimer t;
    QString fname;
public:
    dbgTimer() {t.start();}
    dbgTimer(QString fu) : fname(fu){t.start(); qInfo() << fname << " start";}
    ~dbgTimer() {qInfo() << (fname.isEmpty() ? "" : fname+ " end" )<< endl << "Elapsed time: "<< t.elapsed();}
};

class functionlogging {
private:
    QString fname;
    static int depth;
public:
    functionlogging(QString x) :fname(x) {
        depth++;
        QString fill=QString(">").repeated(depth);
        qDebug().noquote() << fill << fname;
    }
    ~functionlogging(){
        QString fill=QString("<").repeated(depth);
        depth--;
        qDebug().noquote() << fill << fname << endl;
    }
};

// create a log entry for entry and exit of function call

#define LOG_ENTRY_EXIT_FOR(x) functionlogging  SomeLongNameThatIsNotLikelyToBeUsedInTheFunctionlogger(x)
#define LOG_CALL              LOG_ENTRY_EXIT_FOR(__func__)
#define LOG_CALL_W(x)         LOG_ENTRY_EXIT_FOR(__func__ + QString("(\"") + x + QString("\")"))

extern const QDate EndOfTheFuckingWorld;

extern QFile* outFile_p;

void logger(QtMsgType type, const QMessageLogContext &context, const QString &msg);
QString logFilePath();

#endif // HELPER_H
