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
    inline friend bool operator==(const contract& lhs, const contract& rhs)
    {   // friend functions - even in the class definition - are not member
        return lhs.td == rhs.td;
    }
    // construction
    contract(qlonglong id =-1);
    void init();
    // getter & setter
    void setId(qlonglong id) { td.setValue("id", id);}
    qlonglong id() const { return td.getValue("id").toLongLong();}
    QString id_aS()   const { return QString::number(id());}
    void setCreditorId(qlonglong kid) {td.setValue("kreditorId", kid);}
    qlonglong creditorId() const{ return td.getValue("KreditorId").toLongLong();}
    void setLabel(QString l) { td.setValue("Kennung", l);}
    QString label() const { return td.getValue("Kennung").toString();};
    void setInterest100th( int percentpercent) {td.setValue("ZSatz", percentpercent);}
    void setInterestRate( double percent) {td.setValue("ZSatz", int(percent*100));}
    double interestRate() const { return double(td.getValue("ZSatz").toInt())/100.;}
    void setPlannedInvest(double d) { td.setValue("Betrag", ctFromEuro(d));}
    double plannedInvest() const { return euroFromCt( td.getValue("Betrag").toInt());}
    void setReinvesting( bool b =true) { td.setValue("thesaurierend", b);}
    bool reinvesting() const { return (td.getValue("thesaurierend").toInt() != 0);}
    void setNoticePeriod(int m) { td.setValue("Kfrist", m); if( -1 != m) setPlannedEndDate( EndOfTheFuckingWorld);}
    int noticePeriod() const { return td.getValue("Kfrist").toInt();}
    bool hasEndDate() const {return -1 == td.getValue("Kfrist");}
    void setPlannedEndDate( QDate d) { td.setValue("LaufzeitEnde", d); if( d != EndOfTheFuckingWorld) setNoticePeriod(-1);}
    QDate plannedEndDate() const { return td.getValue("LaufzeitEnde").toDate();}
    void setConclusionDate(QDate d) { td.setValue("Vertragsdatum", d);}
    QDate conclusionDate() const { return td.getValue("Vertragsdatum").toDate();}

    // interface
    double value() const;
    double value(QDate d) const;
    QDate latestBooking() const;
    // write to db
    int saveNewContract();
    bool validateAndSaveNewContract(QString& meldung);
    // contract activation
    bool activate(const QDate& aDate, int amount_ct) const;
    bool activate(const QDate& aDate, double amount) const;
    bool isActive() const;
    QDate activationDate() const;
    // booking actions
    int annualSettlement() const;
    bool bookInterest(QDate d) const;
    bool deposit(QDate d, double amount) const;
    bool payout(QDate d, double amount) const;
    bool finalize(bool simulate, const QDate finDate, double& finInterest, double& finPayout);
private:
    // data
    TableDataInserter td;
};

// test helper
contract saveRandomContract(qlonglong creditorId);
void saveRandomContracts(int count);
void activateRandomContracts(int percent);

#endif // VERTRAG_H
