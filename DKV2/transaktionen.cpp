
#include <QDate>
#include <QString>
#include <QMessageBox>

#include "csvwriter.h"
#include "appconfig.h"
#include "helper.h"
#include "booking.h"
#include "investment.h"
#include "dkdbcopy.h"

#include "wizchangecontractvalue.h"
#include "wizactivatecontract.h"
#include "wizterminatecontract.h"
#include "wizcancelcontract.h"
#include "wizannualsettlement.h"
#include "wiznewinvestment.h"
#include "wiznew.h"
#include "transaktionen.h"

bool checkSchema_ConvertIfneeded(const QString& origDbFile)
{   LOG_CALL;

    switch(check_db_version(origDbFile)){
    case lowVersion:
    {
        qInfo() << "lower version -> converting";
        if( QMessageBox::Yes not_eq QMessageBox::question(nullptr, qsl("Achtung"), qsl("Das Format der Datenbank ist veraltet. Soll die Datenbank konvertiert werden?"))) {
                qInfo() << "conversion rejected by user";
                return false;
        }
        QString backup =convert_database_inplace(origDbFile);
        if ( not backup.isEmpty()) {
            QMessageBox::information(nullptr, qsl("Erfolgsmeldung"), qsl("Die Konvertierung ware erfolgreich. Eine Kopie der ursprünglichen Datei liegt unter \n") +backup);
            return true;
        }
        else {
            qCritical() << "db converstion of older DB failed";
            return false;
        }
        break;
    }
    case sameVersion:
        return validateDbSchema(origDbFile, dkdbstructur);
        break;
    case noVersion:
    case higherVersion:
    default:
        return false;
        break;
    }
//    return false;
}

void activateContract(qlonglong cid)
{   LOG_CALL;
    contract v(cid);
    creditor cred(v.creditorId());

    wpActivateContract wiz(getMainWindow());
    wiz.label = v.label();
    wiz.creditorName = cred.firstname() + qsl(" ") + cred.lastname();
    wiz.expectedAmount = v.plannedInvest();
    wiz.setField(qsl("amount"), v.plannedInvest());
    wiz.setField(qsl("date"), v.conclusionDate().addDays(1));
    wiz.exec();
    if( not wiz.field(qsl("confirmed")).toBool()) {
        qInfo() << "contract activation cancled by the user";
        return;
    }
    if( not v.activate(wiz.field(qsl("date")).toDate(), wiz.field(qsl("amount")).toDouble())) {
        qCritical() << "activation failed";
        Q_ASSERT(false);
    }
    return;
}
void changeContractValue(qlonglong cid)
{   LOG_CALL;
    contract con(cid);
    if( not con.isActive()) {
        qCritical() << "tried to changeContractValue of an inactive contract";
        Q_ASSERT(false);
        return;
    }

    creditor cre(con.creditorId());
    wizChangeContract wiz(getMainWindow());
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
    wiz.exec();
    if( not wiz.field(qsl("confirm")).toBool())
        return;
    double interest =0., finalValue =0.;
    if( not c.finalize(false, wiz.field(qsl("date")).toDate(), interest, finalValue)) {
        qDebug() << "failed to terminate contract";
    }
    return;
}
void cancelContract( contract& c)
{   LOG_CALL;
    wizCancelContract wiz(getMainWindow());
    wiz.c = c;
    wiz.creditorName = executeSingleValueSql(qsl("Vorname || ' ' || Nachname"), qsl("Kreditoren"), qsl("id=") + QString::number(c.creditorId())).toString();
    wiz.contractualEnd =QDate::currentDate().addMonths(c.noticePeriod());
    wiz.exec();
    if( not wiz.field(qsl("confirmed")).toBool()) {
        qInfo() << "cancel wizard canceled by user";
        return;
    }
    c.cancel(wiz.field(qsl("date")).toDate());
}

