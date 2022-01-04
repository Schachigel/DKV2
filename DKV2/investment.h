#ifndef INVESTMENT_H
#define INVESTMENT_H
#include <QDate>
#include "contract.h"
#include "dbtable.h"

class investment
{
public:
    investment(qlonglong id =-1, int interest =0, QDate start =EndOfTheFuckingWorld, QDate end =EndOfTheFuckingWorld, const QString& type =qsl(""), bool state =true);
    bool matchesContract(const contract& c);
    static const dbtable& getTableDef();
    qlonglong rowid;
    int interest;
    QDate start;
    QDate end;
    QString type;
    bool state;
    QString toString() const;
};


qlonglong createInvestmentFromContractIfNeeded(const int ZSatz, QDate vDate);
qlonglong saveNewInvestment(int ZSatz, QDate start, QDate end, const QString &type);

bool deleteInvestment(const int ZSatz, const QDate v, const QDate b, const QString& t);
bool closeInvestment(const int ZSatz, const QDate v, const QDate b, const QString& t);
bool openInvestment(const int ZSatz, const QDate v, const QDate b, const QString& t);

int nbrActiveInvestments(const QDate contractDate=EndOfTheFuckingWorld);
QVector<QPair<qlonglong, QString>> activeInvestments(const QDate contractDate=EndOfTheFuckingWorld);
int interestOfInvestmentByRowId(qlonglong rid);
QString investmentInfoForNewContract(qlonglong ridInvestment, double amount, QDate contractStartDate);

QVector<investment> openInvestments(int rate, QDate concluseionDate);
int closeInvestmentsPriorTo(QDate d);

#endif // INVESTMENT_H
