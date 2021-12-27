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
    zero     =3,
    maxId    =4,
    allIModels =maxId
};
inline QString toString(const interestModel m) {
    switch(m) {
    case interestModel::payout:
        return qsl("auszahlend");
    case interestModel::reinvest:
        return qsl("ansparend");
    case interestModel::fixed:
        return qsl(" fest ");
    case interestModel::zero:
        return qsl("zinslos");
    //case interestModel::maxId:
    case interestModel::allIModels:
        return qsl("Alle");
    default:
        Q_ASSERT(false);
    }
    return QString();
}
inline int toInt(const interestModel m) {
    return static_cast<int>(m);
}
inline interestModel fromInt(const int i) {
    if( i < 0 or i >toInt(interestModel::maxId))
        Q_ASSERT(not "Invalid interestModel");
    return static_cast<interestModel>(i);
}

struct contract
{
    // field names
    static const QString tnContracts;
    static const QString tnExContracts;
    static const QString fnId;
    static const QString fnKreditorId;
    static const QString fnKennung;
    static const QString fnAnmerkung;
    static const QString fnZSatz;
    static const QString fnBetrag;
    static const QString fnThesaurierend;
    static const QString fnVertragsDatum;
    static const QString fnKFrist;
    static const QString fnAnlagenId;
    static const QString fnLaufzeitEnde;
    static const QString fnZAktiv;
    static const QString fnZeitstempel;
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
            QString fname =table.Fields().value(i).name();
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
    contract(const qlonglong CONTRACTid =-1);
    void loadFromDb(const qlonglong id);
    void initContractDefaults(const qlonglong creditorId =-1);
    void initRandom(const qlonglong creditorId =-1);

    // getter & setter
    void setId(qlonglong id) { td.setValue(qsl("id"), id);}
    qlonglong id() const { return td.getValue(qsl("id")).toLongLong();}
    QString id_aS()   const { return QString::number(id());}

    void setCreditorId(qlonglong kid) {td.setValue(fnKreditorId, kid);}
    qlonglong creditorId() const{ return td.getValue(fnKreditorId).toLongLong();}

    void setLabel(const QString& l) { td.setValue(fnKennung, l);}
    QString label() const { return td.getValue(fnKennung).toString();};

    void setInterestRate( const double percent) {
        td.setValue(fnZSatz, QVariant (qRound(percent * 100.)));
        if( percent == 0) td.setValue(fnThesaurierend, toInt(interestModel::zero));
    }
    double interestRate() const {
        QVariant p=td.getValue(fnZSatz); // stored as a int (100th percent)

        double iRate = r2(double(p.toInt())/100.);
        return iRate;
    }
    double actualInterestRate() const {
        if( interestActive()) return interestRate();
        else return 0.;
    }

    void setInvestment(qlonglong rId) { td.setValue(fnAnlagenId, (rId>0?QVariant(rId):QVariant()));}
    qlonglong investment() const { return td.getValue(fnAnlagenId).toLongLong();}

    void setPlannedInvest(const double d) { td.setValue(fnBetrag, ctFromEuro(d));}
    double plannedInvest() const { return euroFromCt( td.getValue(fnBetrag).toInt());}

    void setInterestModel( const interestModel b =interestModel::reinvest) {
        td.setValue(fnThesaurierend, toInt(b));
        if( b == interestModel::zero) td.setValue(fnZSatz, 0);
    }
    interestModel iModel() const { return fromInt(td.getValue(fnThesaurierend).toInt());}

    void setNoticePeriod(const int m) { td.setValue(fnKFrist, m); if( -1 not_eq m) setPlannedEndDate( EndOfTheFuckingWorld);}
    int noticePeriod() const { return td.getValue(fnKFrist).toInt();}

    bool hasEndDate() const {return -1 == td.getValue(fnKFrist);}
    void setPlannedEndDate( const QDate d) { td.setValue(fnLaufzeitEnde, d); if( d not_eq EndOfTheFuckingWorld) setNoticePeriod(-1);}
    QDate plannedEndDate() const { return td.getValue(fnLaufzeitEnde).toDate();}
    void setConclusionDate(const QDate d) { td.setValue(fnVertragsDatum, d);}
    QDate conclusionDate() const { return td.getValue(fnVertragsDatum).toDate();}

    void setInterestActive(bool active){ td.setValue(fnZAktiv, active);}
    bool updateInterestActive(bool a);
    bool interestActive() const { return td.getValue(fnZAktiv).toBool();}

    void setComment(const QString& q) {td.setValue(fnAnmerkung, q);}
    QString comment() const {return td.getValue(fnAnmerkung).toString();}
    // interface
    // value -> sum of all bookings to a contract
    double value(const QDate d =EndOfTheFuckingWorld) const;
    // depositValue sum of all deposits to a contract (w/o interest payments)
    double investedValue(const QDate d =EndOfTheFuckingWorld) const;
    // interestBearingValue depends on interestMode
    double interestBearingValue() const;

    const booking& latestBooking();
    void setLatestBooking( const booking& b) { latestB=b;};
    // write to db
    int saveNewContract();
    bool updateComment(const QString&);
    bool updateTerminationDate(QDate termination, int noticePeriod);
    bool updateInvestment(qlonglong id);
/* not used?  int validateAndSaveNewContract(QString& meldung); */
    // contract activation
    bool bookInitialPayment(const QDate aDate, double amount);
    bool initialBookingReceived() const;
    QDate initialBookingDate() const;
    void setInitialBookingDate( const QDate d) {aDate=d; activated=active;}

    bool bookActivateInterest(QDate d);

    // other booking actions
    QDate nextDateForAnnualSettlement();
    bool needsAnnualSettlement( const QDate d);
    int annualSettlement(const int year);
    bool deposit(const QDate d, double amount);
    bool payout(const QDate d, double amount);
    bool cancel(const QDate d);
    bool finalize(const bool simulate, const QDate finDate, double& finInterest, double& finPayout);
    // helper
    QString toString(const QString &name =qsl("")) const;
    QVariant toVariantMap_4annualBooking(int year=9999);
    double payedInterest(int year);

private:
    // data
    TableDataInserter td;
    booking latestB;
    // helper
    double ZinsesZins(const double zins, const double wert,const QDate von, const QDate bis, const bool thesa =true);

    bool bookInBetweenInterest(const QDate d);
    bool storeTerminationDate(const QDate d) const;
    bool archive();
    void reset() {initContractDefaults();}
    mutable enum constract_activation_status
         { uninit =-1, passive =0, active =1 } activated=uninit;
    mutable QDate aDate = EndOfTheFuckingWorld;
};

// test helper
contract saveRandomContract(const qlonglong creditorId);
void saveRandomContracts(const int count);
QDate activateRandomContracts(const int percent);

#endif // VERTRAG_H
