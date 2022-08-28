#ifndef HELPER_H
#define HELPER_H

#include <iso646.h>

#include <QObject>
#include <QApplication>
#include <QString>
#include <QStringLiteral>
#include <QMessageBox>
#include <QMainWindow>
#include <QWidget>
#include <QElapsedTimer>
#include <QFile>
#include <QDebug>

#define qsl(x) QStringLiteral(x)

inline void expected_error (QString MSG) {QMessageBox::information(NULL, qsl("Fehler"), MSG); qInfo() << MSG;}

QString toString(const QBitArray &ba);
QBitArray toQBitArray(const QString& s);

void setFontPs(QWidget* w, int ps);
void centerDlg(QWidget* parent, QWidget* child, int minWidth =300, int minHeight =400);
class dbgTimer
{
    QElapsedTimer t;
    QString fname;
    qint64 labcount =0;
    qint64 lastLab =0;
public:
    dbgTimer() {t.start();}
    dbgTimer(const dbgTimer&) = delete;
    dbgTimer(const QString& fu) : fname(fu){t.start(); qInfo().noquote() << qsl("Debug Timer ") + fname << qsl(" start") << Qt::endl;}
    ~dbgTimer() {qInfo().noquote() << Qt::endl << (fname.isEmpty() ? qsl("") : fname+ qsl(" end") )
                                   << Qt::endl << qsl("Elapsed time: ")<< t.elapsed() << "ms" << Qt::endl;}
    void lab(const QString& msg =QString()) {
        qint64 now =t.elapsed();
        qInfo().noquote() << (fname.isEmpty() ? qsl("") : fname) <<  qsl(" Lab# ")
                          << (msg.isEmpty() ? QString::number(labcount++) : msg)
                          << ":" << (now-lastLab)
                          << "ms (overall: " << now << ")";
        lastLab =now;
    }
};

class functionlogging {
private:
    QString fuName;
    QString fiName;
    static int depth;
public:
    functionlogging(const QString& fu, const QString& fi) :fuName(fu), fiName( fi) {
        depth++;
        QString fill=QString(qsl(">")).repeated(depth);
        qInfo().noquote() << fill << fuName << " (" << fiName << ")";
    }
    functionlogging(const functionlogging&) =delete;
    ~functionlogging() {
        QString fill=QString(qsl("<")).repeated(depth);
        depth--;
        qInfo().noquote() << fill << fuName << Qt::endl;
    }
};

// create a log entry for entry and exit of function call

//#define LOG_ENTRY_EXIT_FOR(x) // ;functionlogging  SomeLongNameThatIsNotLikelyToBeUsedInTheFunctionlogger(x)
//#define LOG_ENTRY_EXIT_FOR(x) functionlogging  SomeLongNameThatIsNotLikelyToBeUsedInTheFunctionlogger(x)
#define LOG_CALL functionlogging  SomeLongNameThatIsNotLikelyToBeUsedInTheFunctionlogger(__func__, __FILE__)
#define LOG_CALL_W(x) functionlogging  SomeLongNameThatIsNotLikelyToBeUsedInTheFunctionlogger( __func__ + qsl("(\"") + (x) + qsl("\")"), __FILE__)

extern const QDate EndOfTheFuckingWorld;
extern const QString EndOfTheFuckingWorld_str;
extern const int daysUntilTheEndOfTheFuckingWorld;
extern const QDate BeginingOfTime;

void logger(QtMsgType type, const QMessageLogContext &context, const QString &msg);
QString logFilePath();

QMainWindow* getMainWindow();

QString getDbFileFromCommandline();

#endif // HELPER_H
