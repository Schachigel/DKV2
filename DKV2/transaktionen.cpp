
#include "helperfile.h"
#include "helpersql.h"
#include "appconfig.h"
#include "creditor.h"
#include "booking.h"
#include "csvwriter.h"
#include "helper.h"
#include "investment.h"

#include "wizactivatecontract.h"
#include "wizcancelcontract.h"
#include "wizchangecontractvalue.h"
#include "wizterminatecontract.h"
//#include "wizannualsettlement.h"
#include "busycursor.h"
#include "dlgannualsettlement.h"
#include "dlgaskdate.h"
#include "dlgchangecontracttermination.h"
#include "dlginterestletters.h"
#include "filewriter.h"
#include "transaktionen.h"
#include "wiznew.h"
#include "wiznewinvestment.h"

namespace {}

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
    if (not wiz.c_tor.isValid()) {
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
    wiz.c_tor.setId(wiz.existingCreditorId);
    qInfo() << "contract for existing creditor will be created";
  }

  if (not wiz.field(pnConfirmContract).toBool()) {
    qInfo() << "user decided not to save the contract";
    return;
  }
  contract cont;
  cont.setCreditorId(wiz.c_tor.id());
  cont.setPlannedInvest(QLocale().toDouble(wiz.field(pnAmount).toString()));
  cont.setInterestRate(wiz.interest / 100.);
  cont.setInvestment(wiz.investmentId);
  cont.setLabel(wiz.field(pnLabel).toString());
  cont.setConclusionDate(wiz.field(pnCDate).toDate());
  cont.setNoticePeriod(wiz.noticePeriod);
  cont.setPlannedEndDate(wiz.field(pnEDate).toDate());
  cont.setInterestModel(wiz.iPaymentMode);
  cont.setComment(wiz.field(pnContractComment).toString());
  cont.setInterestActive(not wiz.field(pnIPaymentDelayed).toBool());
  if (-1 == cont.saveNewContract()) {
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
    busycursor bc;
    wiz.c_tor.setId(creditorId);
    if (wiz.c_tor.update())
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
void changeContractComment(contract *pc) {
  LOG_CALL;
  creditor cred(pc->creditorId());
  QInputDialog ipd(getMainWindow());
  ipd.setInputMode(QInputDialog::TextInput);
  ipd.setWindowTitle(qsl("Anmerkung zum Vertrag ändern"));
  ipd.setStyleSheet(qsl("* { font-size: 10pt; }"));
  ipd.setTextValue(pc->comment());
  ipd.setLabelText(qsl("Ändere den Kommentar zum Vertrag von ") +
                   cred.firstname() + qsl(" ") + cred.lastname());
  ipd.setOption(QInputDialog::UsePlainTextEditForTextInput, true);
  if (ipd.exec() not_eq QDialog::Accepted) {
    qInfo() << "inpud dlg canceled";
    return;
  }
  busycursor bc;
  if (pc->updateComment(ipd.textValue().trimmed()))
    qInfo() << "successfully updated comment";
  else
    qCritical() << "update comment failed";
}
void changeContractTermination(contract *pc) {
  LOG_CALL;
  qDebug() << pc->toString();
  creditor cred(pc->creditorId());
  dlgChangeContractTermination dlg(getMainWindow());

  if (pc->initialBookingReceived())
    dlg.setMinContractTerminationDate(pc->latestBooking().date);
  else
    dlg.setMinContractTerminationDate(pc->conclusionDate().addDays(1));

  dlg.setEndDate(pc->plannedEndDate());
  dlg.setNoticePeriod(pc->noticePeriod());

  if (QDialog::Accepted == dlg.exec())
    pc->updateTerminationDate(dlg.endDate(), dlg.noticePeriod());
  return;
}

void bookInitialPaymentReceived(contract *v) {
  LOG_CALL;
  creditor cred(v->creditorId());

  wizInitialPayment wiz(getMainWindow());
  wiz.label = v->label();
  wiz.creditorName = cred.firstname() + qsl(" ") + cred.lastname();
  wiz.expectedAmount = v->plannedInvest();
  wiz.setField(fnAmount, QLocale().toString(v->plannedInvest()));
  wiz.setField(fnDate, v->conclusionDate());
  wiz.minimalActivationDate = v->conclusionDate();
  wiz.delayedInterest = not v->interestActive();
  wiz.exec();
  if (not wiz.field(qsl("confirmed")).toBool()) {
    qInfo() << "contract activation canceled by the user";
    return;
  }

  if (not v->bookInitialPayment(
          wiz.field(fnDate).toDate(),
          QLocale().toDouble(wiz.field(fnAmount).toString()))) {
    qCritical() << "activation failed";
    Q_ASSERT(false);
  }
  return;
}
void activateInterest(contract *v) {
  LOG_CALL;
  booking lastB = v->latestBooking();
  Q_ASSERT(lastB.type != booking::Type::non);
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
  if (not v->bookActivateInterest(dlg.date())) {
    QString msg{qsl("Beim der Buchung ist ein Fehler eingetreten - bitte "
                    "schaue in die LOG Datei für weitere Informationen")};
    QMessageBox::warning(getMainWindow(), qsl("Buchungsfehler"), msg);
  }
}

void changeContractValue(contract *pc) {
  LOG_CALL;
  if (not pc->initialBookingReceived()) {
    qCritical() << "tried to changeContractValue of an inactive contract";
    Q_ASSERT(false);
    return;
  }

  creditor cre(pc->creditorId());
  wizChangeContract wiz(getMainWindow());
  wiz.creditorName = cre.firstname() + qsl(" ") + cre.lastname();
  wiz.contractLabel = pc->label();
  wiz.currentAmount = pc->value();
  wiz.earlierstDate = pc->latestBooking().date.addDays(1);
  wiz.interestPayoutPossible =
      pc->iModel() == interestModel::payout && pc->interestActive();
  wiz.setField(fnDeposit_notPayment, QVariant(true));

  wiz.exec();
  qDebug() << wiz.field(fnPayoutInterest);
  if (wiz.field(qsl("confirmed")).toBool()) {
    double amount{QLocale().toDouble(wiz.field(qsl("amount")).toString())};
    QDate date{wiz.field(qsl("date")).toDate()};
    qDebug() << wiz.field(fnDeposit_notPayment) << ", " << amount << ", "
             << date;
    if (wiz.field(qsl("deposit_notPayment")).toBool()) {
      pc->deposit(date, amount, wiz.field(fnPayoutInterest).toBool());
    } else {
      pc->payout(date, amount, wiz.field(fnPayoutInterest).toBool());
    }
  } else
    qInfo() << "contract change was cancled by the user";
}
void annualSettlement() {
  LOG_CALL;
  QDate bookingDate = bookings::dateOfnextSettlement();
  if (not bookingDate.isValid() or bookingDate.isNull()) {
    QMessageBox::information(nullptr, qsl("Fehler"),
                             qsl("Ein Jahr für die nächste Zinsberechnung "
                                 "konnte nicht gefunden werden."
                                 "Es gibt keine Verträge für die eine "
                                 "Abrechnung gemacht werden kann."));
    return;
  }
  int yearOfSettlement = bookingDate.year();
  //    wizAnnualSettlement wiz(getMainWindow());
  dlgAnnualsettlement dlg(getMainWindow(), yearOfSettlement);

  int dlgFeedback = dlg.exec();
  if (dlgFeedback == QDialog::Rejected || not dlg.confirmed())
    return;
  busycursor bc;
  QVector<QVariant> ids = executeSingleColumnSql(
      dkdbstructur[contract::tnContracts][contract::fnId]);
  qDebug() << "contracts to try execute annual settlement for: " << ids;
  QVector<contract> changedContracts;
  QVector<QDate> startOfInterrestCalculation;
  QVector<booking> asBookings;
  for (const auto &id : qAsConst(ids)) {
    contract c(id.toLongLong());
    QDate startDate = c.latestBooking().date;
    if (0 not_eq c.annualSettlement(yearOfSettlement)) {
      changedContracts.push_back(c);
      asBookings.push_back(c.latestBooking());
      startOfInterrestCalculation.push_back(startDate);
    }
  }

  if (not dlg.print_csv())
    return;
  csvwriter csv(qsl(";"));
  csv.addColumns(contract::booking_csv_header());
  // Vorname; Nachname; Email; Strasse; Plz; Stadt; IBAN;
  // Kennung; Auszahlend; Begin; Buchungsdatum; Zinssatz; Kreditbetrag;
  // Zins; Endbetrag
  QLocale l;
  for (int i = 0; i < changedContracts.count(); i++) {
    contract &c = changedContracts[i];
    booking &b = asBookings[i];
    // write data to CSV
    creditor cont(c.creditorId());
    csv.appendToRow(cont.firstname());
    csv.appendToRow(cont.lastname());
    csv.appendToRow(cont.email());
    csv.appendToRow(cont.street());
    csv.appendToRow(cont.postalCode());
    csv.appendToRow(cont.city());

    csv.appendToRow(cont.iban());
    csv.appendToRow(c.label());
    csv.appendToRow(toString(c.iModel()));
    csv.appendToRow(startOfInterrestCalculation[i].toString(qsl("dd.MM.yyyy")));
    csv.appendToRow(bookingDate.toString(qsl("dd.MM.yyyy")));
    csv.appendToRow(l.toString(c.interestRate(), 'f', 2));

    if (c.iModel() == interestModel::reinvest)
      csv.appendToRow(l.toString(c.value() - b.amount, 'f', 2));
    else
      csv.appendToRow(l.toString(c.value(), 'f', 2));
    csv.appendToRow(l.toString(b.amount, 'f', 2));
    csv.appendToRow(l.toString(c.value(), 'f', 2));
  }
  QString filename{qsl("%1_Jahresabrechnung-%2.csv")};
  filename = filename.arg(QDate::currentDate().toString(Qt::ISODate),
                          QString::number(yearOfSettlement));
  csv.saveAndShowInExplorer(filename);
  return;
}

/*************************/
/*** Ausdrucke ***********/
/*************************/

void createInitialLetterTemplates() {
  LOG_CALL;
  QDir outDir(appConfig::Outdir());
  outDir.mkdir(qsl("vorlagen"));
  outDir.mkdir(qsl("html"));

  extractTemplateFileFromResource(appConfig::Outdir() + qsl("/vorlagen/"),
                                  qsl("brieflogo.png"));
  extractTemplateFileFromResource(appConfig::Outdir() + qsl("/vorlagen/"),
                                  qsl("zinsbrief.css"));
  extractTemplateFileFromResource(appConfig::Outdir() + qsl("/vorlagen/"),
                                  qsl("zinsbrief.html"));
  extractTemplateFileFromResource(appConfig::Outdir() + qsl("/vorlagen/"),
                                  qsl("endabrechnung.html"));
  extractTemplateFileFromResource(appConfig::Outdir() + qsl("/vorlagen/"),
                                  qsl("endabrechnung.csv"));
  extractTemplateFileFromResource(appConfig::Outdir() + qsl("/html/"),
                                  qsl("zinsbrief.css"));
}

int askUserForYearOfPrintouts() {
  LOG_CALL;
  QVector<int> years = bookings::yearsWithAnnualBookings();
  if (years.size() == 0) {
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

void annualSettlementLetters() {
  LOG_CALL;
  int yearOfSettlement = askUserForYearOfPrintouts();
  if (yearOfSettlement <= 0) {
    qInfo() << "print out canceled by user";
    return;
  }

  busycursor bc;
  QVector<booking> annualBookings =
      bookings::getAnnualSettelments(yearOfSettlement);

  if (annualBookings.size() == 0) {
    bc.finish();
    QMessageBox::information(
        nullptr, qsl("Fehler"),
        qsl("Im Jahr %1 konnten keine Zinsbuchungen gefunden werden. "
            "Es gibt keine Verträge für die eine Abrechnung gemacht werden "
            "kann.")
            .arg(yearOfSettlement));
    return;
  }

  createInitialLetterTemplates();

  QList<QPair<int, QString>> creditors;
  fillCreditorsListForLetters(creditors, yearOfSettlement);
  if (creditors.size() <= 0) {
    qDebug() << "no creditors to create letters for";
    return;
  }

  QVariantMap printData = {};
  printData[qsl("Zinsjahr")] = yearOfSettlement;
  printData[qsl("Zinsdatum")] =
      QDate(yearOfSettlement, 12, 31).toString(qsl("dd.MM.yyyy"));
  printData[qsl("gmbhLogo")] =
      QVariant(appConfig::Outdir() + qsl("/vorlagen/brieflogo.png"));
  printData[qsl("meta")] = getMetaTableAsMap();

  for (auto &cred : qAsConst(creditors)) {
    creditor credRecord(cred.first);
    printData["creditor"] = credRecord.getVariant();

    QVector<QVariant> ids = executeSingleColumnSql(
        dkdbstructur[contract::tnContracts][contract::fnId],
        qsl(" %1=%2 GROUP BY id")
            .arg(contract::fnKreditorId, QString::number(cred.first)));

    QVariantList vl;
    double payedInterest = 0.;
    for (const auto &id : qAsConst(ids)) {
      contract contr(id.toLongLong());
      vl.append(contr.toVariantMap(QDate(yearOfSettlement, 1, 1),
                                   QDate(yearOfSettlement, 12, 31)));
      payedInterest += contr.payedInterest(yearOfSettlement);
    }
    printData[qsl("mitAusbezahltemZins")] = payedInterest > 0.;
    printData[qsl("ausbezahlterZins")] = payedInterest;
    printData[qsl("Vertraege")] = vl;

    QString fileName = qsl("Jahresinfo ")
                           .append(QString::number(yearOfSettlement))
                           .append(qsl("_"))
                           .append(QString::number(credRecord.id()))
                           .append(qsl("_"))
                           .append(credRecord.lastname())
                           .append(qsl(", "))
                           .append(credRecord.firstname().append(qsl(".pdf")));
    /////////////////////////////////////////////////
    savePdfFromHtmlTemplate(qsl("zinsbrief.html"), fileName, printData);
    /////////////////////////////////////////////////
  }
  bc.finish();
  showInExplorer(appConfig::Outdir(), showFolder);
  qInfo() << "Alles OK";
}

/*************************/
/*** contract endings   **/
/*************************/

void deleteInactiveContract(contract *c) {
  LOG_CALL;
  // contracts w/o bookings can be deleted
  // todo: wiz ui with confirmation?
  // todo: if creditor has no other (active or deleted) contracts: propose
  // delete creditor
  contract::remove(c->id());
}

void terminateContract(contract *pc) {
  /* Contract termination is a 2 step process:
   * - Cancel the contract
   *      -> set an end date
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
    qDebug() << "failed to terminate contract";
    return;
  }
  if (wiz.field(qsl("print")).toBool()) {
    // todo        printFinalizedContractAsCsv(c.id ());
  }

  return;
}

void cancelContract(contract &c) {
  LOG_CALL;
  wizCancelContract wiz(getMainWindow());
  wiz.c = c;
  wiz.creditorName = executeSingleValueSql(
                         qsl("Vorname || ' ' || Nachname"), qsl("Kreditoren"),
                         qsl("id=") + QString::number(c.creditorId()))
                         .toString();
  wiz.contractualEnd = QDate::currentDate().addMonths(c.noticePeriod());
  wiz.exec();
  if (not wiz.field(qsl("confirmed")).toBool()) {
    qInfo() << "cancel wizard canceled by user";
    return;
  }
  c.cancel(wiz.field(qsl("date")).toDate());
}

void finalizeContractLetter(contract *c) {
  LOG_CALL;

  busycursor bc;
  // copy Templates (if not available)
  createInitialLetterTemplates();

  QVariantMap printData = {};
  printData[qsl("gmbhLogo")] =
      QVariant(appConfig::Outdir() + qsl("/vorlagen/brieflogo.png"));
  printData[qsl("meta")] = getMetaTableAsMap();

  creditor credRecord(c->creditorId());
  printData[qsl("creditor")] = credRecord.getVariant();
  printData[qsl("Vertrag")] = c->toVariantMap();
  printData[qsl("endBetrag")] = QLocale().toCurrencyString (c->value());

  QString filename_wNoExt = qsl("Endabrechnung ")
                                .append(c->label().replace("/", "_"))
                                .append("_")
                                .append(credRecord.lastname())
                                .append(qsl(", "))
                                .append(credRecord.firstname());
  // TODO: make sure there are no chars, which might not be part of a Windows /
  // Linux filename (:, ...)

  savePdfFromHtmlTemplate(qsl("Endabrechnung.html"),
                          filename_wNoExt +qsl(".pdf"), printData);
  writeRenderedTemplate(qsl("Endabrechnung.csv"),
                        filename_wNoExt +qsl(".csv"), printData);
  showInExplorer(appConfig::Outdir(), showFolder);
  qInfo() << "Vertragsabschlussdokument erfolgreich angelegt";
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
