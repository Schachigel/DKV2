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
extern const QString sqlContractsActiveView;
extern const QString sqlNextAnnualSettlement;
extern const QString sqlContractsByYearByInterest;
extern const QString sqlOverviewActiveContracts;
extern const QString sqlOverviewInActiveContracts;
extern const QString sqlOverviewAllContracts;
//// new, time dep. statistics
extern const QString sqlStat_activeContracts_byIMode_toDate;
extern const QString sqlStat_inactiveContracts_byIMode_toDate;
extern const QString sqlStat_finishedContracts_toDate;
extern const QString sqlStat_allContracts_byIMode_toDate;


#endif // DKDBVIEWS_H
