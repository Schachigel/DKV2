
#include "helperfin.h"
#include "helpersql.h"
#include "dkdbviews.h"
#include "dbfield.h"
#include "dbstatistics.h"

const bool dbStats::calculate =true;

QString dbStats::dataset::toString() const
{
    QString all;
    QTextStream sout(&all);
    sout.setRealNumberNotation(QTextStream::RealNumberNotation::FixedNotation);
    sout.setRealNumberPrecision(4);
    sout.setFieldWidth(3);
    sout << qsl("#Cred      :")   << credCount.size()
         << qsl("; #Cont      :") << nbrContracts
         << qsl("; Value   (e):") << qSetFieldWidth(10) << d2s_2d(value)
         << qsl("; avgInter(%):") << qSetFieldWidth(7) << d2s_4d(avgInterestRate)
         << qsl("; wAvgInt (%):") << qSetFieldWidth(7) << d2s_4d(weightedAvgInterestRate)
         << qsl("; annualI (e):") << qSetFieldWidth(5) << d2s_2d(annualInterest);
    sout.flush();
    return all;
}

bool datasetFromViews(dbStats::dataset& ds, QString statsView, const QString& creditorNbrView)
{   LOG_CALL_W(statsView);

    QString sql =statsView;
    QSqlRecord rec =executeSingleRecordSql(sql);

    if( rec.isEmpty())
        return false;
    ds.nbrContracts = rec.value(qsl("Anzahl")).toInt();
    ds.value = rec.value(qsl("Wert")).toDouble();
    ds.annualInterest = rec.value(qsl("Jahreszins")).toDouble();
    ds.avgInterestRate = rec.value(qsl("mittlereRate")).toDouble();
    ds.weightedAvgInterestRate = rec.value(qsl("gewMittel")).toDouble();

    QString sqlCNbr {qsl("SELECT Anzahl FROM (%1)").arg(creditorNbrView)};
    int nbrCreditors = executeSingleValueSql(sqlCNbr).toInt();
    for( int i=0; i<nbrCreditors; i++)
        ds.credCount.insert(i, 1);
    return true;
}

bool dbStats::fillall()
{   LOG_CALL;
    datasetFromViews(allContracts[0], sqlStat_allerVertraege,       sqlNbrAllCreditors);
    datasetFromViews(allContracts[1], sqlStat_allerVertraege_thesa, sqlNbrAllCreditors_thesa);
    datasetFromViews(allContracts[2], sqlStat_allerVertraege_ausz,  sqlNbrAllCreditors_payout);

    datasetFromViews(activeContracts[0], sqlStat_aktiverVertraege,       sqlNbrActiveCreditors);
    datasetFromViews(activeContracts[1], sqlStat_aktiverVertraege_thesa, sqlNbrActiveCreditors_thesa);
    datasetFromViews(activeContracts[2], sqlStat_aktiverVertraege_ausz,  sqlNbrActiveCreditors_payout);

    datasetFromViews(inactiveContracts[0], sqlStat_passiverVertraege,       sqlInactiveCreditors);
    datasetFromViews(inactiveContracts[1], sqlStat_passiverVertraege_thesa, sqlInactiveCreditors_thesa);
    datasetFromViews(inactiveContracts[2], sqlStat_passiverVertraege_ausz,  sqlInactiveCreditors_payout);

    return false;
}

QString dbStats::toString()
{
    QString all;
    QTextStream sout(&all);
    sout << Qt::endl;
    sout << "Alle Verträge              :" << allContracts[0].toString() << Qt::endl;
    sout << "Alle thesa. Verträge       :" << allContracts[1].toString() << Qt::endl;
    sout << "Alle ausz.  Verträge       :" << allContracts[2].toString() << Qt::endl;

    sout << "Alle akt. Verträge         :" << activeContracts[0].toString() << Qt::endl;
    sout << "Alle akt. thesa. Verträge  :" << activeContracts[1].toString() << Qt::endl;
    sout << "Alle akt. ausz.  Verträge  :" << activeContracts[2].toString() << Qt::endl;

    sout << "Alle inakt. Verträge       :" << inactiveContracts[0].toString() << Qt::endl;
    sout << "Alle inakt. thesa. Verträge:" << inactiveContracts[1].toString() << Qt::endl;
    sout << "Alle inakt. ausz.  Verträge:" << inactiveContracts[2].toString() << Qt::endl;
    sout.flush();

    return all;
}

