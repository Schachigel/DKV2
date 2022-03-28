#ifndef INVESTMENT_H
#define INVESTMENT_H
#include <QDate>
#include "contract.h"
#include "dbtable.h"

struct investment
{
// types
    struct invStatisticData
    {
        int    anzahlVertraege;
        double summeVertraege;
        double EinAuszahlungen;
        double ZzglZins;
    };
// interface
    investment(qlonglong id =-1, const int interest =0, const QDate start =EndOfTheFuckingWorld,
               const QDate end =EndOfTheFuckingWorld, const QString& type =QLatin1String(""), const bool state =true);
    QString toString() const;
    static const dbtable& getTableDef();

    bool matchesContract(const contract& c);
    bool isContinouse(){return ( end == EndOfTheFuckingWorld || not end.isValid ());};
    invStatisticData getStatisticData(const QDate newContractData);

// data
    qlonglong rowid;
    int interest;
    QDate start;
    QDate end;
    QString type;
    bool state;
};


qlonglong createInvestmentFromContractIfNeeded(const int ZSatz, QDate vDate);
qlonglong saveNewInvestment(int ZSatz, QDate start, QDate end, const QString &type);

bool deleteInvestment(const qlonglong rowid);
//bool closeInvestment(const int ZSatz, const QDate v, const QDate b, const QString& t);
//bool openInvestment(const int ZSatz, const QDate v, const QDate b, const QString& t);
bool closeInvestment(const qlonglong rowid);
bool openInvestment (const qlonglong rowid);

int nbrActiveInvestments(const QDate contractDate=EndOfTheFuckingWorld);
QVector<QPair<qlonglong, QString>> activeInvestments(const QDate contractDate=EndOfTheFuckingWorld);
int interestOfInvestmentByRowId(qlonglong rid);
QString investmentInfoForNewContract(const qlonglong ridInvestment, const double amount, const QDate contractStartDate);

QVector<investment> openInvestments(int rate, QDate concluseionDate);
int closeInvestmentsPriorTo(QDate d);


#endif // INVESTMENT_H
