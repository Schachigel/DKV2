#include "transaktionen.h"

#include "appconfig.h"
#include "dkdbhelper.h"
#include "uihelper.h"
#include "filewriter.h"
#include "helperfin.h"
#include "helpersql.h"
#include "uihelper.h"

#include "csvwriter.h"
#include "dbstructure.h"
#include "filewriter.h"

#include "booking.h"
#include "creditor.h"
#include "investment.h"

#include "qnamespace.h"
#include "wizactivatecontract.h"
#include "wizcancelcontract.h"
#include "wizchangecontractvalue.h"
#include "wizterminatecontract.h"
#include "annualSettlement.h"
#include "busycursor.h"
#include "dlgannualsettlement.h"
#include "dlgaskcontractlabel.h"
#include "dlgaskdate.h"
#include "dlgchangecontracttermination.h"
#include "dlginterestletters.h"
#include "dlgchangebooking.h"
#include "wiznew.h"
#include "wiznewinvestment.h"

void newCreditorAndContract() {
    LOG_CALL;
    creditor cred;
    wizNew wiz(cred, getMainWindow());
    wiz.setField(pnNew, true);
    wiz.setField(pnConfirmContract, false);
    // !!!!
    /*auto wizRes =*/wiz.exec();
    // !!!!
    if (wiz.field(pnNew).toBool()) {

        // one can only come here, if the users accepted the creation of the
        // creditor
        if (not wiz.cred.isValid()) {
            // the user was checked during validation of the wizard -> very wrong
            QMessageBox::critical(getMainWindow(), qsl("Eingabefehler"),
                                  qsl("Die Kundendaten sind ungültig"
                                      ". Details findest Du in der Log Datei."));
            qCritical() << "invalid creditor data -> we have to fail";
            return;
        }

        if (cred.save() >= 0)
            qInfo() << "creditor created successfully";
        else {
            QMessageBox::critical(
                        getMainWindow(), qsl("Programm Fehler"),
                        qsl("Die Kundeninfo konnte nicht "
                            "gespeichert werden. Details findest Du in der Log Datei."));
            return;
        }
    } else {
        wiz.cred.setId(wiz.existingCreditorId);
        qInfo() << "contract for existing creditor will be created";
    }

    if (not wiz.field(pnConfirmContract).toBool()) {
        qInfo() << "user decided not to save the contract";
        return;
    }
    contract cont;
    cont.setCreditorId(wiz.cred.id());
    cont.setPlannedInvest(QLocale().toDouble(wiz.field(pnAmount).toString()));
    cont.setInterestRate(wiz.interest / 100.);

    if (isValidRowId(wiz.investmentId))
        cont.setInvestmentId(wiz.investmentId);
    cont.setLabel(wiz.field(pnLabel).toString());
    cont.setConclusionDate(wiz.field(pnCDate).toDate());
    cont.setNoticePeriod(wiz.noticePeriod);
    cont.setPlannedEndDate(wiz.field(pnEDate).toDate());
    cont.setInterestModel(wiz.iPaymentMode);
    cont.setComment(wiz.field(pnContractComment).toString());
    cont.setInterestActive(not wiz.field(pnIPaymentDelayed).toBool());
    if (SQLITE_invalidRowId == cont.saveNewContract()) {
        qCritical() << "New contract could not be saved";
        QMessageBox::critical(
                    getMainWindow(), qsl("Fehler"),
                    qsl("Der Vertrag konnte nicht "
                        "gespeichert werden. Details findest Du in der Log Datei"));
    } else {
        qInfo() << "New contract successfully saved";
    }
    return;
}

void editCreditor(qlonglong creditorId) {
    LOG_CALL;
    creditor cred(creditorId);
    wizNew wiz(cred, getMainWindow());
    wiz.setStartId(page_address);

    if (QDialog::Accepted == wiz.exec()) {
        busyCursor bc;
        wiz.cred.setId(creditorId);
        if (wiz.cred.update())
            qInfo() << "successfully updated creditor";
        else {
            bc.finish();
            QMessageBox::critical(
                        getMainWindow(), qsl("Programm Fehler"),
                        qsl("Die Kundeninfo konnte nicht "
                            "geändert werden. Details findest Du in der Log Datei."));
            return;
        }
    }
}

void changeContractComment(contract *pContract) {
    LOG_CALL;
    creditor cred(pContract->creditorId());
    QInputDialog ipd(getMainWindow());
    ipd.setInputMode(QInputDialog::TextInput);
    ipd.setWindowTitle(qsl("Anmerkung zum Vertrag ändern"));
    ipd.setTextValue(pContract->comment());
    ipd.setLabelText(qsl("Ändere den Kommentar zum Vertrag von ") +
                     cred.firstname() + qsl(" ") + cred.lastname());
    ipd.setOption(QInputDialog::UsePlainTextEditForTextInput, true);
    if (ipd.exec() not_eq QDialog::Accepted) {
        qInfo() << "inpud dlg canceled";
        return;
    }
    busyCursor bc;
    if (pContract->updateComment(ipd.textValue().trimmed()))
        qInfo() << "successfully updated comment";
    else
        qCritical() << "update comment failed";
}

