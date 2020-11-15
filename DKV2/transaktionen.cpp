
#include <QDate>
#include <QString>
#include <QMessageBox>
#include <QDate>

#include "csvwriter.h"
#include "appconfig.h"
#include "helper.h"
#include "booking.h"
#include "wizchangecontractvalue.h"
#include "wizactivatecontract.h"
#include "wizterminatecontract.h"
#include "wizcancelcontract.h"
#include "wizannualsettlement.h"
#include "wiznew.h"
#include "transaktionen.h"


void activateContract(qlonglong cid)
{   LOG_CALL;
    contract v(cid);
    creditor cred(v.creditorId());

    activateContractWiz wiz(getMainWindow());
    QFont f = wiz.font(); f.setPointSize(10); wiz.setFont(f);
    wiz.label = v.label();
    wiz.creditorName = cred.firstname() + qsl(" ") + cred.lastname();
    wiz.expectedAmount = v.plannedInvest();
    wiz.setField(qsl("amount"), v.plannedInvest());
    wiz.setField(qsl("date"), v.conclusionDate().addDays(1));
    wiz.exec();
    if( ! wiz.field(qsl("confirmed")).toBool()) {
        qInfo() << "contract activation cancled by the user";
        return;
    }
    if( ! v.activate(wiz.field(qsl("date")).toDate(), wiz.field(qsl("amount")).toDouble())) {
        qCritical() << "activation failed";
        Q_ASSERT(true);
    }
    return;
}

