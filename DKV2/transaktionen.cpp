
#include <QString>
#include <QMessageBox>
#include <QDate>

#include "askdatedlg.h"
#include "helper.h"
#include "changecontractvaluewiz.h"
#include "activatecontractwiz.h"
#include "transaktionen.h"


void activateContract(qlonglong contractId)
{   LOG_CALL;
    contract v(contractId);
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
    if( !v.activate(wiz.field("date").toDate(), wiz.field("amount").toDouble())) {
        qCritical() << "activation failed";
        Q_ASSERT(true);
    }
    return;
}

void changeContractValue(qlonglong vid)
{
    contract con(vid);
    if( ! con.isActive()) {
        qCritical() << "tried to changeContractValue of an inactive contract";
        Q_ASSERT(true);
        return;
    }

    creditor cre(con.creditorId());
    ChangeContractWiz wiz;
    QFont f = wiz.font(); f.setPointSize(10);
    wiz.setFont(f);
    wiz.creditorName = cre.firstname() + " " + cre.lastname();
    wiz.contractLabel= con.label();
    wiz.currentAmount= con.currentValue();
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


void beendeVertrag(qlonglong )
{   LOG_CALL;
Q_ASSERT(!"repair");
    contract v;
}
void terminateContract_Final( contract& )
{   LOG_CALL;
Q_ASSERT(!"repair");
    return;
}
void cancelContract_wConclusionDate( contract& )
{   LOG_CALL;
Q_ASSERT(!"repair");
}
void VertragsEnde_PassiverV(contract& )
{   LOG_CALL;
Q_ASSERT(!"repair");
}

