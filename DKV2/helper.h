#ifndef HELPER_H
#define HELPER_H

#include <iso646.h>


#define qsl(x) QStringLiteral(x)

inline const QVariant emptyStringV {""}; // keep the qsl to force the variant to type String!
inline QString singleQuoted(const QString& s) { return qsl("'%1'").arg(s);}

inline void expected_error (QString MSG) {QMessageBox::information(NULL, qsl("Fehler"), MSG); qInfo() << MSG;}

inline QString cat (QString s) { return s; }

/*
template <typename T, typename ...Ts>
inline QString cat(T s1, Ts ... rest)
{
    return s1 +QStringLiteral(" _ ") +cat (rest ...);
}
ODER
template <typename ...TS>
void log(TS&& ... args)
{
    QDebug out = qInfo().noquote();
    ((out << std::forward<TS>(args)), ...);
}
*/

/*
template <typename t, typename ... TS>
inline t returnLog(int sev, t returnvalue, TS ...args)
{
    if( sev == 0) {
        (( qInfo().noquote () << std::forward<TS>(args)), ...);
    }
    else {
        (( qCritical().noquote () << std::forward<TS>(args)), ...);
    }
    return returnvalue;
}
*/

inline QString qCat(QString s)
{
    return s;
}

template<typename t, typename ... args>
inline QString qCat(t first, args... rest)
{
    return first +qsl(" ") +qCat(rest...);
}

template <typename ret_t, typename ... TS>
inline ret_t returnLog(int sev, ret_t ret, TS ...args)
{
    if( sev == 0) {
        qInfo().noquote() << qCat(args...);
    } else {
        qCritical().noquote() << qCat(args...);
    }
    return ret;
}

#define RETURN_OK(ret, ...)  return returnLog( 0, ret, __VA_ARGS__)
#define RETURN_ERR(ret, ...) return returnLog( 1, ret, __VA_ARGS__)

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
    dbgTimer(const QString& fu) : fname(fu){t.start(); qInfo().noquote() << qsl("Debug Timer ") + fname << qsl(" start") << qsl("\n");}
    ~dbgTimer() {qInfo().noquote() << "\n" << (fname.isEmpty() ? QString() : fname+ qsl(" end") )
                                   << "\n" << qsl("Elapsed time: ")<< t.elapsed() << "ms" << qsl("\n");}
    void lab(const QString& msg =QString()) {
        qint64 now =t.elapsed();
        qInfo().noquote() << (fname.isEmpty() ? QString() : fname) <<  qsl(" Lab# ")
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
        qInfo().noquote() << fill << fuName << qsl("\n");
    }
};

// create a log entry for entry and exit of function call

#define LOG_CALL functionlogging  SomeLongNameThatIsNotLikelyToBeUsedInTheFunctionlogger(__func__, __FILE__)
#define LOG_CALL_W(x) functionlogging  SomeLongNameThatIsNotLikelyToBeUsedInTheFunctionlogger( __func__ + qsl("(\"") + (x) + qsl("\")"), __FILE__)

inline const QDate EndOfTheFuckingWorld{QDate(9999, 12, 31)};
inline const QString EndOfTheFuckingWorld_str{qsl("9999-12-31")};
inline const int daysUntilTheEndOfTheFuckingWorld{2916000};
inline const QDate BeginingOfTime{QDate(1900, 1, 1)};
inline bool isLastDayOfTheYear(QDate d) { return (d.month() == 12 && d.day() == 31);}

void logger(QtMsgType type, const QMessageLogContext &context, const QString &msg);
QString logFilePath();

QMainWindow* getMainWindow();

QString getDbFileFromCommandline();

#endif // HELPER_H
