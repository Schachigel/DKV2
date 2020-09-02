
#include "helpersql.h"
#include "dbstatistics.h"

bool dbStatistics::fillall()
{
    nbrCreditors =getNbrCreditors();
    nbrContracts =getNbrContracts();
    valueContracts =getValueContracts();
    weightedAvgInterestRate =getWeightedAvgInterestRate();
    avgInterestRate =getAvgInterestRate();
    annualInterest =getAnnualInterest();
    // active contracts vs...
    nbrCreditors_activeContracts=getNbrCreditors_activeContracts();
    nbrActiveContracts =getNbrActiveContracts();
    valueActiveContracts =getValueActiveContracts();
    weightedAvgInterestActiveContracts =getWeightedAvgInterestActiveContracts();
    avgInterestActiveContracts =getAvgInterestActiveContracts();
    annualInterestActiveContracts =getAnnualInterestActiveContracts();
    // inactive contracts
    nbrCreditors_inactiveContracts =getNbrCreditors_inactiveContracts();
    nbrInactiveContracts =getNbrInactiveContracts();
    valueInactiveContracts =getValueInactiveContracts();
    weightedAvgInterestInactiveContracts =getWeightedAvgInterestInactiveContracts();
    avgInterestInactiveContracts =getAvgInterestInactiveContracts();
    expectedAnnualInterestInactiveContrasts =getAnnualInterestInactiveContracts();
    // off all active contracts: reinvesting vs. ...
    nbrActiveReinvesting =getNbrActiveReinvesting();
    valueActiveReinvesting =getValueActiveReinvesting();
    annualInterestReinvestment = getAnnualInterestActiveReinvesting();
    // non reinvesting
    nbrActiveNotReinvesting =getNbrActiveNotReinvesting();
    valueActiveNotReinvesting =getValueActiveNotReinvesting();
    annualInterestPayout =getAnnualInterestActiveNotReinvesting();

    return false;
}
#define n2s(x) QString::number(x)
#define n2d_2s(x) QString::number(x, 'f', 2)
QString dbStatistics::toString()
{
    QString all {qsl("\nAll Contracts / Active / inactive\n"
                     "Creditors: %1 / %2 / %3\n"
                     "Contracts: %4 / %5 / %6\n"
                     "Wert:      %7 / %8 / %9\n")};
    all = all.arg(n2s(nbrCreditors), n2s(nbrCreditors_activeContracts), n2s(nbrCreditors_inactiveContracts),
                  n2s(nbrContracts), n2s(nbrActiveContracts), n2s(nbrInactiveContracts),
                  n2d_2s(valueContracts), n2d_2s(valueActiveContracts), n2d_2s(valueInactiveContracts));
    all += qsl( "gew.Avg.I: %1 / %2 / %3\n"
                "Avg. I.  : %4 / %5 / %6\n"
                "Annual I.: %7 / %8 / %9\n\n");
    all = all.arg(n2d_2s(weightedAvgInterestRate), n2d_2s(weightedAvgInterestActiveContracts), n2d_2s(weightedAvgInterestInactiveContracts),
                  n2d_2s(avgInterestRate), n2d_2s(avgInterestActiveContracts), n2d_2s(avgInterestInactiveContracts),
                  n2d_2s(annualInterest), n2d_2s(annualInterestActiveContracts), n2d_2s(expectedAnnualInterestInactiveContrasts));

    all += qsl("Active Contracts reinvesting / w payout\n"
                "nbr      : %1 / %2\n"
                "value    : %3 / %4\n"
                "Interest : %5 / %6\n\n");
    all = all.arg( n2s(nbrActiveReinvesting), n2s(nbrActiveNotReinvesting),
                  n2d_2s(valueActiveReinvesting), n2d_2s(valueActiveNotReinvesting),
                  n2d_2s(annualInterestReinvestment), n2d_2s(annualInterestPayout));
    return all;
}
// all contract summary
int getNbrCreditors()
{   // does not count creditors w/o a contract
    return executeSingleValueSql(qsl("AnzahlDkGeber"), qsl("AnzahlAlleKreditoren")).toInt();
}
int getNbrContracts()
{
    return executeSingleValueSql(qsl("COUNT(*)"), qsl("Vertraege")).toInt();
}
double getValueContracts()
{
    return executeSingleValueSql(qsl("SUM(ABS(Wert))"), qsl("WertAlleVertraege")).toDouble() / 100.;
}
double getWeightedAvgInterestRate()
{
    return executeSingleValueSql(qsl("median"), qsl("GewichteterMittlererZinsAlleVertraege")).toDouble();
}
double getAvgInterestRate()
{
    return executeSingleValueSql(qsl("AVG(Zinssatz)"), qsl("WertAlleVertraege")).toDouble();
}
double getAnnualInterest()
{
    return getWeightedAvgInterestRate()*getValueContracts() /100.;
}
// active vs ...
int getNbrCreditors_activeContracts()
{
    return executeSingleValueSql(qsl("AnzahlDkGeber"), qsl("AnzahlAktiveDkGeber")).toInt();
}
int getNbrActiveContracts()
{
    return executeSingleValueSql(qsl("COUNT(*)"), qsl("WertAktiveVertraege")).toInt();
}
double getValueActiveContracts()
{
    return executeSingleValueSql(qsl("SUM(Wert)"), qsl("WertAktiveVertraege")).toInt() /100.;
}
double getWeightedAvgInterestActiveContracts()
{
    return executeSingleValueSql(qsl("median"), qsl("GewichteterMittlererZinsAktiverVertraege")).toDouble();
}
double getAvgInterestActiveContracts()
{
    return executeSingleValueSql(qsl("AVG(Zinssatz)"), qsl("WertAktiveVertraege")).toDouble();
}
double getAnnualInterestActiveContracts()
{
    return getWeightedAvgInterestActiveContracts() *getValueActiveContracts() /100.;
};
// ... inactive
int getNbrCreditors_inactiveContracts()
{
    return executeSingleValueSql(qsl("AnzahlDkGeber"), qsl("AnzahlInaktiveDkGeber")).toInt();
}
int getNbrInactiveContracts()
{
    return executeSingleValueSql(qsl("COUNT(*)"), qsl("WertPassiveVertraege")).toInt();
}
double getValueInactiveContracts()
{
    return executeSingleValueSql(qsl("SUM(Wert)"), qsl("WertPassiveVertraege")).toDouble() / -100.;
}
double getWeightedAvgInterestInactiveContracts()
{
    return executeSingleValueSql("*", "GewichteterMittlererZinsInaktiverVertraege").toDouble();
}
double getAvgInterestInactiveContracts()
{
    return executeSingleValueSql(qsl("AVG(Zinssatz)"), qsl("WertPassiveVertraege")).toDouble();
}
double getAnnualInterestInactiveContracts()
{
    return getWeightedAvgInterestInactiveContracts() *getValueInactiveContracts() /100.;
}

// active: reinvesting vs. ...
int getNbrActiveReinvesting()
{
    return executeSingleValueSql("COUNT(*)", "WertAktiveVertraege", "thesa").toInt();
}
double getValueActiveReinvesting()
{
    return executeSingleValueSql("SUM(Wert)", "WertAktiveVertraege", "thesa").toInt() /100.;
}
double getAnnualInterestActiveReinvesting()
{
    return executeSingleValueSql(qsl("SUM(Wert*Zinssatz/100)"), qsl("WertAktiveVertraege"), "thesa").toDouble() /100.;
}
// ... wPayout
int getNbrActiveNotReinvesting()
{
    return executeSingleValueSql("COUNT(*)", "WertAktiveVertraege", "NOT thesa").toInt();
}
double getValueActiveNotReinvesting()
{
    return executeSingleValueSql("SUM(Wert)", "WertAktiveVertraege", "NOT thesa").toInt() /100.;
}
double getAnnualInterestActiveNotReinvesting()
{
    return executeSingleValueSql(qsl("SUM(Wert*Zinssatz/100)"), qsl("WertAktiveVertraege"), "NOT thesa").toDouble() /100.;
}
