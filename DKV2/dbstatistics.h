#ifndef DBSTATISTICS_H
#define DBSTATISTICS_H

#include <QStringList>

#include "helper.h"

struct dbStatistics
{
    // active contracts vs...
    int nbrCreditors_activeContracts =0;
    int nbrActiveContracts =0;
    double valueActiveContracts =0.;
    double avgInterestActiveContracts =0.;
    double weightedAvgInterestActiveContracts =0.;
    double annualInterestActiveContracts =0.;
    // inactive contracts
    int nbrCreditors_inactiveContracts =0;
    int nbrInactiveContracts =0;
    double valueInactiveContracts =0.;
    double avgInterestInactiveContracts =0.;
    double weightedAvgInterestInactiveContracts =0.;
    double expectedAnnualInterestInactiveContrasts =0.;
    // all contract summary
    int nbrCreditors =0;
    int nbrContracts =0;
    double valueContracts;
    double avgInterestRate =0.;
    double weightedAvgInterestRate =0.;
    double annualInterest =0.;

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
        auto icmp = [ ](QString s, int l, int r) {
            if( l != r) {
                s = s + qsl(" (%1, %2)");
                s = s.arg(QString::number(l), QString::number(r));
                qInfo() << "comparison of dbStatistics failed in " << s;
                return false;
            }
            return true;
        };
        auto dcmp = [ ](QString s, double l, double r) {

            if( ! qFuzzyCompare(1. +l, 1.+ r)) {
                s = s + qsl(" (%1, %2)");
                s = s.arg(QString::number(l), QString::number(r));
                qInfo() << "comparison of dbStatistics failed in " << s;
                return false;
            }
            return true;
        };
        bool ret =
               icmp(qsl("nbrCreditors_activeContracts"), lhs.nbrCreditors_activeContracts, rhs.nbrCreditors_activeContracts);
        ret &= icmp(qsl("nbrActiveContracts"), lhs.nbrActiveContracts, rhs.nbrActiveContracts);
        ret &= dcmp(qsl("valueActiveContracts"), lhs.valueActiveContracts, rhs.valueActiveContracts);
        ret &= dcmp(qsl("avgInterestActiveContracts"), lhs.avgInterestActiveContracts, rhs.avgInterestActiveContracts);
        ret &= dcmp(qsl("weightedAvgInterestActiveContracts"), lhs.weightedAvgInterestActiveContracts, rhs.weightedAvgInterestActiveContracts);
        ret &= dcmp(qsl("annualInterestActiveContracts"), lhs.annualInterestActiveContracts, rhs.annualInterestActiveContracts);

        ret &= icmp(qsl("nbrCreditors_inactiveContracts"), lhs.nbrCreditors_inactiveContracts, rhs.nbrCreditors_inactiveContracts);
        ret &= icmp(qsl("nbrInactiveContracts"), lhs.nbrInactiveContracts, rhs.nbrInactiveContracts);
        ret &= dcmp(qsl("valueInactiveContracts"), lhs.valueInactiveContracts, rhs.valueInactiveContracts);
        ret &= icmp(qsl("avgInterestInactiveContracts"), lhs.avgInterestInactiveContracts, rhs.avgInterestInactiveContracts);
        ret &= icmp(qsl("weightedAvgInterestInactiveContracts"), lhs.weightedAvgInterestInactiveContracts, rhs.weightedAvgInterestInactiveContracts);
        ret &= dcmp(qsl("expectedAnnualInterestInactiveContrasts"), lhs.expectedAnnualInterestInactiveContrasts, rhs.expectedAnnualInterestInactiveContrasts);

        ret &= icmp(qsl("nbrCreditors"), lhs.nbrCreditors, rhs.nbrCreditors);
        ret &= icmp(qsl("nbrContracts"), lhs.nbrContracts, rhs.nbrContracts);
        ret &= dcmp(qsl("valueContracts"), lhs.valueContracts, rhs.valueContracts);
        ret &= dcmp(qsl("avgInterestRate"), lhs.avgInterestRate, rhs.avgInterestRate);
        ret &= dcmp(qsl("weightedAvgInterestRate"), lhs.weightedAvgInterestRate, rhs.weightedAvgInterestRate);
        ret &= dcmp(qsl("annualInterest"), lhs.annualInterest, rhs.annualInterest);

        ret &= icmp(qsl("nbrActiveReinvesting"), lhs.nbrActiveReinvesting, rhs.nbrActiveReinvesting);
        ret &= dcmp(qsl("valueActiveReinvesting"), lhs.valueActiveReinvesting, rhs.valueActiveReinvesting);
        ret &= dcmp(qsl("annualInterestReinvestment"), lhs.annualInterestReinvestment, rhs.annualInterestReinvestment);

        ret &= icmp(qsl("nbrActiveNotReinvesting"), lhs.nbrActiveNotReinvesting, rhs.nbrActiveNotReinvesting);
        ret &= dcmp(qsl("valueActiveNotReinvesting"), lhs.valueActiveNotReinvesting, rhs.valueActiveNotReinvesting);
        ret &= dcmp(qsl("annualInterestPayout"), lhs.annualInterestPayout, rhs.annualInterestPayout);
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
