#ifndef VERTRAG_H
#define VERTRAG_H

#include <QString>
#include <QVector>
#include <QDate>

#include "helper.h"
#include "helperfin.h"
#include "tabledatainserter.h"
#include "dkdbhelper.h"
#include "booking.h"
#include "creditor.h"

enum class interestModel {
    payout   =0,
    reinvest =1,
    fixed    =2,
    maxId
};
inline QString toString(interestModel m) {
    switch(m) {
    case interestModel::payout:
        return "auszahlend";
    case interestModel::reinvest:
        return "thesaurierend";
    case interestModel::fixed:
        return "unveränderlich";
    case interestModel::maxId:
    default:
        Q_ASSERT(true);
    }
    return QString();
}
inline int toInt(interestModel m) {
    return static_cast<int>(m);
}
inline interestModel fromInt(int i) {
    if( i < 0 || i >=toInt(interestModel::maxId))
        Q_ASSERT("Invalid interestModel");
    return static_cast<interestModel>(i);
}

struct contract
{
    // static & friends
    static const dbtable& getTableDef();
    static const dbtable& getTableDef_deletedContracts();
    static bool remove(qlonglong id);
    static QString booking_csv_header();
    inline friend bool operator==(const contract& lhs, const contract& rhs)
    {   // friend functions - even in the class definition - are not member
        return lhs.td == rhs.td && lhs.latestB == rhs.latestB;
    }
    inline friend bool operator!=(const contract& lhs, const contract& rhs)
    {
        return !(lhs==rhs);
    }
    // construction
    contract(qlonglong id =-1);
    void init();
    void initRandom(qlonglong creditorId =-1);
    // getter & setter
    void setId(qlonglong id) { td.setValue(qsl("id"), id);}
    qlonglong id() const { return td.getValue(qsl("id")).toLongLong();}
    QString id_aS()   const { return QString::number(id());}
    void setCreditorId(qlonglong kid) {td.setValue(qsl("kreditorId"), kid);}
    qlonglong creditorId() const{ return td.getValue(qsl("KreditorId")).toLongLong();}
    void setLabel(const QString& l) { td.setValue(qsl("Kennung"), l);}
    QString label() const { return td.getValue(qsl("Kennung")).toString();};
    void setInterestRate( const double& percent) { td.setValue(qsl("ZSatz"), QVariant (qRound(percent * 100.))); }
    double interestRate() const {
        QVariant p(td.getValue(qsl("ZSatz"))); // stored as a int (100th percent)
        return r2(double(p.toInt())/100.);
    }
    void setPlannedInvest(const double& d) { td.setValue(qsl("Betrag"), ctFromEuro(d));}
    double plannedInvest() const { return euroFromCt( td.getValue(qsl("Betrag")).toInt());}
    void setInterestModel( interestModel b =interestModel::reinvest) { td.setValue(qsl("thesaurierend"), toInt(b));}
    interestModel interestModel() const { return fromInt(td.getValue(qsl("thesaurierend")).toInt());}
    void setNoticePeriod(int m) { td.setValue(qsl("Kfrist"), m); if( -1 != m) setPlannedEndDate( EndOfTheFuckingWorld);}
    int noticePeriod() const { return td.getValue(qsl("Kfrist")).toInt();}
    bool hasEndDate() const {return -1 == td.getValue(qsl("Kfrist"));}
    void setPlannedEndDate( const QDate& d) { td.setValue(qsl("LaufzeitEnde"), d); if( d != EndOfTheFuckingWorld) setNoticePeriod(-1);}
    QDate plannedEndDate() const { return td.getValue(qsl("LaufzeitEnde")).toDate();}
    void setConclusionDate(const QDate& d) { td.setValue(qsl("Vertragsdatum"), d);}
    QDate conclusionDate() const { return td.getValue(qsl("Vertragsdatum")).toDate();}

    // interface
    // value -> sum of all bookings to a contract
    double value(const QDate& d =EndOfTheFuckingWorld) const;
    // depositValue sum of all deposits to a contract (w/o interest payments)
    double investedValue(const QDate& d =EndOfTheFuckingWorld) const;
    // interestBearingValue depends on interestMode
    double interestBearingValue() const;

    const booking& latestBooking();
    void setLatestBooking( const booking& b) { latestB=b;};
    // write to db
    int saveNewContract();
/* not used?  int validateAndSaveNewContract(QString& meldung); */
    // contract activation
    bool activate(const QDate& aDate, const double& amount);
    bool isActive() const;
    QDate activationDate() const;
    void setActivationDate( const QDate& d) {aDate=d; activated=active;}
    // other booking actions
    QDate nextDateForAnnualSettlement();
    bool needsAnnualSettlement( const QDate& d);
    int annualSettlement(const int year, const bool transactual =true);
    bool deposit(const QDate& d, const double& amount);
    bool payout(const QDate& d, const double& amount);
    bool cancel(const QDate& d);
    bool finalize(const bool simulate, const QDate& finDate, double& finInterest, double& finPayout);
    // helper
    QString toString(QString name =qsl("")) const;

private:
    // data
    TableDataInserter td;
    booking latestB{ -1};
    // helper
    bool bookInBetweenInterest(const QDate& d);
    bool storeTerminationDate(const QDate& d) const;
    bool archive();
    void reset() {init();}
    mutable enum constract_activation_status
         { uninit =-1, passive =0, active =1 } activated=uninit;
    mutable QDate aDate = EndOfTheFuckingWorld;
};

// test helper
contract saveRandomContract(qlonglong creditorId);
void saveRandomContracts(int count);
QDate activateRandomContracts(int percent);

#endif // VERTRAG_H
