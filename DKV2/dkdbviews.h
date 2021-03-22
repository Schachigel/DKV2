#ifndef DKDBVIEWS_H
#define DKDBVIEWS_H

#include <QString>
#include "helpersql.h"

// create db views

extern const QString vnContractView;
extern const QString vnExContractView;
extern const QString vnInvestmentsView;

extern const QString vnContractsActiveDetailsView;
extern const QString vnVertraege_aktiv;
extern const QString vnContractsInactiveView;
extern const QString vnContractsAllView;

extern const QString vnNextAnnualSettlement_firstAS;
extern const QString vnNextAnnualSettlement_nextAS;
extern const QString vnNextAnnualSettlement;

extern const QString vnContractsByYearByInterest;
extern const QString vnNbrAllCreditors;
extern const QString vnNbrAllCreditors_thesa;
extern const QString vnNbrAllCreditors_payout;
extern const QString vnNbrActiveCreditors;
extern const QString vnNbrActiveCreditors_thesa;
extern const QString vnNbrActiveCreditors_payout;
extern const QString vnInactiveCreditors;
extern const QString vnInactiveCreditors_thesa;
extern const QString vnInactiveCreditors_payout;
extern const QString vnInterestByYearOverview;

extern const QString vnStat_allerVertraege;
extern const QString vnStat_allerVertraege_thesa;
extern const QString vnStat_allerVertraege_ausz;
extern const QString vnStat_aktiverVertraege;
extern const QString vnStat_aktiverVertraege_thesa;
extern const QString vnStat_aktiverVertraege_ausz;
extern const QString vnStat_passiverVertraege;
extern const QString vnStat_passiverVertraege_thesa;
extern const QString vnStat_passiverVertraege_ausz;

extern const QString vnBookingsOverview;

QVector<dbViewDev>& getViews();

#endif // DKDBVIEWS_H
