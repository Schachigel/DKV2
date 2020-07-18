#ifndef HELPER_H
#define HELPER_H

#include <QMainWindow>
#include <QWidget>
#include <QApplication>
#include <QObject>


#include <QString>
#include <QElapsedTimer>
#include <QFile>
#include <QDebug>

#include "helperfile.h"


class dbgTimer
{
    QElapsedTimer t;
    QString fname;
public:
    dbgTimer() {t.start();}
    dbgTimer(QString fu) : fname(fu){t.start(); qInfo().noquote() << "Debug Timer " + fname << " start" << Qt::endl;}
    ~dbgTimer() {qInfo().noquote() << Qt::endl << (fname.isEmpty() ? "" : fname+ " end" )<< Qt::endl << "Elapsed time: "<< t.elapsed() << Qt::endl;}
};

class functionlogging {
private:
    QString fname;
    static int depth;
public:
    functionlogging(QString x) :fname(x) {
        depth++;
        QString fill=QString(">").repeated(depth);
        qInfo().noquote() << fill << fname;
    }
    ~functionlogging(){
        QString fill=QString("<").repeated(depth);
        depth--;
        qInfo().noquote() << fill << fname << Qt::endl;
    }
};

// create a log entry for entry and exit of function call

//#define LOG_ENTRY_EXIT_FOR(x) // ;functionlogging  SomeLongNameThatIsNotLikelyToBeUsedInTheFunctionlogger(x)
#define LOG_ENTRY_EXIT_FOR(x) functionlogging  SomeLongNameThatIsNotLikelyToBeUsedInTheFunctionlogger(x)
#define LOG_CALL              LOG_ENTRY_EXIT_FOR(__func__)
#define LOG_CALL_W(x)         LOG_ENTRY_EXIT_FOR(__func__ + QString("(\"") + x + QString("\")"))

extern const QDate EndOfTheFuckingWorld;
extern const QDate BeginingOfTime;

extern QFile* outFile_p;

void logger(QtMsgType type, const QMessageLogContext &context, const QString &msg);
QString logFilePath();

QMainWindow* getMainWindow();


#endif // HELPER_H