void changeContractTermination(contract *pContract) {
    LOG_CALL;
    qInfo() << pContract->toString();
    dlgChangeContractTermination dlg(getMainWindow());

    if (pContract->initialPaymentReceived())
        dlg.setMinContractTerminationDate(pContract->latestBooking().date);
    else
        dlg.setMinContractTerminationDate(pContract->conclusionDate().addDays(1));

    dlg.setEndDate(pContract->plannedEndDate());
    dlg.setNoticePeriod(pContract->noticePeriod());

    if (QDialog::Accepted == dlg.exec())
        pContract->updateTerminationDate(dlg.endDate(), dlg.noticePeriod());
    return;
}

void changeContractDate(contract *pContract) {
    QDate oldCD {pContract->conclusionDate()};
    QDate actD {pContract->initialPaymentDate()};

    dlgAskDate dlg;
    dlg.setDate(oldCD);
    dlg.setHeader(qsl("Vertragsdatum ändern"));
    dlg.setMsg(qsl("Gib ein neues Datum für den Vertrag ein. <p>Es muss vor dem "
                   "ersten Zahlungseingang %1 liegen")
               .arg(actD.toString("dd.MM.yyyy")));

    if (not actD.isValid())
        actD= QDate(EndOfTheFuckingWorld); // any contract conclusion date should be
    // valid, if initial booking was not made
    while (QDialog::Accepted == dlg.exec()) {
        if (dlg.date() == oldCD) {
            qInfo() << __FUNCTION__ << " contract date was not changed";
            return;
        }
        if (dlg.date() > actD) {
            qInfo() << __FUNCTION__ << " date is too late";
            QMessageBox::information(
                        getMainWindow(), qsl("Ungültiges Datum"),
                        qsl("Das Vertragsdatum muss vor dem ersten Geldeingang liegen."));
            continue;
        }
        if (pContract->updateConclusionDate(dlg.date())) {
            qInfo() << __FUNCTION__ << " contract date was changed successfully to "
                    << dlg.date();
            return;
        } else {
            qInfo() << __FUNCTION__
                    << " update of conclusion date was not successful";
            QMessageBox::information(
                        getMainWindow(), qsl("Aktualisierung fehlgeschlagen"),
                        qsl("Das Vertragsdatum konnte nicht aktualisiert werden. Genaueres "
                            "findest Du in der LOG Datei"));
            continue;
        }
    }
    qInfo() << __FUNCTION__ << " Dialog was canceled";
}

void changeContractLabel(contract *pContract) {
    QString currentLabel = pContract->label();
    dlgAskContractLabel dlg(currentLabel);

    while (QDialog::Accepted == dlg.exec()) {
        if (currentLabel == dlg.newLabel()) {
            qInfo() << "label was not changed";
            return;
        }
        if (not isValidNewContractLabel(dlg.newLabel())) {
            qInfo() << "label is already in use: " << dlg.newLabel();
            QMessageBox::information(
                        getMainWindow(), qsl("Aktualisierung fehlgeschlagen"),
                        qsl("Die Kennung wird bereits verwendet. Bitte wähle eine andere."));
            continue;
        }
        if (pContract->updateLabel(dlg.newLabel())) {
            qInfo() << "Label was changed to " << dlg.newLabel();
            return;
        } else {
            qInfo() << "update of label failed";
            QMessageBox::information(
                        getMainWindow(), qsl("Aktualisierung fehlgeschlagen"),
                        qsl("Die Kennung konnte nicht aktualisiert werden. Genaueres findest "
                            "Du in der LOG Datei"));
            continue;
        }
    }
    qInfo() << __FUNCTION__ << " Dialog was canceld";
}

void changeInitialPaymentDate(contract *pContract) {
    if (not pContract->initialPaymentReceived()) {
        qCritical()
            << "Contract has no initial payment -> ui should prevent us from comeing here";
        return;
    }

    QDate conclusionDate{pContract->conclusionDate()}; // b-date has to be > conclusion
    QDate currentIPDate{pContract->initialPaymentDate()};
    dlgAskDate dlg;
    dlg.setDate(currentIPDate);
    dlg.setHeader(qsl("Datum des Geldeingangs ändern"));
    dlg.setMsg(qsl("Gib das aktualisierte Datum für den Geldeingang ein.<p>Es "
                   "muss nach dem "
                   "Vertragsdatum %1 liegen.")
               .arg(conclusionDate.toString("dd.MM.yyyy")));
    while (QDialog::Accepted == dlg.exec()) {
        if (dlg.date() == currentIPDate) {
            qInfo() << __FUNCTION__ << " initial payment date was not changed";
            return;
        }
        if (dlg.date() < conclusionDate) {
            qInfo() << __FUNCTION__ << " date is too early";
            QMessageBox::information(
                        getMainWindow(), qsl("Ungültiges Datum"),
                        qsl("Das Datum des Geldeingangs muss nach dem Vertragsdatum %1 "
                            "liegen.")
                        .arg(conclusionDate.toString(qsl("dd.MM.yyyy"))));
            continue;
        }
        if (pContract->updateInitialPaymentDate(dlg.date())) {
            qInfo() << __FUNCTION__ << " initial payment date successfully changed";
            return;
        } else {
            qInfo() << __FUNCTION__
                    << " update of initial payment date was not successful";
            QMessageBox::information(
                        getMainWindow(), qsl("Aktualisierung fehlgeschlagen"),
                        qsl("Das Datum des Geldeingangs konnte nicht aktualisiert werden. "
                            "Genaueres findest Du in der LOG Datei"));
            continue;
        }
    }
    qInfo() << __FUNCTION__ << " dialog was canceled";
}