void dbStats::addContract(double value, double interest, dbStats::payoutType kind, qlonglong credId)
{   LOG_CALL;
    Q_ASSERT(kind not_eq dbStats::t_nt);
    dbStats old =*this;
    countCred_addContract( allContracts[t_nt].credCount, credId);
    countCred_addContract( inactiveContracts[t_nt].credCount, credId);
    countCred_addContract( allContracts[kind].credCount, credId);
    countCred_addContract(inactiveContracts[kind].credCount, credId);

    double annualInterest = r2(value *interest /100.);
    // nbr Contracts
    allContracts[dbStats::t_nt].nbrContracts +=1;
    allContracts[kind].nbrContracts +=1;
    inactiveContracts[dbStats::t_nt].nbrContracts +=1;
    inactiveContracts[kind].nbrContracts +=1;
    // value...
    allContracts[dbStats::t_nt].value +=value;
    allContracts[kind].value         +=value;
    inactiveContracts[dbStats::t_nt].value +=value;
    inactiveContracts[kind].value         +=value;
    // avgInterrest
    allContracts[dbStats::t_nt].avgInterestRate =
        r4((old.allContracts[dbStats::t_nt].avgInterestRate * old.allContracts[dbStats::t_nt].nbrContracts +interest)
         / (old.allContracts[dbStats::t_nt].nbrContracts +1l));
    allContracts[kind].avgInterestRate =
        r4((old.allContracts[kind].avgInterestRate * old.allContracts[kind].nbrContracts +interest)
         / (old.allContracts[kind].nbrContracts+1l));
    inactiveContracts[dbStats::t_nt].avgInterestRate =
        r4((old.inactiveContracts[dbStats::t_nt].avgInterestRate * old.inactiveContracts[dbStats::t_nt].nbrContracts +interest)
         / (old.inactiveContracts[dbStats::t_nt].nbrContracts+1l));
    inactiveContracts[kind].avgInterestRate =
        r4((old.inactiveContracts[kind].avgInterestRate * old.inactiveContracts[kind].nbrContracts +interest)
         / (old.inactiveContracts[kind].nbrContracts+1l));
    // weighted average
    allContracts[dbStats::t_nt].weightedAvgInterestRate =r6(100. *
                                                           (old.allContracts[dbStats::t_nt].annualInterest +annualInterest)
                                                           / (old.allContracts[dbStats::t_nt].value +value));
    allContracts[kind].weightedAvgInterestRate = r6(100. *
                                                  (old.allContracts[kind].annualInterest +annualInterest)
                                                  / (old.allContracts[kind].value +value));
    inactiveContracts[dbStats::t_nt].weightedAvgInterestRate = r6(100*
                                                                (old.inactiveContracts[dbStats::t_nt].annualInterest +annualInterest)
                                                                / (old.inactiveContracts[dbStats::t_nt].value +value));
    inactiveContracts[kind].weightedAvgInterestRate =r6(100.*
                                                       (old.inactiveContracts[kind].annualInterest +annualInterest)
                                                       / (old.inactiveContracts[kind].value +value));
    // annualinterest
    allContracts[dbStats::t_nt].annualInterest += annualInterest;
    allContracts[kind]        .annualInterest += annualInterest;
    inactiveContracts[dbStats::t_nt].annualInterest +=annualInterest;
    inactiveContracts[kind]        .annualInterest +=annualInterest;
}
void dbStats::activateContract(double value, double plannedInvest, double interestRate, dbStats::payoutType kind, qlonglong credId)
{
    dbStats old =*this;
    countCred_removeContract( inactiveContracts[t_nt].credCount, credId);
    countCred_addContract(    activeContracts[t_nt].credCount, credId);
    countCred_removeContract( inactiveContracts[kind].credCount, credId);
    countCred_addContract(    activeContracts[kind].credCount, credId);

    double changeInInvestment =value -plannedInvest;
    double geplanterJahreszinsEuro  =r2(plannedInvest *interestRate);
    double aktivierterJahreszinsEuro=r2(value *interestRate);
    allContracts[dbStats::t_nt].value += changeInInvestment;
    { // all contracts w and w/o reinvest
    double bisherigerJahreszinsEuro =r2(old.allContracts[dbStats::t_nt].weightedAvgInterestRate *old.allContracts[dbStats::t_nt].value);
    double neuerGesamtwertEuro      =r2(old.allContracts[dbStats::t_nt].value +value -plannedInvest);
    allContracts[dbStats::t_nt].weightedAvgInterestRate = r4((bisherigerJahreszinsEuro -geplanterJahreszinsEuro +aktivierterJahreszinsEuro) /neuerGesamtwertEuro);
    allContracts[dbStats::t_nt].annualInterest += aktivierterJahreszinsEuro -geplanterJahreszinsEuro;
    }
    { // all contracts w same payout schema
    double bisherigerJahreszinsEuro =r2(old.allContracts[kind].weightedAvgInterestRate *old.allContracts[kind].value );
    double neuerGesamtwertEuro      =r2(old.allContracts[kind].value +changeInInvestment);
    allContracts[kind].value += changeInInvestment;
    allContracts[kind].weightedAvgInterestRate =r4(
        (bisherigerJahreszinsEuro -geplanterJahreszinsEuro +aktivierterJahreszinsEuro)
        /neuerGesamtwertEuro);
    allContracts[kind].annualInterest += r2(value *interestRate /100.) -r2(plannedInvest *interestRate /100.);
    }
    activeContracts[dbStats::t_nt].nbrContracts +=1;
    activeContracts[kind].nbrContracts +=1;
    inactiveContracts[dbStats::t_nt].nbrContracts -=1;
    inactiveContracts[kind].nbrContracts -=1;

    activeContracts[dbStats::t_nt].value   +=value;
    activeContracts[kind].value            +=value;
    inactiveContracts[dbStats::t_nt].value -=plannedInvest;
    inactiveContracts[kind].value          -=plannedInvest;

    activeContracts[dbStats::t_nt].avgInterestRate =
        (old.activeContracts[dbStats::t_nt].avgInterestRate *old.activeContracts[dbStats::t_nt].nbrContracts +interestRate)
         /activeContracts[dbStats::t_nt].nbrContracts;
    activeContracts[kind].avgInterestRate =
        (old.activeContracts[kind].avgInterestRate *old.activeContracts[kind].nbrContracts +interestRate)
         /activeContracts[kind].nbrContracts;
    inactiveContracts[dbStats::t_nt].avgInterestRate =
        (old.inactiveContracts[dbStats::t_nt].avgInterestRate *old.inactiveContracts[dbStats::t_nt].nbrContracts -interestRate)
         /inactiveContracts[dbStats::t_nt].nbrContracts;
    inactiveContracts[kind].avgInterestRate =
        (old.inactiveContracts[kind].avgInterestRate *old.inactiveContracts[kind].nbrContracts -interestRate)
         /inactiveContracts[kind].nbrContracts;

    activeContracts[dbStats::t_nt].weightedAvgInterestRate = r6(100.*
           (r2(old.activeContracts[dbStats::t_nt].weightedAvgInterestRate *old.activeContracts[dbStats::t_nt].value /100.) +r2(interestRate *value /100.))
            /(old.activeContracts[dbStats::t_nt].value +value));
    activeContracts[kind].weightedAvgInterestRate = r6(100.*
           (r2(old.activeContracts[kind].weightedAvgInterestRate *old.activeContracts[kind].value /100.) +r2(interestRate *value /100.))
            /(old.activeContracts[kind].value +value));

    double oldAnnualInterest =r2(old.inactiveContracts[dbStats::t_nt].weightedAvgInterestRate *old.inactiveContracts[dbStats::t_nt].value /100.);
    double annualInterestOfActivatedContract =r2(interestRate *plannedInvest /100.);
    double newAnnualInterest =old.inactiveContracts[dbStats::t_nt].value -plannedInvest;

    inactiveContracts[dbStats::t_nt].weightedAvgInterestRate = dcmp(QString(), oldAnnualInterest,
                                                                    annualInterestOfActivatedContract) ? 0. :
                                                             r6(100.* (oldAnnualInterest-annualInterestOfActivatedContract)/newAnnualInterest);

    inactiveContracts[kind].weightedAvgInterestRate =
        dcmp(QString(), old.inactiveContracts[kind].value,
             plannedInvest) ? 0. :
             r6(100.*
               (r2(old.inactiveContracts[kind].weightedAvgInterestRate *old.inactiveContracts[kind].value /100.) -r2(interestRate *plannedInvest /100.))
                /(old.inactiveContracts[kind].value -plannedInvest));

    activeContracts[dbStats::t_nt].annualInterest =old.activeContracts[dbStats::t_nt].annualInterest +(value *interestRate /100.);
    activeContracts[kind].annualInterest =old.activeContracts[kind].annualInterest +(value *interestRate /100.);
    inactiveContracts[dbStats::t_nt].annualInterest =old.inactiveContracts[dbStats::t_nt].annualInterest -(plannedInvest *interestRate /100.);
    inactiveContracts[kind].annualInterest =old.inactiveContracts[kind].annualInterest -(plannedInvest *interestRate /100.);
}
void dbStats::reinvest(double oldContValue, double interestRate, int days)
{
    double oldValue =allContracts[t_nt].value;
    double oldAnnualInterest =r2(allContracts[t_nt].weightedAvgInterestRate *oldValue /100.);
    const double interest = r2(days /360. *oldContValue *interestRate /100. );
    const double newContValue =oldContValue +interest;
    const double newAnnualInterest =r2((newContValue -oldContValue)*interestRate /100.);
    allContracts[t_nt].value +=interest;
    allContracts[t_nt].annualInterest +=newAnnualInterest;
    allContracts[t_nt].weightedAvgInterestRate =r6(100. *(oldAnnualInterest +newAnnualInterest) /(oldValue +interest));

    oldValue =allContracts[thesa].value;
    oldAnnualInterest =r2(allContracts[thesa].weightedAvgInterestRate *oldValue /100.);
    allContracts[thesa].value +=interest;
    allContracts[thesa].annualInterest +=newAnnualInterest;
    allContracts[thesa].weightedAvgInterestRate =r6(100. *(oldAnnualInterest +newAnnualInterest) /(oldValue +interest));

    // activeContracts...
    oldValue =activeContracts[t_nt].value;
    oldAnnualInterest =r2(activeContracts[t_nt].weightedAvgInterestRate *oldValue /100.);
    activeContracts[t_nt].value +=interest;
    activeContracts[t_nt].annualInterest +=newAnnualInterest;
    activeContracts[t_nt].weightedAvgInterestRate =r6(100. *(oldAnnualInterest +newAnnualInterest) /(oldValue +interest));

    oldValue =activeContracts[thesa].value;
    oldAnnualInterest =r2(activeContracts[thesa].weightedAvgInterestRate *oldValue /100.);
    activeContracts[thesa].value +=interest;
    activeContracts[thesa].annualInterest +=newAnnualInterest;
    activeContracts[thesa].weightedAvgInterestRate =r6(100. *(oldAnnualInterest +newAnnualInterest) /(oldValue +interest));
}

