#ifndef DBSTATISTICS_H
#define DBSTATISTICS_H

#include <QStringList>

#include "helper.h"
#include "helperfin.h"
#include "contract.h"

double valueOfAllContracts();


inline bool icmp (const QString& s, const int l, const int r) {
    if( l not_eq r) {
        qInfo() << "comparison of stat.data failed. " << s + qsl(" (%1, %2)").arg(i2s(l), i2s(r));
        return false;
    }
    return true;
}

inline bool dcmp (const QString& s, const double l, const double r) {
    if( not qFuzzyCompare(1. +l, 1.+ r)) {
        qInfo() << "comparison of stat.data failed. " << s + qsl(" (%1, %2)").arg(d2s_4d(l), d2s_4d(r));
        return false;
    }
    return true;
};

inline bool dMaxDiff(const QString& s, const double l, const double r, const double maxDiff) {
    double diff = qAbs(l-r);
    if( diff > maxDiff) {
        qInfo() << "epsilon comparison failed: " << s +qsl(" (%1, %2)").arg(d2s_4d(l), d2s_4d(r));
        return false;
    }
    return true;
}

struct dbStats
{
    enum payoutType {
        t_nt =0, thesa =1, pout =2, pt_size =3
    };
    enum activationStatus {
        act_nInact =0, active =1, inactive =2, as_size =3
    };
    struct dataset
    {
        dataset(const QString& n) { name = n; }
        QString name;
        // int nbrCreditors =0;
        QMap<int, int> credCount;
        int nbrContracts = 0;
        double value = 0.;
        double avgInterestRate = 0.;
        double weightedAvgInterestRate = 0.;
        double annualInterest = 0.;
        inline friend bool operator==(const dataset& lhs, const dataset& rhs) {
            bool ret = true;
            ret &= icmp(lhs.name + qsl(" - nbr Creditors  "), lhs.credCount.size(), rhs.credCount.size());
            ret &= icmp(lhs.name + qsl(" - nbr Contracts  "), lhs.nbrContracts, rhs.nbrContracts);
            ret &= dcmp(lhs.name + qsl(" - Value  (euro)  "), lhs.value, rhs.value);
            ret &= dcmp(lhs.name + qsl(" - annaul int. %  "), lhs.annualInterest, rhs.annualInterest);
            ret &= dcmp(lhs.name + qsl(" - avg Interest % "), lhs.value, rhs.value);
            ret &= dMaxDiff(lhs.name + qsl(" - wAvg Interest% "), lhs.weightedAvgInterestRate, rhs.weightedAvgInterestRate, 0.0002l);
            return ret;
        }
        inline friend bool operator!=(const dataset& lhs, const dataset& rhs) {
            return not (lhs == rhs);
        }
        QString toString() const;
    };

    // all contract summary
    dataset allContracts[activationStatus::as_size]      ={qsl("act+Inact-all  "), qsl("thesa          "), qsl("payout         ")};
    dataset activeContracts[activationStatus::as_size]   ={qsl("active-all     "), qsl("active-thesa   "), qsl("active-payout  ")};
    dataset inactiveContracts[activationStatus::as_size] ={qsl("inactive-all   "), qsl("inactive-thesa "), qsl("inactive-payout")};
    inline friend bool operator==(const dbStats& lhs, const dbStats& rhs)
    {
        for( int i =0; i<3; i++) {
            if( lhs.allContracts[i]     not_eq rhs.allContracts[i] )    return false;
            if( lhs.activeContracts[i]   not_eq rhs.activeContracts[i])   return false;
            if( lhs.inactiveContracts[i] not_eq rhs.inactiveContracts[i]) return false;
        }
        return true;
    }
    inline friend bool operator!=(const dbStats& lhs, const dbStats& rhs) {
        return not (lhs == rhs);
    }
    bool fillall();
    static const bool calculate;
    dbStats(bool init=false) {
        if( init)
            fillall();
    }
    QString toString();
    // for testing
    void addContract(double value, double interest, dbStats::payoutType kind, qlonglong creditorId);
    void activateContract(double value, double plannedInvest, double interestRate, dbStats::payoutType kind, qlonglong creditorId);
    void reinvest(double value, double interestRate, int days);
    void changeContract(double value, double interestRate, int days, dbStats::payoutType kind);
    // EO for testing
private:
    void countCred_addContract(QMap<int, int> &credCount, int id)
    {
        int current =credCount.value(id, -1);
        if( current == -1)
            credCount.insert(id, 1);
        else
            credCount.insert(id, current +1);
    }
    void countCred_removeContract(QMap<int, int> &credCount, int id)
    {
        int current = credCount.value(id, -1);
        if( current == -1) {
            Q_ASSERT(false);// remove creditor that not yet had a contract
        } else if( current == 1) {
            credCount.remove(id);
        } else {
            credCount.insert(id, current-1);
        }
        return;
    }


};
// for testing
inline dbStats getStatistic() {
    dbStats dbs (true);
    qInfo().noquote() << dbs.toString();
    return dbs;
}
// EO for testing

// all contract summary


#endif // DBSTATISTICS_H
