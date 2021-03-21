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
inline QString toString(const interestModel m) {
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
inline int toInt(const interestModel m) {
    return static_cast<int>(m);
}
inline interestModel fromInt(const int i) {
    if( i < 0 or i >=toInt(interestModel::maxId))
        Q_ASSERT("Invalid interestModel");
    return static_cast<interestModel>(i);
}

struct contract
{
    // static & friends
    static const dbtable& getTableDef();
    static const dbtable& getTableDef_deletedContracts();
    static bool remove(const qlonglong id);
    static QString booking_csv_header();
    inline friend bool operator==(const contract& lhs, const contract& rhs)
    {   // friend functions - even in the class definition - are not member
        bool ret =true;
        if( lhs.td.getRecord().count() not_eq rhs.td.getRecord().count()) {
            qInfo() << "contract comparison: field count mismatch " << lhs.td.getRecord().count() << " / " << rhs.td.getRecord().count();
            ret =false;
        }
        dbtable table =getTableDef();
        for( int i =0; i < table.Fields().count(); i++){
            QString fname =table.Fields()[i].name();
            if( fname == qsl("Zeitstempel"))
                continue;
            if( lhs.td.getValue(fname) == rhs.td.getValue(fname))
                continue;
            else {
                qInfo() << "contract field missmatch " << fname << ": " << lhs.td.getValue(fname) << " / " << rhs.td.getValue(fname);
                ret = false;
            }
        }
        return ret;
    }
    inline friend bool operator!=(const contract& lhs, const contract& rhs)
    {
        return not (lhs==rhs);
    }
    // construction
    contract(const qlonglong id =-1);
    void init();
    void initRandom(const qlonglong creditorId =-1);
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
    void setInterestModel( const interestModel b =interestModel::reinvest) { td.setValue(qsl("thesaurierend"), toInt(b));}
    interestModel iModel() const { return fromInt(td.getValue(qsl("thesaurierend")).toInt());}
    void setNoticePeriod(const int m) { td.setValue(qsl("Kfrist"), m); if( -1 not_eq m) setPlannedEndDate( EndOfTheFuckingWorld);}
    int noticePeriod() const { return td.getValue(qsl("Kfrist")).toInt();}
    bool hasEndDate() const {return -1 == td.getValue(qsl("Kfrist"));}
    void setPlannedEndDate( const QDate& d) { td.setValue(qsl("LaufzeitEnde"), d); if( d not_eq EndOfTheFuckingWorld) setNoticePeriod(-1);}
    QDate plannedEndDate() const { return td.getValue(qsl("LaufzeitEnde")).toDate();}
    void setConclusionDate(const QDate& d) { td.setValue(qsl("Vertragsdatum"), d);}
    QDate conclusionDate() const { return td.getValue(qsl("Vertragsdatum")).toDate();}
    void setComment(const QString& q) {td.setValue(qsl("Anmerkung"), q);}
    QString comment() const {return td.getValue(qsl("Anmerkung")).toString();}
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
    int annualSettlement(const int year);
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
contract saveRandomContract(const qlonglong creditorId);
void saveRandomContracts(const int count);
QDate activateRandomContracts(const int percent);

#endif // VERTRAG_H
