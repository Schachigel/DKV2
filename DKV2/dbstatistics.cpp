
#include "helpersql.h"
#include "dbstatistics.h"

bool dbStatistics::fillall()
{
    numberOfContracts =nbrCreditors();
    numberOfContracts =nbrContracts();
    valueOfAllContracts =valueContracts();
    weightedAverageInterestRateOfAllContracts =weightedAverageInterestRate_();
    averageInterestRateOfAllContracts =avgInterestRate_();
    annualInterestOfAllContracts =annualInterest();
    // active contracts vs...
    numberOfCreditorsOfActiveContracts=nbrCreditorsWithActiveContracts();
    numberOfActiveContracts =nbrActiveContracts();
    valueOfActiveContracts =valueActiveContracts();
    weightedAverageInterestRateOfActiveContracts =weightedAvgInterestActiveContracts();
    averageInterestOfActiveContracts =avgInterestActiveContracts();
    annualInterestOfActiveContrasts =annualInterestActiveContracts();
    // inactive contracts
    numberOfCreditorsOfInactiveContracts =nbrCreditorsInactiveContracts();
    numberOfInactiveContracts =nbrInactiveContracts();
    valueOfInactiveContracts =valueInactiveContracts();
    weightedAverageInterestOfInactiveContracts =weightedAvgInterestInactiveContracts();
    averageInterestOfInactiveContracts =avgInterestInactiveContracts();
    annualInterestOfInactiveContrasts =annualInterestInactiveContracts();
    // off all active contracts: reinvesting vs. ...
    numberOfReinvestingContracts =nbrReinvesting();
    valueOfReinvestingContracts =valueReinvesting();
    annualInterestWithReinvestment = annualInterestReinvest();
    // non reinvesting
    numberOf_nonReinvestingContracts =nbrNotReinvesting();
    valueOf_nonReinvestingContracts =valueNotReinvesting();
    annualInterestWithPayout =annualInterestPayout();

    return false;
}
// all contract summary
int nbrCreditors()
{   // does not count creditors w/o a contract
    return executeSingleValueSql(qsl("*"), qsl("AnzahlAlleKreditoren")).toInt();
}
int nbrContracts()
{
    return executeSingleValueSql(qsl("COUNT(*)"), qsl("Verträge")).toInt();
}
double valueContracts()
{
    return executeSingleValueSql(qsl("SUM(Wert)"), qsl("WertAlleVerträge")).toDouble();
}
double weightedAverageInterestRate_()
{
    return executeSingleValueSql(qsl("*"), qsl("GewichteterMittlererZinsAlleVerträge")).toDouble();
}
double avgInterestRate_()
{
    return executeSingleValueSql(qsl("AVG(Zinssatz)"), qsl("WertAlleVertraege")).toDouble();
}
double annualInterest()
{
    return weightedAverageInterestRate_()*valueContracts();
}
// active vs ...
int nbrCreditorsWithActiveContracts()
{
    return executeSingleValueSql(qsl("AnzahlDkGeber"), qsl("AnzahlAktiveDkGeber")).toInt();
}
int nbrActiveContracts()
{
    return executeSingleValueSql(qsl("COUNT(*)"), qsl("WertAktiveVertraege")).toInt();
}
double valueActiveContracts()
{
    return executeSingleValueSql(qsl("SUM(Wert)"), qsl("WertAktiveVertraege")).toInt() /100.;
}
double weightedAvgInterestActiveContracts()
{
    return executeSingleValueSql(qsl("*"), qsl("GewichteterMittlererZinsAktiverVerträge")).toDouble();
}
double avgInterestActiveContracts()
{
    return executeSingleValueSql(qsl("AVG(Zinssatz)"), qsl("WertAktiveVertraege")).toDouble();
}
double annualInterestActiveContracts()
{
    return weightedAvgInterestActiveContracts()*valueActiveContracts();
};
// ... inactive
int nbrCreditorsInactiveContracts()
{
    return executeSingleValueSql(qsl("AnzahlDkGeber"), qsl("AnzahlInaktiveDkGeber")).toInt();
}
int nbrInactiveContracts()
{
    return executeSingleValueSql(qsl("COUNT(*)"), qsl("WertInaktiveVertraege")).toInt();
}
double valueInactiveContracts()
{
    return executeSingleValueSql(qsl("SUM(Wert)"), qsl("WertPassiveVertraege")).toDouble();
}
double weightedAvgInterestInactiveContracts()
{
    return executeSingleValueSql("*", "GewichteterMittlererZinsInaktiverVerträge").toDouble();
}
double avgInterestInactiveContracts()
{
    return executeSingleValueSql(qsl("AVG(Zinssatz)"), qsl("WertPassiveVertraege")).toDouble();
}
double annualInterestInactiveContracts()
{
    return weightedAvgInterestInactiveContracts()*valueInactiveContracts();
}

// active: reinvesting vs. ...
int nbrReinvesting(){}
double valueReinvesting(){}
double annualInterestReinvest(){}
// ... wPayout
int nbrNotReinvesting(){}
double valueNotReinvesting(){}
double annualInterestPayout(){}
