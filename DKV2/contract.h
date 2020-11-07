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

struct contract
{
    // static & friends
    static const dbtable& getTableDef();
    static const dbtable& getTableDef_deletedContracts();
    static bool remove(qlonglong id);
    static QString booking_csv_header();
    inline friend bool operator==(const contract& lhs, const contract& rhs)
    {   // friend functions - even in the class definition - are not member
        return lhs.td == rhs.td;
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
    void setLabel(QString l) { td.setValue(qsl("Kennung"), l);}
    QString label() const { return td.getValue(qsl("Kennung")).toString();};
    void setInterestRate( double percent) { td.setValue(qsl("ZSatz"), QVariant (qRound(percent * 100.))); }
    double interestRate() const {
        QVariant p(td.getValue(qsl("ZSatz"))); // stored as a int (100th percent)
        return r2(double(p.toInt())/100.);
    }
    void setPlannedInvest(double d) { td.setValue(qsl("Betrag"), ctFromEuro(d));}
    double plannedInvest() const { return euroFromCt( td.getValue(qsl("Betrag")).toInt());}
    void setReinvesting( bool b =true) { td.setValue(qsl("thesaurierend"), b);}
    bool reinvesting() const { return (td.getValue(qsl("thesaurierend")).toInt() != 0);}
    void setNoticePeriod(int m) { td.setValue(qsl("Kfrist"), m); if( -1 != m) setPlannedEndDate( EndOfTheFuckingWorld);}
    int noticePeriod() const { return td.getValue(qsl("Kfrist")).toInt();}
    bool hasEndDate() const {return -1 == td.getValue(qsl("Kfrist"));}
    void setPlannedEndDate( QDate d) { td.setValue(qsl("LaufzeitEnde"), d); if( d != EndOfTheFuckingWorld) setNoticePeriod(-1);}
    QDate plannedEndDate() const { return td.getValue(qsl("LaufzeitEnde")).toDate();}
    void setConclusionDate(QDate d) { td.setValue(qsl("Vertragsdatum"), d);}
    QDate conclusionDate() const { return td.getValue(qsl("Vertragsdatum")).toDate();}

    // interface
    double value() const;
    double value(QDate d) const;
    booking latestBooking();
    // write to db
    int saveNewContract();
/* not used?  int validateAndSaveNewContract(QString& meldung); */
    // contract activation
    bool activate(const QDate& aDate, double amount);
    bool isActive() const;
    QDate activationDate() const;
    // other booking actions
    int annualSettlement(int year, const bool transactual =true);
    bool deposit(QDate d, double amount);
    bool payout(QDate d, double amount);
    bool cancel(QDate);
    bool finalize(bool simulate, const QDate finDate, double& finInterest, double& finPayout);
    // helper
    QString toString(QString name =qsl(""));
private:
    // data
    TableDataInserter td;
    booking latest{ -1 };
    // helper
    bool bookInterest(QDate d);
    bool storeTerminationDate(QDate d) const;
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
