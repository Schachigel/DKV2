#ifndef TEST_STATISTICS_H
#define TEST_STATISTICS_H

#include <QObject>
#include <QSqlDatabase>

#include "../DKV2/contract.h"

#include "testhelper.h"

const QString testTemplateDb ={qsl("../data/template.sqlite")};

struct statSet {
    statSet(int co, int cre, double v, double i, double ai)
        : nbrContracts(co), nbrCreditors(cre), volume(v), interest(i), avgInterest(ai){}
    statSet() : nbrContracts(0), nbrCreditors(0), volume(0.), interest(0.), avgInterest(0.){}
    bool operator ==(const statSet &b) const {
        return (nbrContracts ==b.nbrContracts and nbrCreditors ==b.nbrCreditors and
                volume ==b.volume and interest == b.interest and avgInterest == b.avgInterest);
    }
    int           nbrContracts =0;
    int           nbrCreditors =0;
    double        volume =0.;
    double        interest =0.;
    double        avgInterest =0.;
};
typedef QMap<interestModel, statSet> stats;


class test_statistics : public QObject
{
    Q_OBJECT
public:
    explicit test_statistics(QObject *p =nullptr) : QObject(p){};
private:
    // helper functions
signals:
private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    // the actual tests
    void test_start();
};

#endif // TEST_STATISTICS_H
