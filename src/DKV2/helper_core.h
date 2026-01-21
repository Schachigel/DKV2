#ifndef HELPER_CORE_H
#define HELPER_CORE_H

#define qsl(x) QStringLiteral(x)
inline QString singleQuoted(const QString& s) { return qsl("'%1'").arg(s);}
inline const QVariant emptyStringV {""}; // keep the qsl to force the variant to type String!

QString toString(const QBitArray &ba);
QBitArray toQBitArray(const QString& s);

class functionlogging {
private:
    QString fuName;
    QString fiName;
    static int depth;
public:
    functionlogging(const QString& fu, const QString& fi) :fuName(fu), fiName( fi) {
        depth++;
        QString fill=QString(qsl(">")).repeated(depth);
        qInfo().noquote() << QLatin1String("------- Enter function ");
        qInfo().noquote() << fill << fuName << " (" << fiName << ")";
    }
    functionlogging(const functionlogging&) =delete;
    ~functionlogging() {
        QString fill{ QString(qsl("<")).repeated(depth) +fuName +qsl("\n")};
        qInfo().noquote() << fill;
        depth--;

    }
};

// create a log entry for entry and exit of function call

#define LOG_CALL functionlogging  SomeLongNameThatIsNotLikelyToBeUsedInTheFunctionlogger(__func__, __FILE__)
#define LOG_CALL_W(x) functionlogging  SomeLongNameThatIsNotLikelyToBeUsedInTheFunctionlogger( __func__ + qsl("(\"") + (x) + qsl("\")"), __FILE__)

inline const QDate EndOfTheFuckingWorld{QDate(9999, 12, 31)};
inline const QString EndOfTheFuckingWorld_str{qsl("9999-12-31")};
inline const int daysUntilTheEndOfTheFuckingWorld{2916000};
inline const QDate BeginingOfTime{QDate(1900, 1, 1)};

void logger(QtMsgType type, const QMessageLogContext &context, const QString &msg);
QString logFilePath();

////////////////

inline QString cat (QString s) { return s; }

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
////////////////


#endif // HELPER_CORE_H
