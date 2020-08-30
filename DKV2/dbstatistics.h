#ifndef DBSTATISTICS_H
#define DBSTATISTICS_H

struct dbStatistics
{
    // all contract summary
    int numberOfCreditors;
    int numberOfContracts;
    double valueOfAllContracts;
    double weightedAverageInterestRateOfAllContracts;
    double averageInterestRateOfAllContracts;
    double annualInterestOfAllContracts;
    // active contracts vs...
    int numberOfCreditorsOfActiveContracts;
    int numberOfActiveContracts;
    double valueOfActiveContracts;
    double weightedAverageInterestRateOfActiveContracts;
    double averageInterestOfActiveContracts;
    double annualInterestOfActiveContrasts;
    // inactive contracts
    int numberOfCreditorsOfInactiveContracts;
    int numberOfInactiveContracts;
    double valueOfInactiveContracts;
    double weightedAverageInterestOfInactiveContracts;
    double averageInterestOfInactiveContracts;
    double annualInterestOfInactiveContrasts;
    // off all active contracts: reinvesting vs. ...
    int numberOfReinvestingContracts;
    double valueOfReinvestingContracts;
    double annualInterestWithReinvestment;
    // non reinvesting
    int numberOf_nonReinvestingContracts;
    double valueOf_nonReinvestingContracts;
    double annualInterestWithPayout;

    inline friend bool operator==(const dbStatistics& lhs, const dbStatistics& rhs)
    {
        return lhs.numberOfCreditors == rhs.numberOfCreditors
        && lhs.numberOfContracts == rhs.numberOfContracts
        && lhs.valueOfAllContracts == rhs.valueOfAllContracts
        && lhs.weightedAverageInterestRateOfAllContracts == rhs.weightedAverageInterestRateOfAllContracts
        && lhs.averageInterestRateOfAllContracts == rhs.averageInterestRateOfAllContracts
        && lhs.annualInterestOfAllContracts == rhs.annualInterestOfAllContracts
        && lhs.numberOfCreditorsOfActiveContracts == rhs.numberOfCreditorsOfActiveContracts
        && lhs.numberOfActiveContracts == rhs.numberOfActiveContracts
        && lhs.valueOfActiveContracts == rhs.valueOfActiveContracts
        && lhs.weightedAverageInterestRateOfActiveContracts == rhs.weightedAverageInterestRateOfActiveContracts
        && lhs.averageInterestOfActiveContracts == rhs.averageInterestOfActiveContracts
        && lhs.annualInterestOfActiveContrasts == rhs.annualInterestOfActiveContrasts
        && lhs.numberOfCreditorsOfInactiveContracts == rhs.numberOfCreditorsOfInactiveContracts
        && lhs.numberOfInactiveContracts == rhs.numberOfInactiveContracts
        && lhs.valueOfInactiveContracts == rhs.valueOfInactiveContracts
        && lhs.weightedAverageInterestOfInactiveContracts == rhs.weightedAverageInterestOfInactiveContracts
        && lhs.averageInterestOfInactiveContracts == rhs.averageInterestOfInactiveContracts
        && lhs.annualInterestOfInactiveContrasts == rhs.annualInterestOfInactiveContrasts
        && lhs.numberOfReinvestingContracts == rhs.numberOfReinvestingContracts
        && lhs.valueOfReinvestingContracts == rhs.valueOfReinvestingContracts
        && lhs.annualInterestWithReinvestment == rhs.annualInterestWithReinvestment
        && lhs.numberOf_nonReinvestingContracts == rhs.numberOf_nonReinvestingContracts
        && lhs.valueOf_nonReinvestingContracts == rhs.valueOf_nonReinvestingContracts
        && lhs.annualInterestWithPayout == rhs.annualInterestWithPayout;
    }
    bool fillall();
};

// all contract summary
int nbrCreditors();
int nbrContracts();
double valueContracts();
double weightedAverageInterestRate_();
double avgInterestRate_();
double annualInterest();
// active vs ...
int nbrCreditorsWithActiveContracts();
int     nbrActiveContracts();
double  valueActiveContracts();
double weightedAvgInterestActiveContracts();
double avgInterestActiveContracts();
double annualInterestActiveContracts();
// ... inactive
int nbrCreditorsInactiveContracts();
int nbrInactiveContracts();
double  valueInactiveContracts();
double weightedAvgInterestInactiveContracts();
double avgInterestInactiveContracts();
double annualInterestInactiveContracts();
// active: reinvesting vs. ...
int nbrNotReinvesting();
double valueNotReinvesting();
double annualInterestReinvest();
// ... wPayout
int nbrReinvesting();
double valueReinvesting();
double annualInterestPayout();

#endif // DBSTATISTICS_H