void receiveInitialBooking(contract *contract) {
    LOG_CALL;
    creditor cred(contract->creditorId());

    wizInitialPayment wiz(getMainWindow());
    wiz.label = contract->label();
    wiz.creditorName = cred.firstname() + qsl(" ") + cred.lastname();
    wiz.expectedAmount = contract->plannedInvest();
    wiz.setField(fnAmount, QLocale().toString(wiz.expectedAmount, 'f', 2));
    wiz.setField(fnDate, contract->conclusionDate());
    wiz.minimalActivationDate = contract->conclusionDate();
    wiz.delayedInterest = not contract->interestActive();
    wiz.exec();
    if (not wiz.field(qsl("confirmed")).toBool()) {
        qInfo() << "contract activation canceled by the user";
        return;
    }

    if (not contract->bookInitialPayment(
                wiz.field(fnDate).toDate(),
                QLocale().toDouble(wiz.field(fnAmount).toString()))) {
        qCritical() << Q_FUNC_INFO <<  "contract activation failed";
        Q_UNREACHABLE();    }
    return;
}

void activateInterest(contract *ctr) {
    LOG_CALL;
    booking lastB = ctr->latestBooking();
    Q_ASSERT(lastB.type != bookingType::non);
    QDate earlierstActivation = lastB.date.addDays(1);
    dlgAskDate dlg(getMainWindow());
    dlg.setDate(earlierstActivation);
    dlg.setHeader(qsl("Aktivierung der Zinszahlung"));
    dlg.setMsg(qsl("Gib das Datum an, zu dem die Zinszahlung des Vertrags "
                   "aktiviert werden soll"));
    do {
        if (QDialog::Rejected == dlg.exec()) {
            qInfo() << "interest activation was canceled by the user";
            return;
        }
        if (dlg.date() < earlierstActivation) {
            QString msg{
                qsl("Das Datum kann nicht vor dem letzten Buchungsdatum (%1) sein!")
                        .arg(earlierstActivation.toString(Qt::ISODate))};
            QMessageBox::information(getMainWindow(), qsl("Ungültiges Datum"), msg);
            continue;
        }
        break;
    } while (true);
    BookingResult booking_OK {ctr->bookActivateInterest(dlg.date())};
    if (not booking_OK) {
        QString msg{qsl("Beim der Buchung ist ein Fehler eingetreten:\n %1").arg(booking_OK.error)};
        QMessageBox::warning(getMainWindow(), qsl("Buchungsfehler"), msg);
    }
}

namespace {
inline bool isLastDaysOfYear(QDate d) { return 12 == d.month()
           && (30 == d.day() || 31 == d.day()); }
}
void doDeposit_or_payout(contract *pContract) {
    LOG_CALL;
    if (not pContract->initialPaymentReceived()) {
        qCritical() << "tried to changeContractValue of an inactive contract"
            "; should be prevented by UI";
        Q_UNREACHABLE();
        return;
    }

    creditor cre(pContract->creditorId());
    wizChangeContract wiz(getMainWindow());
    wiz.creditorName = cre.firstname() + qsl(" ") + cre.lastname();
    wiz.contractLabel = pContract->label();
    wiz.currentAmount = pContract->value();
    wiz.earlierstDate = [&pc =*pContract]() -> QDate{
        QDate lbd =pc.latestBookingDate();
        if( isLastDaysOfYear(lbd))
            return QDate(lbd.year() +1, 1, 1);
        else
            return (lbd.addDays(1));
    }();
    wiz.interestPayoutPossible =
            pContract->iModel() == interestModel::payout && pContract->interestActive();
    wiz.setField(fnDeposit_notPayment, QVariant(true));

    wiz.exec();
    qInfo() << wiz.field(fnPayoutInterest);
    if (wiz.field(qsl("confirmed")).toBool()) {
        double amount{QLocale().toDouble(wiz.field(qsl("amount")).toString())};
        QDate date{wiz.field(qsl("date")).toDate()};
        if (wiz.field(qsl("deposit_notPayment")).toBool()) {
            pContract->deposit(date, amount, wiz.field(fnPayoutInterest).toBool());
        } else {
            pContract->payout(date, amount, wiz.field(fnPayoutInterest).toBool());
        }
    } else
        qInfo() << "contract change was canceld by the user";
}

