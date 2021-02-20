#ifndef DKDBVIEWS_H
#define DKDBVIEWS_H

#include <QString>
// create db views
struct dbViewDev{
    const QString name;
    const QString sql;
};

extern const QString sqlContractView;
extern const QString sqlExContractView;
extern const QString sqlBookingsOverview;
extern const QString sqlContractsActiveDetailsView;
extern const QString sqlContractsActiveView;
extern const QString sqlContractsInactiveView;
extern const QString sqlContractsAllView;
extern const QString sqlNextAnnualSettlement_firstAS;
extern const QString sqlNextAnnualSettlement_nextAS;
extern const QString sqlNextAnnualSettlement;
extern const QString sqlContractsByYearByInterest;
extern const QString sqlNbrAllCreditors;
extern const QString sqlNbrAllCreditors_thesa;
extern const QString sqlNbrAllCreditors_payout;
extern const QString sqlNbrActiveCreditors;
extern const QString sqlNbrActiveCreditors_thesa;
extern const QString sqlNbrActiveCreditors_payout;
extern const QString sqlInactiveCreditors;
extern const QString sqlInactiveCreditors_thesa;
extern const QString sqlInactiveCreditors_payout;

extern QVector<dbViewDev> views;


#endif // DKDBVIEWS_H
