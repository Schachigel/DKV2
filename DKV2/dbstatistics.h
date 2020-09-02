#ifndef DBSTATISTICS_H
#define DBSTATISTICS_H

#include <QStringList>

#include "helper.h"

struct dbStatistics
{
    // all contract summary
    int nbrCreditors =0;
    int nbrContracts =0;
    double valueContracts;
    double weightedAvgInterestRate =0.;
    double avgInterestRate =0.;
    double annualInterest =0.;
    // active contracts vs...
    int nbrCreditors_activeContracts =0;
    int nbrActiveContracts =0;
    double valueActiveContracts =0.;
    double weightedAvgInterestActiveContracts =0.;
    double avgInterestActiveContracts =0.;
    double annualInterestActiveContracts =0.;
    // inactive contracts
    int nbrCreditors_inactiveContracts =0;
    int nbrInactiveContracts =0;
    double valueInactiveContracts =0.;
    double weightedAvgInterestInactiveContracts =0.;
    double avgInterestInactiveContracts =0.;
    double expectedAnnualInterestInactiveContrasts =0.;
    // off all active contracts: reinvesting vs. ...
    int nbrActiveReinvesting =0;
    double valueActiveReinvesting =0.;
    double annualInterestReinvestment =0.;
    // non reinvesting
    int nbrActiveNotReinvesting =0;
    double valueActiveNotReinvesting =0.;
    double annualInterestPayout =0.;

    inline friend bool operator==(const dbStatistics& lhs, const dbStatistics& rhs)
    {
        auto cmp = [ ](QString s, auto l, auto r) {
            if( l != r) {
                s = s + qsl(" (%1, %2)");
                s = s.arg(QString::number(l), QString::number(r));
                qInfo() << "comparison of dbStatistics failed in " << s;
                return false;
            }
            return true;
        };
        bool ret = cmp(qsl("nbrCreditors"), lhs.nbrCreditors, rhs.nbrCreditors);
        ret &= cmp(qsl("nbrContracts"), lhs.nbrContracts, rhs.nbrContracts);
        ret &= cmp(qsl("valueContracts"), lhs.valueContracts, rhs.valueContracts);
        ret &= cmp(qsl("weightedAvgInterestRate"), lhs.weightedAvgInterestRate, rhs.weightedAvgInterestRate);
        ret &= cmp(qsl("avgInterestRate"), lhs.avgInterestRate, rhs.avgInterestRate);
        ret &= cmp(qsl("annualInterest"), lhs.annualInterest, rhs.annualInterest);
        ret &= cmp(qsl("nbrCreditors_activeContracts"), lhs.nbrCreditors_activeContracts, rhs.nbrCreditors_activeContracts);
        ret &= cmp(qsl("nbrActiveContracts"), lhs.nbrActiveContracts, rhs.nbrActiveContracts);
        ret &= cmp(qsl("valueActiveContracts"), lhs.valueActiveContracts, rhs.valueActiveContracts);
        ret &= cmp(qsl("weightedAvgInterestActiveContracts"), lhs.weightedAvgInterestActiveContracts, rhs.weightedAvgInterestActiveContracts);
        ret &= cmp(qsl("avgInterestActiveContracts"), lhs.avgInterestActiveContracts, rhs.avgInterestActiveContracts);
        ret &= cmp(qsl("annualInterestActiveContracts"), lhs.annualInterestActiveContracts, rhs.annualInterestActiveContracts);
        ret &= cmp(qsl("nbrCreditors_inactiveContracts"), lhs.nbrCreditors_inactiveContracts, rhs.nbrCreditors_inactiveContracts);
        ret &= cmp(qsl("nbrInactiveContracts"), lhs.nbrInactiveContracts, rhs.nbrInactiveContracts);
        ret &= cmp(qsl("valueInactiveContracts"), lhs.valueInactiveContracts, rhs.valueInactiveContracts);
        ret &= cmp(qsl("weightedAvgInterestInactiveContracts"), lhs.weightedAvgInterestInactiveContracts, rhs.weightedAvgInterestInactiveContracts);
        ret &= cmp(qsl("avgInterestInactiveContracts"), lhs.avgInterestInactiveContracts, rhs.avgInterestInactiveContracts);
        ret &= cmp(qsl("expectedAnnualInterestInactiveContrasts"), lhs.expectedAnnualInterestInactiveContrasts, rhs.expectedAnnualInterestInactiveContrasts);
        ret &= cmp(qsl("nbrActiveReinvesting"), lhs.nbrActiveReinvesting, rhs.nbrActiveReinvesting);
        ret &= cmp(qsl("valueActiveReinvesting"), lhs.valueActiveReinvesting, rhs.valueActiveReinvesting);
        ret &= cmp(qsl("annualInterestReinvestment"), lhs.annualInterestReinvestment, rhs.annualInterestReinvestment);
        ret &= cmp(qsl("nbrActiveNotReinvesting"), lhs.nbrActiveNotReinvesting, rhs.nbrActiveNotReinvesting);
        ret &= cmp(qsl("valueActiveNotReinvesting"), lhs.valueActiveNotReinvesting, rhs.valueActiveNotReinvesting);
        ret &= cmp(qsl("annualInterestPayout"), lhs.annualInterestPayout, rhs.annualInterestPayout);
        return ret;
    }
    bool fillall();
    dbStatistics(bool calc=false) {
        if( calc)
            fillall();
    }
    QString toString();
};

inline dbStatistics getStatistic() {
    dbStatistics dbs (true);
    qInfo().noquote() << dbs.toString();
    return dbs;
}

// all contract summary
int    getNbrCreditors();
int    getNbrContracts();
double getValueContracts();
double getWeightedAvgInterestRate();
double getAvgInterestRate();
double getAnnualInterest();
// active vs ...
int    getNbrCreditors_activeContracts();
int    getNbrActiveContracts();
double getValueActiveContracts();
double getWeightedAvgInterestActiveContracts();
double getAvgInterestActiveContracts();
double getAnnualInterestActiveContracts();
// ... inactive
int    getNbrCreditors_inactiveContracts();
int    getNbrInactiveContracts();
double getValueInactiveContracts();
double getWeightedAvgInterestInactiveContracts();
double getAvgInterestInactiveContracts();
double getAnnualInterestInactiveContracts();
// active: reinvesting vs. ...
int    getNbrActiveReinvesting();
double getValueActiveReinvesting();
double getAnnualInterestActiveReinvesting();
// ... wPayout
int    getNbrActiveNotReinvesting();
double getValueActiveNotReinvesting();
double getAnnualInterestActiveNotReinvesting();

#endif // DBSTATISTICS_H