void dbStats::changeContract(double value, double interestRate, int days, dbStats::payoutType kind)
{
    double interest =allContracts[t_nt].value *interestRate /100. *days /360.;

    allContracts[t_nt].value +=value +interest;
    allContracts[t_nt].annualInterest =r2(allContracts[t_nt].value *interestRate /100.);
    allContracts[t_nt].weightedAvgInterestRate =r6(100. *allContracts[t_nt].annualInterest /allContracts[t_nt].value);

    allContracts[kind].value +=value +interest;
    allContracts[kind].annualInterest =r2(allContracts[kind].value *interestRate /100.);
    allContracts[kind].weightedAvgInterestRate =r6(100. *allContracts[kind].annualInterest /allContracts[kind].value);

    activeContracts[t_nt].value +=value +interest;
    activeContracts[t_nt].annualInterest =r2(activeContracts[t_nt].value *interestRate /100.);
    activeContracts[t_nt].weightedAvgInterestRate =r6(100. *activeContracts[t_nt].annualInterest /activeContracts[t_nt].value);

    activeContracts[kind].value +=value +interest;
    activeContracts[kind].annualInterest =r2(activeContracts[kind].value *interestRate /100.);
    activeContracts[kind].weightedAvgInterestRate =r6(100. *activeContracts[kind].annualInterest /activeContracts[kind].value);
}

// all contract summary
