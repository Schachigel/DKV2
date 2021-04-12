#ifndef HELPER_H
#define HELPER_H

#include <iso646.h>

#include <QObject>
#include <QApplication>
#include <QString>
#include <QStringLiteral>
#include <QMainWindow>
#include <QWidget>
#include <QElapsedTimer>
#include <QFile>
#include <QDebug>

#include "helperfile.h"

#define qsl(x) QStringLiteral(x)

class dbgTimer
{
    QElapsedTimer t;
    QString fname;
public:
    dbgTimer() {t.start();}
    dbgTimer(QString fu) : fname(fu){t.start(); qInfo().noquote() << qsl("Debug Timer ") + fname << qsl(" start") << Qt::endl;}
    ~dbgTimer() {qInfo().noquote() << Qt::endl << (fname.isEmpty() ? qsl("") : fname+ qsl(" end") )
                                   << Qt::endl << qsl("Elapsed time: ")<< t.elapsed() << Qt::endl;}
};

class functionlogging {
private:
    QString fname;
    static int depth;
public:
    functionlogging(QString x) :fname(x) {
        depth++;
        QString fill=QString(qsl(">")).repeated(depth);
        qInfo().noquote() << fill << fname;
    }
    ~functionlogging(){
        QString fill=QString(qsl("<")).repeated(depth);
        depth--;
        qInfo().noquote() << fill << fname << Qt::endl;
    }
};

// create a log entry for entry and exit of function call

//#define LOG_ENTRY_EXIT_FOR(x) // ;functionlogging  SomeLongNameThatIsNotLikelyToBeUsedInTheFunctionlogger(x)
//#define LOG_ENTRY_EXIT_FOR(x) functionlogging  SomeLongNameThatIsNotLikelyToBeUsedInTheFunctionlogger(x)
#define LOG_CALL functionlogging  SomeLongNameThatIsNotLikelyToBeUsedInTheFunctionlogger(__func__)
#define LOG_CALL_W(x) functionlogging  SomeLongNameThatIsNotLikelyToBeUsedInTheFunctionlogger( __func__ + QString(qsl("(\"")) + x + QString(qsl("\")")))

extern const QDate EndOfTheFuckingWorld;
extern const QDate BeginingOfTime;

extern QFile* outFile_p;

void logger(QtMsgType type, const QMessageLogContext &context, const QString &msg);
QString logFilePath();

QMainWindow* getMainWindow();

QString getDbFileFromCommandline();

#endif // HELPER_H
