
#include <QDate>
#include <QString>
#include <QMessageBox>
#include <QDate>

#include "helper.h"
#include "booking.h"
#include "wizchangecontractvalue.h"
#include "wizactivatecontract.h"
#include "wizterminatecontract.h"
#include "wizannualsettlement.h"
#include "transaktionen.h"


void activateContract(qlonglong cid)
{   LOG_CALL;
    contract v(cid);
    creditor cred(v.creditorId());

    activateContractWiz wiz;
    QFont f = wiz.font(); f.setPointSize(10); wiz.setFont(f);
    wiz.label = v.label();
    wiz.creditorName = cred.firstname() + " " + cred.lastname();
    wiz.expectedAmount = v.plannedInvest();
    wiz.setField("amount", v.plannedInvest());
    wiz.setField("date", v.conclusionDate().addDays(1));
    wiz.exec();
    if( ! wiz.field("confirmed").toBool()) {
        qInfo() << "contract activation cancled by the user";
        return;
    }
    if( ! v.activate(wiz.field("date").toDate(), wiz.field("amount").toDouble())) {
        qCritical() << "activation failed";
        Q_ASSERT(true);
    }
    return;
}

void changeContractValue(qlonglong cid)
{
    contract con(cid);
    if( ! con.isActive()) {
        qCritical() << "tried to changeContractValue of an inactive contract";
        Q_ASSERT(true);
        return;
    }

    creditor cre(con.creditorId());
    wizChangeContract wiz;
    QFont f = wiz.font(); f.setPointSize(10); wiz.setFont(f);
    wiz.creditorName = cre.firstname() + " " + cre.lastname();
    wiz.contractLabel= con.label();
    wiz.currentAmount= con.value();
    wiz.earlierstDate = con.latestBooking().addDays(1);
    wiz.setField("deposit_notPayment", QVariant(true));

    wiz.exec();
    if( wiz.field("confirmed").toBool()) {
        double amount {wiz.field("amount").toDouble()};
        QDate date {wiz.field("date").toDate()};
        qDebug() << wiz.field("deposit_notPayment") << ", " << amount << ", " << date;
        if( wiz.field("deposit_notPayment").toBool()) {
            con.deposit(date, amount);
        } else {
            con.payout(date, amount);
        }
    } else
        qInfo() << "contract change was cancled by the user";
}

void deleteInactiveContract(qlonglong cid)
{   LOG_CALL;
    // contracts w/o bookings can be deleted
// todo: wiz ui with confirmation?
    contract::remove(cid);
}

void terminateContract(qlonglong cid)
{   LOG_CALL;
    contract c(cid);
    if( c.hasEndDate()) {
        terminateContract_Final(c);
    } else {
        cancelContract(c);
    }
}
void terminateContract_Final( contract& c)
{   LOG_CALL;

    // todo: wiz UI:
    // done ask termination date
    // done check it to be after lastBooking
    // done calculate final value
    // done wiz page for confirmation / print option 4 contract history

    wizTerminateContract wiz(nullptr, c);
    QFont f = wiz.font(); f.setPointSize(10); wiz.setFont(f);
    wiz.exec();

    //open do final interest booking, do payout booking
    //open move to exVerträge, exBuchungen
    //open print pdf

    return;
}
void cancelContract( contract& )
{   LOG_CALL;
Q_ASSERT(!"repair");
    // UI asking for date
    // change noticePeriod -1
    // change terminationDate (date)
}

void annualSettlement()
{
    QDate vYear=bookings::dateOfnextSettlement();
    if( ! vYear.isValid() || vYear.isNull()) {
        QMessageBox::information(nullptr, "Fehler",
            "Ein Jahr für die nächste Zinsberechnung konnte nicht gefunden werden."
            "Es keine Verträge für die eine Abrechnung gemacht werden kann.");
        return;
    }
    wizAnnualSettlement wiz;
    wiz.setField("year", vYear.year() -1);
    wiz.exec();
}
