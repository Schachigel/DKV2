#ifndef VERTRAG_H
#define VERTRAG_H

#include "helper.h"
#include "helperfin.h"
#include "tabledatainserter.h"
#include "booking.h"

enum class interestModel
{
    payout = 0,
    reinvest /*= 1*/,
    fixed /*= 2*/,
    zero  /*= 3*/,
    maxId /*= 4*/,
    allIModels = maxId
};
inline QString interestModelDisplayString(const interestModel m) {
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
inline interestModel interestModelFromInt(const int i) {
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
    // Vergleichsoperatoren für TESTS !!
    friend bool operator==(const contract& lhs, const contract& rhs);
    friend bool operator!=(const contract& lhs, const contract& rhs);

    // construction
    contract(const tableindex_t CONTRACTid =SQLITE_invalidRowId, bool isTerminated =false);
    void loadFromDb(const tableindex_t id);
    void loadExFromDb(const tableindex_t id);
    void initContractDefaults(const tableindex_t creditorId =SQLITE_invalidRowId);
    void initRandom(const tableindex_t creditorId =SQLITE_invalidRowId);

    // getter & setter
    void setId(tableindex_t id) { td.setValue(qsl("id"), id);}
    tableindex_t id() const { return td.getValue(qsl("id")).toLongLong();}
    QString id_aS()   const { Q_ASSERT( id() >= SQLITE_minimalRowId); return i2s(id());}
    void setCreditorId(tableindex_t kid) {td.setValue(fnKreditorId, kid);}
    tableindex_t creditorId() const{ return td.getValue(fnKreditorId).toLongLong();}

    void setLabel(const QString &l) { td.setValue(fnKennung, l); }
    QString label() const { return td.getValue(fnKennung).toString();};

    void setInterestRate( const double percent) {
        td.setValue(fnZSatz, QVariant (qRound(percent * 100.)));
        if( percent == 0) td.setValue(fnThesaurierend, toInt(interestModel::zero));
    }
    double interestRate() const {
        QVariant p {td.getValue(fnZSatz)}; // stored as a int (100th percent)
        return dbInterest2Interest (p.toInt());
    }
    int dbInterest() const {
        return td.getValue(fnZSatz).toInt();
    }
    double actualInterestRate() const {
        if( interestActive()) return interestRate();
        else return 0.;
    }

    void setInvestmentId(tableindex_t rId) { td.setValue(fnAnlagenId, (isValidRowId (rId)?QVariant(rId):QVariant::fromValue((nullptr))));}
    tableindex_t investmentId() const { return td.getValue(fnAnlagenId).toLongLong();}

    void setPlannedInvest(const double d) { td.setValue(fnBetrag, ctFromEuro(d));}
    double plannedInvest() const { return euroFromCt( td.getValue(fnBetrag).toInt());}

    void setInterestModel( const interestModel b =interestModel::reinvest) {
        td.setValue(fnThesaurierend, toInt(b));
        if( b == interestModel::zero) td.setValue(fnZSatz, 0);
    }
    interestModel iModel() const { return interestModelFromInt(td.getValue(fnThesaurierend).toInt());}

    void setNoticePeriod(const int m) { td.setValue(fnKFrist, m); if( -1 not_eq m) setPlannedEndDate( EndOfTheFuckingWorld);}
    int noticePeriod() const { return td.getValue(fnKFrist).toInt();}

    bool hasEndDate() const {return -1 == td.getValue(fnKFrist);}
    void setPlannedEndDate( const QDate d) { td.setValue(fnLaufzeitEnde, d); if( d not_eq EndOfTheFuckingWorld) setNoticePeriod(-1);}
    QDate plannedEndDate() const { return td.getValue(fnLaufzeitEnde).toDate();}

    void setConclusionDate(const QDate d) { td.setValue(fnVertragsDatum, d);}
    QDate conclusionDate() const { return td.getValue(fnVertragsDatum).toDate();}

    void setInterestActive(bool active){ td.setValue(fnZAktiv, active);}
    bool interestActive() const { return td.getValue(fnZAktiv).toBool();}

    void setComment(const QString& q) {td.setValue(fnAnmerkung, q);}
    QString comment() const {return td.getValue(fnAnmerkung).toString();}

    // interface
    // value -> sum of all bookings to a contract
    double value(const QDate d =EndOfTheFuckingWorld) const;
    // sum of all deposits to a contract (w/o interest payments)
    double investedValue(const QDate d =EndOfTheFuckingWorld) const;
    // interestBearingValue depends on interestMode
    double interestBearingValue() const;
    const booking latestBooking();

    // write to db
    tableindex_t saveNewContract();
    bool updateComment(const QString&);
    bool updateTerminationDate(QDate termination, int noticePeriod);
    bool updateInvestment(tableindex_t id);
    bool deleteInactive();

    // contract activation
    bool bookInitialPayment(const QDate aDate, double amount);
    bool initialBookingReceived() const;
    bool bookActivateInterest(const QDate d);

    // other booking actions
    QDate nextDateForAnnualSettlement();
    bool needsAnnualSettlement( const QDate d);
    int annualSettlement(const int year);
    bool deposit(const QDate d, double amount, bool payoutInterest =false);
    bool payout(const QDate d, double amount, bool payoutInterest =false);
    bool cancel(const QDate d);
    bool finalize(const bool simulate, const QDate finDate, double& finInterest, double& finPayout);
    // helper
    QString toString(const QString &name =QString()) const;
    QVariantMap toVariantMap(QDate fromDate = BeginingOfTime, QDate toDate = EndOfTheFuckingWorld);
    double payedInterestAtTermination();
    double getAnnualInterest(int year, bookingType interestType = bookingType::annualInterestDeposit);

    // allow contract objects from deleted contracts
    bool isTerminated =false;
private:
    bool updateSetInterestActive();

    // data
    TableDataInserter td;
    // helper
    // moved to helperfin.cpp double ZinsesZins(const double zins, const double wert,const QDate von, const QDate bis, const bool thesa =true);

    bool bookInBetweenInterest(const QDate d, bool payout =false);
    bool storeTerminationDate(const QDate d) const;
    bool archive();
    void reset() {initContractDefaults();}
};

// test helper
contract saveRandomContract(const tableindex_t creditorId);
void saveRandomContractPerCreditor();
void saveRandomContracts(const int count);
int activateAllContracts(int year);
QDate activateRandomContracts(const int percent);

#endif // VERTRAG_H
