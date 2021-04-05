#ifndef DKDBVIEWS_H
#define DKDBVIEWS_H

#include <QString>
#include "helpersql.h"

// create db views
// views used as table in tableViews
extern const QString vnContractView;
extern const QString vnExContractView;
extern const QString vnInvestmentsView;
// a nice view to check bookings, not used in the app
extern const QString vnBookingsOverview;
// statistics w/o time dep. saved as views
extern const QString vnStat_activeContracts_byIMode;
extern const QString vnStat_inactiveContracts_byIMode;
const QMap<QString, QString>& getViews();

bool remove_all_views(const QSqlDatabase& db =QSqlDatabase::database());

extern const QString sqlContractsActiveDetailsView;
extern const QString sqlVertraege_aktiv;
extern const QString sqlContractsInactiveView;
extern const QString sqlContractsActiveView;
extern const QString sqlContractsAllView;
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
extern const QString sqlInterestByYearOverview;
extern const QString sqlStat_allerVertraege;
extern const QString sqlStat_allerVertraege_thesa;
extern const QString sqlStat_allerVertraege_ausz;
extern const QString sqlStat_aktiverVertraege;
extern const QString sqlStat_aktiverVertraege_thesa;
extern const QString sqlStat_aktiverVertraege_ausz;
extern const QString sqlStat_passiverVertraege;
extern const QString sqlStat_passiverVertraege_thesa;
extern const QString sqlStat_passiverVertraege_ausz;

// new, time dep. statistics
extern const QString sqlStat_activeContracts_byIMode_toDate;
extern const QString sqlStat_inactiveContracts_byIMode_toDate;
extern const QString sqlStat_allContracts_byIMode_toDate;


#endif // DKDBVIEWS_H