void changeContractValue(qlonglong cid)
{   LOG_CALL;
    contract con(cid);
    if( ! con.isActive()) {
        qCritical() << "tried to changeContractValue of an inactive contract";
        Q_ASSERT(true);
        return;
    }

    creditor cre(con.creditorId());
    wizChangeContract wiz(getMainWindow());
    QFont f = wiz.font(); f.setPointSize(10); wiz.setFont(f);
    wiz.creditorName = cre.firstname() + qsl(" ") + cre.lastname();
    wiz.contractLabel= con.label();
    wiz.currentAmount= con.value();
    wiz.earlierstDate = con.latestBooking().date.addDays(1);
    wiz.setField(qsl("deposit_notPayment"), QVariant(true));

    wiz.exec();
    if( wiz.field(qsl("confirmed")).toBool()) {
        double amount {wiz.field(qsl("amount")).toDouble()};
        QDate date {wiz.field(qsl("date")).toDate()};
        qDebug() << wiz.field(qsl("deposit_notPayment")) << ", " << amount << ", " << date;
        if( wiz.field(qsl("deposit_notPayment")).toBool()) {
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
// if creditor has no other contracts: delete creditor
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
    wizTerminateContract wiz(getMainWindow(), c);
    QFont f = wiz.font(); f.setPointSize(10); wiz.setFont(f);
    wiz.exec();
    if( ! wiz.field(qsl("confirm")).toBool())
        return;
    double interest =0., finalValue =0.;
    if( ! c.finalize(false, wiz.field(qsl("date")).toDate(), interest, finalValue)) {
        qDebug() << "failed to terminate contract";
    }
    return;
}
void cancelContract( contract& c)
{   LOG_CALL;
    wizCancelContract wiz(getMainWindow());
    QFont f = wiz.font(); f.setPointSize(10); wiz.setFont(f);
    wiz.c = c;
    wiz.creditorName = executeSingleValueSql(qsl("Vorname || ' ' || Nachname"), qsl("Kreditoren"), qsl("id=") + QString::number(c.creditorId())).toString();
    wiz.contractualEnd =QDate::currentDate().addMonths(c.noticePeriod());
    wiz.exec();
    if( ! wiz.field(qsl("confirmed")).toBool()) {
        qInfo() << "cancel wizard canceled by user";
        return;
    }
    c.cancel(wiz.field(qsl("date")).toDate());
}

void annualSettlement()
{   LOG_CALL;
    QDate vYear=bookings::dateOfnextSettlement();
    if( ! vYear.isValid() || vYear.isNull()) {
        QMessageBox::information(nullptr, qsl("Fehler"),
            qsl("Ein Jahr für die nächste Zinsberechnung konnte nicht gefunden werden."
            "Es gibt keine Verträge für die eine Abrechnung gemacht werden kann."));
        return;
    }
    wizAnnualSettlement wiz(getMainWindow());
    wiz.setField(qsl("year"), vYear.year() -1);
    wiz.exec();
    if( ! wiz.field(qsl("confirm")).toBool())
        return;
    QSqlQuery q;
    q.setForwardOnly(true);
    q.exec(qsl("SELECT id FROM Vertraege"));
    QVector<contract> changedContracts;
    QVector<booking>  asBookings;
    double payedInterest =0.;
    while(q.next()) {
        contract c(q.value(qsl("id")).toLongLong());
        if(c.annualSettlement(wiz.field(qsl("year")).toInt())) {
            changedContracts.push_back(c);
            asBookings.push_back(c.latestBooking());
            payedInterest += c.latestBooking().amount;
        }
    }
    if( ! wiz.field(qsl("printCsv")).toBool())
        return;

    csvwriter csv(qsl(";"));
    csv.addColumns(contract::booking_csv_header());
    // Vorname; Nachname; Email; Strasse; Plz; Stadt; IBAN;
    // Kennung; Auszahlend; Buchungsdatum; Zinssatz; Kreditbetrag;
    // Zins; Endbetrag
    QLocale l;
    for(int i =0; i < changedContracts.count(); i++) {
        contract& c =changedContracts[i];
        booking&  b =asBookings[i];
        // write data to CSV
        creditor cont(c.creditorId());
        csv.appendToRow(cont.firstname()); csv.appendToRow(cont.lastname());
        csv.appendToRow(cont.email());     csv.appendToRow(cont.street());
        csv.appendToRow(cont.postalCode());csv.appendToRow(cont.city());
        csv.appendToRow(cont.iban());      csv.appendToRow(c.label());
        c.reinvesting() ? csv.appendToRow(qsl("thesaurierend")) : csv.appendToRow(qsl("ausschüttend"));
        csv.appendToRow(QDate(wiz.field(qsl("year")).toInt(), 1, 1).toString(qsl("dd.MM.yyyy")));
        csv.appendToRow(l.toString(c.interestRate(), 'f', 2));
        csv.appendToRow(l.toString(c.value()-b.amount, 'f', 2));
        csv.appendToRow(l.toString(b.amount, 'f', 2));
        csv.appendToRow(l.toString(c.value(), 'f', 2));
    }
    QString filename(QDate::currentDate().toString(Qt::ISODate) + qsl("-Jahresabrechnung.csv"));
    csv.saveAndShowInExplorer(filename);
    return;
}

void editCreditor(qlonglong creditorId)
{   LOG_CALL;
    wizEditCreditor wiz(getMainWindow());
    QFont f = wiz.font(); f.setPointSize(10); wiz.setFont(f);
    creditor cred(creditorId);
    wiz.setField(qsl("firstname"), cred.firstname());
    wiz.setField(qsl("lastname"), cred.lastname());
    wiz.setField(qsl("street"), cred.street());
    wiz.setField(qsl("pcode"), cred.postalCode());
    wiz.setField(qsl("city"), cred.city());
    wiz.setField(qsl("email"), cred.email());
    wiz.setField(qsl("comment"), cred.comment());
    wiz.setField(qsl("iban"), cred.iban());
    wiz.setField(qsl("bic"), cred.bic());
    wiz.setField(qsl("confirmContract"), false);
    wiz.creditorId = creditorId;
    if( QDialog::Accepted == wiz.exec()) {
        qInfo() << "successfully updated creditor";
    }
}
void newCreditorAndContract()
{   LOG_CALL;
    wizNew wiz(getMainWindow());
    QFont f = wiz.font(); f.setPointSize(10); wiz.setFont(f);
    wiz.setField(qsl("create_new"), true);
    wiz.setField(qsl("confirmContract"), false);
    wiz.exec();
    return;
}