void changeBookingValue(qlonglong bookingId)
{
    changeBookingData cbd;
    getChangeBookingData (cbd, bookingId);
    dlgChangeBooking dlg;
    dlg.Kennung      =cbd.VKennung;
    dlg.Buchungsdatum=cbd.Buchungsdatum;
    dlg.Vorname      =cbd.Vorname;
    dlg.Nachname     =cbd.Nachname;
    dlg.ursprWertInCt =cbd.BetragInCt;

    if( dlg.exec() == QDialog::Rejected) {
        QMessageBox::information (getMainWindow (), qsl("Abbruch"), qsl("Die Änderung der Buchung wurde abgebrochen"));
        return;
    }
    qInfo() << qsl("Änderung der Buchung %1 auf %2").arg(QString::number(bookingId), s_ct2euro( dlg.neuerWertInCt));
    if( not writeBookingUpdate(bookingId, dlg.neuerWertInCt)) {
        QMessageBox::warning (getMainWindow (), "Fehler", "Die Buchung konnte nicht angepasst werden");
    }
}

void undoLastBooking(contract *v)
{
// todo: this should remove all bookings of the same date
// todo: have a ui to select the booking, from which all "younger" bookings should be deleted
    QString sqlMsg {qsl(R"str(
SELECT Vertraege.Kennung
 , Kreditoren.Vorname, Kreditoren.Nachname
 , Buchungen.BuchungsArt, Buchungen.Betrag, Buchungen.Datum, Buchungen.id
FROM Vertraege
 JOIN Buchungen ON Vertraege.id = Buchungen.VertragsId
 JOIN Kreditoren ON Vertraege.KreditorId = Kreditoren.id
WHERE Buchungen.VertragsId = %1
ORDER BY Buchungen.rowid DESC
LIMIT 1
)str").arg(v->id ())};

    QSqlRecord rec =executeSingleRecordSql (sqlMsg);
    if( rec.isEmpty ()) {
        qCritical() << "die jüngste Buchung konnte nicht abgerufen werden";
        return;
    }
    QString BArt =bookingTypeDisplayString(bookingtypeFromInt(rec.value (qsl("Buchungen.BuchungsArt")).toLongLong ()));
    QString BBetrag =s_ct2euro (rec.value (qsl("Buchungen.Betrag")).toInt ());
    QString BDatum =rec.value (qsl("Buchungen.Datum")).toDate ().toString (Qt::TextDate);
    QString VKennung = rec.value (qsl("Vertraege.Kennung")).toString ();
    QString VN =rec.value( qsl("Kreditoren.Vorname")).toString ();
    QString NN =rec.value (qsl("Kreditoren.Nachname")).toString ();

    QString msg {qsl("Soll die Buchung <p>'%1' über %2 vom %3 <p>des"
                     " Vertrags %4 von %5 %6 <p>rückgängig gemacht werden?")
                .arg(BArt, BBetrag, BDatum,
                     VKennung, VN, NN)};
    if( QMessageBox::question (NULL, qsl("Buchung löschen?"), msg) != QMessageBox::Yes) {
        qInfo() << "Löschen der Buchung wurde abgebrochen";
        return;
    }
    QString sqlDelete ={qsl("DELETE FROM Buchungen WHERE Buchungen.id = %1").arg(rec.value(qsl("Buchungen.id")).toLongLong ())};
    if( not executeSql_wNoRecords (sqlDelete)) {
        qCritical() << "failed to delete booking ";
    }
}

namespace {
void print_as_csv(const QDate &bookingDate,
                  const QVector<contract> &changedContracts,
                  const QVector<QDate> &startOfInterrestCalculation,
                  const QVector<booking> &asBookings) {

    CsvWriter csv;
    csv.addColumns({{"Vorname"}, {"Nachname"}, {"Email"}, {"Strasse"}, {"Plz"},
                    {"Stadt"}, {"IBAN"}, {"Kennung"}, {"Auszahlend"}, {"Beginn"},
                    {"Buchungsdatum"}, {"Zinssatz"}, {"Kreditbetrag"},
                    {"Zinssatz"}, {"Zins"}, {"Endbetrag"}});
    QLocale l;
    for (int i = 0; i < changedContracts.count(); i++) {
        const contract &c = changedContracts[i];
        const booking &b = asBookings[i];
        // write data to CSV
        creditor cont(c.creditorId());
        csv.appendValueToNextRecord(cont.firstname());
        csv.appendValueToNextRecord(cont.lastname());
        csv.appendValueToNextRecord(cont.email());
        csv.appendValueToNextRecord(cont.street());
        csv.appendValueToNextRecord(cont.postalCode());
        csv.appendValueToNextRecord(cont.city());

        csv.appendValueToNextRecord(cont.iban());
        csv.appendValueToNextRecord(c.label());
        csv.appendValueToNextRecord(interestModelDisplayString(c.iModel()));
        csv.appendValueToNextRecord(startOfInterrestCalculation[i].toString(qsl("dd.MM.yyyy")));
        csv.appendValueToNextRecord(bookingDate.toString(qsl("dd.MM.yyyy")));
        csv.appendValueToNextRecord(l.toString(c.interestRate(), 'f', 2));

        if (c.iModel() == interestModel::reinvest)
            csv.appendValueToNextRecord(l.toString(c.value() - b.amount, 'f', 2));
        else
            csv.appendValueToNextRecord(l.toString(c.value(), 'f', 2));
        csv.appendValueToNextRecord(l.toString(b.amount, 'f', 2));
        csv.appendValueToNextRecord(l.toString(c.value(), 'f', 2));
    }
    QString filename{qsl("%1_Jahresabrechnung-%2.csv")};
    filename = filename.arg(QDate::currentDate().toString(Qt::ISODate),
                            i2s(bookingDate.year()));

    saveStringToUtf8File(filename, csv.toString());
    showInExplorer(filename);
}

void print_annaul_settlement_csv(int year) {

}
} // namespace
void annualSettlement() {
    LOG_CALL;
    QDate nextAS_dateForAnyContract = dateOfnextSettlement();
    if (not nextAS_dateForAnyContract.isValid()) {
        QMessageBox::information(getMainWindow(), qsl("! Info !"),
                                 qsl("Eine Jahreszinsabrechnung ist derzeit nicht möglich.\n"
                                     "Es gibt keine Verträge für die eine "
                                     "Abrechnung gemacht werden kann."));
        return;
    }

    qInfo() << QString("Next AS possible for %1; lets ask the user").arg( nextAS_dateForAnyContract.year());
    int yearOfSettlement =nextAS_dateForAnyContract.year();
    dlgAnnualSettlement dlg(yearOfSettlement, getMainWindow());
    if (dlg.exec() == QDialog::Rejected || not dlg.confirmed())
        return;

    busyCursor bc;
    // annual settlement takes place HERE
    if( not executeCompleteAS( yearOfSettlement)) {
        // annual settlement takes place HERE
        qCritical() << "annual settlement failed !!";
    }
}

/*************************/
/*** Ausdrucke Jahresend Briefe *******/
/*************************/
namespace {
void createInitialLetterTemplates() {
    LOG_CALL;
    QDir outDir(appconfig::Outdir());
    outDir.mkdir(qsl("vorlagen"));
    outDir.mkdir(qsl("html"));
    const QString vorlagenVerzeichnis = appconfig::Outdir() + qsl("/vorlagen/");
    extractTemplateFileFromResource(vorlagenVerzeichnis, qsl("brieflogo.png"));
    extractTemplateFileFromResource(vorlagenVerzeichnis, qsl("zinsbrief.css"));
    extractTemplateFileFromResource(vorlagenVerzeichnis, qsl("zinsbrief.html"));
    extractTemplateFileFromResource(vorlagenVerzeichnis, qsl("zinsliste.html"));
    extractTemplateFileFromResource(vorlagenVerzeichnis,
                                    qsl("zinsbuchungen.csv"));
#ifdef Q_OS_WIN
    extractTemplateFileFromResource(vorlagenVerzeichnis, qsl("zinsmails-win.bat"),
                                    qsl("zinsmails.bat"));
#else
    extractTemplateFileFromResource(
                vorlagenVerzeichnis, qsl("zinsmails-linux.bat"), qsl("zinsmails.bat"));
#endif
    extractTemplateFileFromResource(vorlagenVerzeichnis, qsl("dkv2mail.bat"));
    extractTemplateFileFromResource(vorlagenVerzeichnis, qsl("dkv2mail"));
    extractTemplateFileFromResource(vorlagenVerzeichnis,
                                    qsl("endabrechnung.html"));
    extractTemplateFileFromResource(vorlagenVerzeichnis,
                                    qsl("endabr-Zinsnachw.html"));
    extractTemplateFileFromResource(vorlagenVerzeichnis,
                                    qsl("endabrechnung.csv"));
    extractTemplateFileFromResource(appconfig::Outdir() + qsl("/html/"),
                                    qsl("zinsbrief.css"));
}

int askUserForYearOfPrintouts() {
    LOG_CALL;
    QVector<int> years = yearsWithAnnualBookings();
    if (years.isEmpty()) {
        QMessageBox::information(
                    getMainWindow(), qsl("Keine Daten"),
                    qsl("Es liegen keine Abrechnungen zum Ausdruck vor"));
        return -1;
    }
    dlgInterestLetters dlg(getMainWindow(), years);

    if (QDialog::Rejected == dlg.exec())
        return -1;

    return dlg.getYear();
}
} // namespace

QVariantList getContractList(qlonglong creditorId, QDate startDate,
                             QDate endDate, bool isTerminated) {
    QVariantList vl;
    /* get list of contracts */
    QVector<QVariant> ids = executeSingleColumnSql(
                                isTerminated ? dkdbstructur[contract::tnExContracts][contract::fnId]
                            : dkdbstructur[contract::tnContracts][contract::fnId],
            qsl(" %1=%2 GROUP BY id").arg(contract::fnKreditorId, i2s(creditorId)));

    for (const auto &id : std::as_const(ids)) {
        contract contr(id.toLongLong(), isTerminated);
        /* Forget contracts that don't exist in the period.
i.e. conclusionDate must be before end of period
and contract must not have been finalized before start of period */
        bool oldFinalizedContract =
                isTerminated && (contr.plannedEndDate() < startDate);
        if (contr.conclusionDate() <= endDate && oldFinalizedContract == false) {
            QVariantMap contractMap = contr.toVariantMap(startDate, endDate);
            vl.append(contractMap);
        }
    }
    return vl;
}
void annualSettlementLetters() {
    LOG_CALL;
    int yearOfSettlement = askUserForYearOfPrintouts();
    if (yearOfSettlement <= 0) {
        qInfo() << "print out canceled by user";
        return;
    }

    busyCursor bc;
    createInitialLetterTemplates();

    QList<qlonglong> creditorIds;
    creditorsWithAnnualSettlement(creditorIds, yearOfSettlement);
    if (creditorIds.size() <= 0) {
        qInfo() << "no Kreditoren to create letters for";
        return;
    }

    QVariantMap printData = {};
    printData[qsl("Zinsjahr")] = yearOfSettlement;
    printData[qsl("Zinsdatum")] =
            QDate(yearOfSettlement, 12, 31).toString(qsl("dd.MM.yyyy"));
    printData[qsl("gmbhLogo")] =
            QVariant(appconfig::Outdir() + qsl("/vorlagen/brieflogo.png"));
    printData[qsl("meta")] = getMetaTableAsMap();

    /* storage for data of all Kreditoren */
    QVariantList Kreditoren;
    QVariantList Auszahlungen;

    double totalBetrag2 = 0.;
    double annualInterest2 = 0;
    double otherInterest2 = 0;
    double interestForPayout2 = 0.;
    double interestCredit2 = 0.;
    for (const auto &cred : std::as_const(creditorIds)) {
        creditor credRecord(cred);
        QVariantMap currCreditorMap = credRecord.getVariantMap();
        printData["creditor"] = currCreditorMap;

        QVariantList vl;
        double otherInterest = 0.;
        double annualInterest = 0.;
        double interestForPayout = 0.;
        double interestCredit = 0.;
        double totalBetrag = 0;
        /* get active contracts */
        vl = getContractList(cred, QDate(yearOfSettlement, 1, 1),
                             QDate(yearOfSettlement, 12, 31), false);
        /* get terminated contracts */
        vl.append(getContractList(cred, QDate(yearOfSettlement, 1, 1),
                                  QDate(yearOfSettlement, 12, 31), true));

        if (vl.size() > 0) {
            double payedInterest = 0;
            for (const QVariant &v : std::as_const(vl)) {
                QVariantMap vm = v.toMap();
                otherInterest += vm["dSonstigeZinsen"].toDouble();
                annualInterest += vm["dJahresZinsen"].toDouble();
                interestForPayout += vm["dAuszahlung"].toDouble();
                interestCredit += vm["dZinsgutschrift"].toDouble();
                totalBetrag += vm["dEndBetrag"].toDouble();
            }
            payedInterest = otherInterest + interestForPayout;
            printData[qsl("ausbezahlterZins")] =
                    payedInterest == 0. ? "" : s_d2euro(payedInterest);
            printData[qsl("mitAusbezahltemZins")] = payedInterest > 0.;
            printData[qsl("mitZins")] = payedInterest + interestCredit > 0.;
            printData[qsl("SumAuszahlung")] =
                    interestForPayout == 0. ? "" : s_d2euro(interestForPayout);
            printData[qsl("dSumJahresZinsen")] = annualInterest;

            printData[qsl("SumJahresZinsen")] =
                    annualInterest == 0. ? "" : s_d2euro(annualInterest);

            printData[qsl("sonstigerZins")] =
                    otherInterest == 0. ? "" : s_d2euro(otherInterest);

            printData["SumZinsgutschrift"] =
                    interestCredit == 0. ? "" : s_d2euro(interestCredit);

            printData[qsl("Vertraege")] = vl;

            printData[qsl("totalBetrag")] = s_d2euro(totalBetrag);

            QString fileName = qsl("Jahresinfo %1_%3, %4")
                               .arg(i2s(yearOfSettlement), credRecord.lastname(),
                                    credRecord.firstname().append(qsl(".pdf")));
            fileName = fileName.replace("/", "-").replace("*", "#").replace(":", "#");
            /* save data for eMail batch file */
            currCreditorMap[qsl("Vertraege")] = vl;
            currCreditorMap["SumBetrag"] = s_d2euro(totalBetrag);
            currCreditorMap[qsl("Attachment")] = fileName;
            currCreditorMap[qsl("SumJahresZinsen")] =
                    annualInterest == 0. ? "" : s_d2euro(annualInterest);

            currCreditorMap[qsl("SumSonstigeZinsen")] =
                    otherInterest == 0. ? "" : s_d2euro(otherInterest);

            currCreditorMap[qsl("SumZinsgutschrift")] =
                    interestCredit == 0. ? "" : s_d2euro(interestCredit);

            if (currCreditorMap[qsl("Email")] == "") {
                currCreditorMap.remove(qsl("Email"));
            }

            if (interestForPayout > 0.) {
                currCreditorMap[qsl("SumAuszahlung")] = s_d2euro(interestForPayout);
                Auszahlungen.append(currCreditorMap);
            } else {
                currCreditorMap[qsl("SumAuszahlung")] = "";
            }

            annualInterest2 += annualInterest;
            otherInterest2 += otherInterest;
            interestForPayout2 += interestForPayout;
            interestCredit2 += interestCredit;
            totalBetrag2 += totalBetrag;
            Kreditoren.append(currCreditorMap);
            /////////////////////////////////////////////////
            savePdfFromHtmlTemplate(qsl("zinsbrief.html"), fileName, printData);
        }
        /////////////////////////////////////////////////
    }

    // Create the eMail Batch file.
    printData[qsl("Kreditoren")] = Kreditoren;
    printData[qsl("Auszahlungen")] = Auszahlungen;
    printData[qsl("Sum2Betrag")] = s_d2euro(totalBetrag2);
    printData[qsl("Sum2JahresZinsen")] = s_d2euro(annualInterest2);
    printData[qsl("Sum2SonstigeZinsen")] = s_d2euro(otherInterest2);
    printData[qsl("Sum2Auszahlung")] = s_d2euro(interestForPayout2);
    printData[qsl("Sum2Zinsgutschrift")] = s_d2euro(interestCredit2);

    writeRenderedTemplate(
                qsl("zinsmails.bat"),
                qsl("zinsmails").append(i2s(yearOfSettlement)).append(qsl(".bat")),
                printData);

    // Create the list of yearly bookings
    savePdfFromHtmlTemplate(
                qsl("zinsliste.html"),
                qsl("Zinsliste-").append(i2s(yearOfSettlement)).append(qsl(".pdf")),
                printData);

    // Create the csv of annual interest bookings
    writeRenderedTemplate(
                qsl("zinsbuchungen.csv"),
                qsl("zinsbuchungen").append(i2s(yearOfSettlement)).append(qsl(".csv")),
                printData);

    bc.finish();
    showInExplorer(appconfig::Outdir(), showObject::folder);
    qInfo() << "Alles OK";
}

/*************************/
/*** contract endings   **/
/*************************/

void deleteInactiveContract(contract *c) {
    LOG_CALL;
    // contracts w/o bookings can be deleted
    // todo: if creditor has no other (active or deleted) contracts: propose
    // delete creditor
    if (QMessageBox::question(getMainWindow(), qsl("Vertrag löschen"),
                              qsl("Soll der inaktive Vertrag ") + c->label() +
                              qsl(" gelöscht werden?")) == QMessageBox::Yes) {
        if (not c->deleteInactive()) {
            QMessageBox::information(getMainWindow(), qsl("Fehler"),
                                     qsl("Der Vertrag konnte nicht gelöscht werden"));
        }
    }
    if (QMessageBox::question(
                getMainWindow(), qsl("Kreditor*in löschen?"),
                qsl("Soll die zugehörige Kreditgeber*in gelöscht werden?")) ==
            QMessageBox::Yes) {
        creditor::remove(c->creditorId());
    }
}
void terminateContract(contract *pc) {
   /* Contract termination is a 2 step process:
   * - Cancel the contract
   *      -> set planned end date
   * - terminate contract
   *      -> set actual end date
   *      -> calculate interest and payout value
   *      -> move contract from Vertraege to exVertraege table
   */
    LOG_CALL;
    if (pc->hasEndDate())
        terminateContract_Final(*pc);
    else
        cancelContract(*pc);
}
void terminateContract_Final(contract &c) {
    LOG_CALL;
    wizTerminateContract wiz(getMainWindow(), c);
    wiz.exec();
    if (not wiz.field(qsl("confirm")).toBool())
        return;
    double interest = 0., finalValue = 0.;
    if (not c.finalize(false, wiz.field(qsl("date")).toDate(), interest,
                       finalValue)) {
        QMessageBox::warning(nullptr, qsl("Fehler"),
                             qsl("Die Geldanlage konnte nicht beendet "
                                 "werden.\nDetails findest Du in der LOG Datei!"));
        qInfo() << "failed to terminate contract";
        return;
    }
    if (wiz.field(qsl("print")).toBool())

    {
        // todo        printFinalizedContractAsCsv(c.id ());
    }

    return;
}
void cancelContract(contract &c) {
    LOG_CALL;
    wizCancelContract wiz(getMainWindow());
    wiz.c = c;
    wiz.creditorName = Vor_Nachname_Kreditor(c.creditorId());

    wiz.contractualEnd = QDate::currentDate().addMonths(c.noticePeriod());
    wiz.exec();
    if (not wiz.field(qsl("confirmed")).toBool())
    {
        qInfo() << "cancel wizard canceled by user";
        return;
    }
    c.cancel(wiz.field(qsl("date")).toDate(), wiz.field(qsl("KüDatum")).toDate());
}
void finalizeContractLetter(contract *c) {
    LOG_CALL;

    busyCursor bc;
    // copy Templates (if not available)
    createInitialLetterTemplates();

    QVariantMap printData = {};
    printData[qsl("gmbhLogo")] =
            QVariant(appconfig::Outdir() + qsl("/vorlagen/brieflogo.png"));
    printData[qsl("meta")] = getMetaTableAsMap();

    creditor credRecord(c->creditorId());
    printData[qsl("creditor")] = QVariant(credRecord.getVariantMap());
    printData[qsl("Vertrag")] = c->toVariantMap();
    printData[qsl("endBetrag")] = s_d2euro(c->value());
    double ausbezZins = c->payedInterestAtTermination();
    printData[qsl("ausbezahlterZins")] = ausbezZins;
    printData[qsl("mitAusbezahltemZins")] = !qFuzzyCompare(ausbezZins, 0.);
    QString filenamepattern =
            qsl("%1_%2,%3")
            .arg(c->label().replace("/", "_"), credRecord.lastname(),
                 credRecord.firstname());
    // TODO: make sure there are no chars, which might not be part of a Windows /
    // Linux filename (:, ...)

    savePdfFromHtmlTemplate(qsl("Endabrechnung.html"),
                            qsl("Endabrechnung-") + filenamepattern + qsl(".pdf"),
                            printData);
    if (printData[qsl("mitAusbezahltemZins")].toBool())
        savePdfFromHtmlTemplate(
                    qsl("endabr-Zinsnachw.html"),
                    qsl("FinalerZinsnachweis-") + filenamepattern + qsl(".pdf"), printData);

    writeRenderedTemplate(qsl("Endabrechnung.csv"),
                          qsl("Endabrechnung-") + filenamepattern + qsl(".csv"),
                          printData);
    showInExplorer(appconfig::Outdir(), showObject::folder);
    qInfo() << "Vertragsabschlussdokument erfolgreich angelegt";
}

void deleteFinalizedContract(contract *c) {
    if (QMessageBox::Yes !=
            QMessageBox::question(getMainWindow(), qsl("Beendeten Vertrag löschen"),
                                  qsl("Soll der Vertrag %1 entgültig aus der "
                                      "Datenbank gelöscht werden?")
                                  .arg(c->label()),
                                  QMessageBox::Yes | QMessageBox::No)) {
        return;
    }
    autoRollbackTransaction arbt;
    executeSql_wNoRecords(
                qsl("DELETE FROM exBuchungen WHERE VertragsId = %1").arg(c->id_aS()));
    executeSql_wNoRecords(
                qsl("DELETE FROM exVertraege WHERE id = %1").arg(c->id_aS()));
    executeSql_wNoRecords(
                qsl("DELETE FROM Kreditoren  WHERE id = %1").arg(c->creditorId()));
    arbt.commit();
}

/*************************/
/*** investments        **/
/*************************/

qlonglong createInvestment_matchingContract(int &interest, QDate &from,
                                            QDate &to) {
    LOG_CALL;
    // give the user a UI to create a investment which will match a certain set of
    // contract data
    wizNewInvestment wiz;
    wiz.setField(pnZSatz, QVariant(interest));
    wiz.initStartDate(from);
    wiz.setField(pnBis, QVariant(from.addYears(1).addDays(-1)));
    wiz.exec();
    if (not wiz.field(pnKorrekt).toBool()) {
        qInfo() << "investment wiz was canceled";
        return 0;
    }
    qlonglong newId =
            saveNewInvestment(wiz.field(pnZSatz).toInt(), wiz.field(pnVon).toDate(),
                              wiz.field(pnBis).toDate(), wiz.field(pnTyp).toString());
    if (0 >= newId) {
        qCritical() << "Investment could not be saved";
        QMessageBox::warning(nullptr, qsl("Fehler"),
                             qsl("Die Geldanlage konnte nicht gespeichert werden"));
        return 0;
    }
    // in-out parameter
    interest = wiz.field(pnZSatz).toInt();
    from = wiz.field(pnVon).toDate();
    to = wiz.field(pnBis).toDate();

    return newId;
}
void createInvestment() {
    LOG_CALL;
    wizNewInvestment wiz;
    wiz.initStartDate(QDate::currentDate());
    wiz.exec();
    if (not wiz.field(pnKorrekt).toBool()) {
        qInfo() << "investment wiz was canceled";
        return;
    }
    if (0 >= saveNewInvestment(
                wiz.field(pnZSatz).toInt(), wiz.field(pnVon).toDate(),
                wiz.field(pnBis).toDate(), wiz.field(pnTyp).toString())) {
        qCritical() << "Investment could not be saved";
        QMessageBox::warning(nullptr, qsl("Fehler"),
                             qsl("Die Geldanlage konnte nicht gespeichert werden"));
    }
}