void annualSettlement()
{   LOG_CALL;
    QDate bookingDate=bookings::dateOfnextSettlement();
    if( not bookingDate.isValid() or bookingDate.isNull()) {
        QMessageBox::information(nullptr, qsl("Fehler"),
            qsl("Ein Jahr für die nächste Zinsberechnung konnte nicht gefunden werden."
            "Es gibt keine Verträge für die eine Abrechnung gemacht werden kann."));
        return;
    }
    int yearOfSettlement =bookingDate.year();
    wizAnnualSettlement wiz(getMainWindow());
    wiz.setField(qsl("year"), yearOfSettlement);
    wiz.exec();
    if( not wiz.field(qsl("confirm")).toBool())
        return;

    QVector<QVariant> ids =executeSingleColumnSql(dkdbstructur[qsl("Vertraege")][qsl("id")]);
    qDebug() << "contracts to try execute annual settlement for: " << ids;
    QVector<contract> changedContracts;
    QVector<QDate> startOfInterrestCalculation;
    QVector<booking>  asBookings;
    for( auto id: ids)
    {
        contract c(id.toLongLong());
        QDate startDate = c.latestBooking().date;
        if(0 not_eq c.annualSettlement(yearOfSettlement)) {
            changedContracts.push_back(c);
            asBookings.push_back(c.latestBooking());
            startOfInterrestCalculation.push_back(startDate);
        }
    }

    if( not wiz.field(qsl("printCsv")).toBool())
        return;
    csvwriter csv(qsl(";"));
    csv.addColumns(contract::booking_csv_header());
    // Vorname; Nachname; Email; Strasse; Plz; Stadt; IBAN;
    // Kennung; Auszahlend; Begin; Buchungsdatum; Zinssatz; Kreditbetrag;
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
        csv.appendToRow(toString(c.iModel()));
        csv.appendToRow(startOfInterrestCalculation[i].toString(qsl("dd.MM.yyyy")));
        csv.appendToRow(bookingDate.toString(qsl("dd.MM.yyyy")));
        csv.appendToRow(l.toString(c.interestRate(), 'f', 2));

        if( c.iModel() == interestModel::reinvest)
            csv.appendToRow(l.toString(c.value()-b.amount, 'f', 2));
        else
            csv.appendToRow(l.toString(c.value(), 'f', 2));
        csv.appendToRow(l.toString(b.amount, 'f', 2));
        csv.appendToRow(l.toString(c.value(), 'f', 2));
    }
    QString filename{qsl("%1_Jahresabrechnung-%2.csv")};
    filename =filename.arg(QDate::currentDate().toString(Qt::ISODate), QString::number(yearOfSettlement));
    csv.saveAndShowInExplorer(filename);
    return;
}

void editCreditor(qlonglong creditorId)
{   LOG_CALL;
    wizNew wiz(getMainWindow());
    creditor cred(creditorId);
    wiz.setField(pnFName, cred.firstname());
    wiz.setField(pnLName, cred.lastname());
    wiz.setField(pnStreet, cred.street());
    wiz.setField(pnPcode, cred.postalCode());
    wiz.setField(pnCity, cred.city());
    wiz.setField(pnEMail, cred.email());
    wiz.setField(pnComment, cred.comment());
    wiz.setField(pnIban, cred.iban());
    wiz.setField(pnBic, cred.bic());
    wiz.selectCreateContract =false;
    //wiz.setField(pnConfirmContract, false);
    wiz.creditorId = creditorId;
    wiz.setUpdateMode(true);
    wiz.setStartId(page_address);

    if( QDialog::Accepted == wiz.exec()) {
        qInfo() << "successfully updated creditor";
    }
}
void newCreditorAndContract()
{   LOG_CALL;
    wizNew wiz(getMainWindow());
    wiz.setField(pnNew, true);
    wiz.setField(pnConfirmContract, false);
    wiz.exec();
    return;
}

void changeContractComment(qlonglong id)
{
    contract c(id);
    QInputDialog ipd(getMainWindow());
    ipd.setInputMode(QInputDialog::TextInput);
    ipd.setTextValue(c.comment());
    ipd.setLabelText(qsl("Ändere den Kommentar zu dem Vertrag"));
    ipd.setOption(QInputDialog::UsePlainTextEditForTextInput, true);
    if( ipd.exec() not_eq QDialog::Accepted) {
        qInfo() << "inpud dlg canceled";
        return;
    }
    c.updateComment(ipd.textValue());
}

void createInvestment()
{
    wizNewInvestment wiz;
    wiz.exec();
    if( not wiz.field(pnKorrekt).toBool()) {
        qInfo() << "investment wiz was canceled";
        return;
    }
    if( not saveNewInvestment( wiz.field(pnZSatz).toInt(),
                          wiz.field(pnVon).toDate(),
                          wiz.field(pnBis).toDate(),
                          wiz.field(pnTyp).toString())) {
        qCritical() << "Investment could not be saved";
        QMessageBox::warning(nullptr, qsl("Fehler"), qsl("Die Geldanlage konnte nicht gespeichert werden"));
    }
}
