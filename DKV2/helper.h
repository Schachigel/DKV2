#ifndef HELPER_H
#define HELPER_H

#include <QString>
#include <QDebug>

// Generic class for tracing the function call
class functionlogging {
private:
    QString fname;
public:
    functionlogging(QString x) :fname(x) {
        qDebug() << "Begin of " << fname;
    }
    ~functionlogging(){
        qDebug() << "End of " << fname;
    }
};

//Define MACRO for easy use for end user.
#define LOG_ENTRY_EXIT_FOR(x)       functionlogging  SomeLongNameThatIsNotLikelyToBeUsedInTheFunctionlogger(x)
#define LOG_ENTRY_and_EXIT              LOG_ENTRY_EXIT_FOR(__func__)

#endif // HELPER_H
