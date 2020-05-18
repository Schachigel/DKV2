#ifndef VERTRAG_H
#define VERTRAG_H

//#include <QtCore>
#include <QString>
#include <QDate>
#include <QVariant>
#include "helper.h"
#include "finhelper.h"
#include "creditor.h"
#include "dkdbhelper.h"


struct contract
{
    // types
    enum contract_status{
        inactive = 1,
        active,
        //terminated
    };
    // construction
    contract() :td(getTableDef()) { setId(-1); setStatus(inactive);};

    // getter & setter
    void setId(qlonglong id) { td.setValue("id", id);}
    qlonglong id() const{ return td.getValue("id").toLongLong();}
    void setCreditorId(qlonglong kid) {td.setValue("kreditorId", kid);}
    qlonglong creditorId() const{ return td.getValue("KreditorId").toLongLong();}
    void setLabel(QString l) { return td.setValue("Kennung", l);}
    QString label() const { return td.getValue("Kennung").toString();};
    void setInterestRate( int percentpercent) {td.setValue("ZSatz", round2digits(double(percentpercent)/100.));}
    void setInterestRate( double percent) {td.setValue("ZSatz", round2digits(percent));}
    double interestRate() const { return td.getValue("ZSatz").toDouble();}
    int interestRateInt() const { return td.getValue("ZSatz").toDouble()*100;}
    void setPlannedInvest(double i) { td.setValue("Betrag", i);}
    double plannedInvest() const { return round2digits(td.getValue("Betrag").toDouble());}
    void setReinvesting( bool b) { td.setValue("thesaurierend", b ? 1: 0);}
    bool reinvesting() const { return (td.getValue("thesaurierend").toInt() != 0);}
    void setStatus(contract_status c) { td.setValue("Status", c);}
    contract_status status() const { return contract_status(td.getValue("Status").toInt());}
    void setNoticePeriod(int m) { td.setValue("Kfrist", m);}
    int noticePeriod() const { return td.getValue("Kfrist").toInt();}
    void setConclusionDate(QDate d) { td.setValue("Vertragsdatum", d);}
    QDate conclusionDate() const { return td.getValue("Vertragsdatum").toDate();}
    void setPlannedEndDate( QDate d) { td.setValue("LaufzeitEnde", d);}
    QDate plannedEndDate() const { return td.getValue("LaufzeitEnde").toDate();}

    // interface
    static const dbtable& getTableDef();
    bool validateAndSaveNewContract(QString& meldung);
    int saveNewContract();

//    bool loadContractFromDb(qlonglong id);
//    bool activateContract(const QDate& aDate);
//    bool bookAnnualInterest(const QDate& YearEnd);
//    bool cancelActiveContract(const QDate& kTermin);
//    bool terminateActiveContract(const QDate& termin);
//    bool deleteInactiveContract();
private:
    // data
    TableDataInserter td;
    // helper
};

#endif // VERTRAG_H
