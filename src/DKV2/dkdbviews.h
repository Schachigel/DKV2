#ifndef DKDBVIEWS_H
#define DKDBVIEWS_H

#include "qsqldatabase.h"
bool createDkDbViews( const QMap<QString, QString>& views, const QSqlDatabase& db =QSqlDatabase::database ());

//  views
extern const QString vnContractView;
extern const QString sqlContractView;
extern const QString vnExContractView;
extern const QString sqlExContractView;
extern const QString vnInvestmentsView;
extern const QString sqlInvestmentsView;

// a nice view to check bookings, not used in the app
extern const QString vnBookingsOverview;

// statistics w/o time dep. saved as views
extern const QMap<QString, QString> views;

    //bool remove_all_views(const QSqlDatabase& db =QSqlDatabase::database());

// annual interest calculation
extern const QString sqlNextAnnualSettlement;
extern const QString sqlContractDataForAnnualSettlement;

// printing lists
extern const QString sqlContractsActiveDetailsView;
extern const QString sqlContractsActiveView;

// uebersichten
// Uebersichten Kurzinfo
extern const QString sqlOverviewActiveContracts;
extern const QString sqlOverviewInActiveContracts;
extern const QString sqlOverviewAllContracts;
// uebersichten Zinsauszahlungen pro Jahr
extern const QString sqlInterestByYearOverview;

extern const QString sqlContractsByYearByInterest;
//// new, time dep. statistics
extern const QString sqlStat_activeContracts_byIMode_toDate;
extern const QString sqlStat_inactiveContracts_byIMode_toDate;
extern const QString sqlStat_finishedContracts_toDate;
extern const QString sqlStat_allContracts_byIMode_toDate;

#endif // DKDBVIEWS_H
