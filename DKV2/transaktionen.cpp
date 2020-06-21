
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
    QFont f = wiz.font();
    f.setPointSize(10); wiz.setFont(f);
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
    if( !v.activate(wiz.field("amount").toDouble(), wiz.field("date").toDate())) {
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
    QFont f = wiz.font(); f.setPointSize(10);
    wiz.setFont(f);
    wiz.creditorName = cre.firstname() + " " + cre.lastname();
    wiz.contractLabel= con.label();
    wiz.currentAmount= con.Value();
    wiz.earlierstDate = con.latestBooking().addDays(1);
    wiz.setField("deposit_notPayment", QVariant(true));

    wiz.exec();
    if( wiz.field("confirmed").toBool()) {
        double amount {wiz.field("amount").toDouble()};
        QDate date {wiz.field("date").toDate()};
        qDebug() << wiz.field("deposit_notPayment") << ", " << amount << ", " << date;
        if( wiz.field("deposit_notPayment").toBool()) {
            con.deposit(amount, date);
        } else {
            con.payout(amount, date);
        }
    } else
        qInfo() << "contract change was cancled by the user";
}

void deleteInactiveContract(qlonglong cid)
{   LOG_CALL;
    // contracts w/o bookings can be deleted
// todo: wiz ui with confirmation?
    contract v(cid);
    v.remove();
}

void terminateContract(qlonglong cid)
{
    contract c(cid);
    if( -1 == c.noticePeriod()) {
        // contract w termination date
        terminateContract_Final(c);
    } else {
        cancelContract_wNoticePeriod(c);
    }
}
void terminateContract_Final( contract& c)
{   LOG_CALL;

    // todo: wiz UI:
    //OK ask termination date
    //nOK calculate final value,
    //OK wiz page for confirmation / print option 4 contract history
    // do final interest booking, do payout booking
    // move to exVerträge, exBuchungen
    // print pdf

    wizTerminateContract wiz(nullptr, c);
    wiz.exec();

    return;
}
void cancelContract_wNoticePeriod( contract& )
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
