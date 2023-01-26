
#include "appconfig.h"
#include "dkdbhelper.h"
#include "helper.h"
#include "helperfile.h"
#include "helpersql.h"

#include "csvwriter.h"
#include "dbstructure.h"
#include "filewriter.h"

#include "booking.h"
#include "creditor.h"
#include "investment.h"

#include "wizactivatecontract.h"
#include "wizcancelcontract.h"
#include "wizchangecontractvalue.h"
#include "wizterminatecontract.h"
// #include "wizannualsettlement.h"
#include "busycursor.h"
#include "dlgannualsettlement.h"
#include "dlgaskdate.h"
#include "dlgchangecontracttermination.h"
#include "dlginterestletters.h"
#include "transaktionen.h"
#include "wiznew.h"
#include "wiznewinvestment.h"

namespace
{
}

void newCreditorAndContract()
{
  LOG_CALL;
  creditor cred;
  wizNew wiz(cred, getMainWindow());
  wiz.setField(pnNew, true);
  wiz.setField(pnConfirmContract, false);
  // !!!!
  /*auto wizRes =*/wiz.exec();
  // !!!!
  if (wiz.field(pnNew).toBool())
  {

    // one can only come here, if the users accepted the creation of the
    // creditor
    if (not wiz.cred.isValid())
    {
      // the user was checked during validation of the wizard -> very wrong
      QMessageBox::critical(getMainWindow(), qsl("Eingabefehler"),
                            qsl("Die Kundendaten sind ungültig"
                                ". Details findest Du in der Log Datei."));
      qCritical() << "invalid creditor data -> we have to fail";
      return;
    }

    if (cred.save() >= 0)
      qInfo() << "creditor created successfully";
    else
    {
      QMessageBox::critical(
          getMainWindow(), qsl("Programm Fehler"),
          qsl("Die Kundeninfo konnte nicht "
              "gespeichert werden. Details findest Du in der Log Datei."));
      return;
    }
  }
  else
  {
    wiz.cred.setId(wiz.existingCreditorId);
    qInfo() << "contract for existing creditor will be created";
  }

  if (not wiz.field(pnConfirmContract).toBool())
  {
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
  if (SQLITE_invalidRowId == cont.saveNewContract())
  {
    qCritical() << "New contract could not be saved";
    QMessageBox::critical(
        getMainWindow(), qsl("Fehler"),
        qsl("Der Vertrag konnte nicht "
            "gespeichert werden. Details findest Du in der Log Datei"));
  }
  else
  {
    qInfo() << "New contract successfully saved";
  }
  return;
}

void editCreditor(qlonglong creditorId)
{
  LOG_CALL;
  creditor cred(creditorId);
  wizNew wiz(cred, getMainWindow());
  wiz.setStartId(page_address);

  if (QDialog::Accepted == wiz.exec())
  {
    busycursor bc;
    wiz.cred.setId(creditorId);
    if (wiz.cred.update())
      qInfo() << "successfully updated creditor";
    else
    {
      bc.finish();
      QMessageBox::critical(
          getMainWindow(), qsl("Programm Fehler"),
          qsl("Die Kundeninfo konnte nicht "
              "geändert werden. Details findest Du in der Log Datei."));
      return;
    }
  }
}
void changeContractComment(contract *pc)
{
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
  if (ipd.exec() not_eq QDialog::Accepted)
  {
    qInfo() << "inpud dlg canceled";
    return;
  }
  busycursor bc;
  if (pc->updateComment(ipd.textValue().trimmed()))
    qInfo() << "successfully updated comment";
  else
    qCritical() << "update comment failed";
}
void changeContractTermination(contract *pc)
{
  LOG_CALL;
  qInfo() << pc->toString();
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

void receiveInitialBooking(contract *v)
{
  LOG_CALL;
  creditor cred(v->creditorId());

  wizInitialPayment wiz(getMainWindow());
  wiz.label = v->label();
  wiz.creditorName = cred.firstname() + qsl(" ") + cred.lastname();
  wiz.expectedAmount = v->plannedInvest();
  wiz.setField(fnAmount, QLocale().toString(v->plannedInvest(), 'f', 2));
  wiz.setField(fnDate, v->conclusionDate());
  wiz.minimalActivationDate = v->conclusionDate();
  wiz.delayedInterest = not v->interestActive();
  wiz.exec();
  if (not wiz.field(qsl("confirmed")).toBool())
  {
    qInfo() << "contract activation canceled by the user";
    return;
  }

  if (not v->bookInitialPayment(
          wiz.field(fnDate).toDate(),
          QLocale().toDouble(wiz.field(fnAmount).toString())))
  {
    qCritical() << "contract activation failed";
    Q_ASSERT(false);
  }
  return;
}
void activateInterest(contract *v)
{
  LOG_CALL;
  booking lastB = v->latestBooking();
  Q_ASSERT(lastB.type != bookingType::non);
  QDate earlierstActivation = lastB.date.addDays(1);
  dlgAskDate dlg(getMainWindow());
  dlg.setDate(earlierstActivation);
  dlg.setHeader(qsl("Aktivierung der Zinszahlung"));
  dlg.setMsg(qsl("Gib das Datum an, zu dem die Zinszahlung des Vertrags "
                 "aktiviert werden soll"));
  do
  {
    if (QDialog::Rejected == dlg.exec())
    {
      qInfo() << "interest activation was canceled by the user";
      return;
    }
    if (dlg.date() < earlierstActivation)
    {
      QString msg{
          qsl("Das Datum kann nicht vor dem letzten Buchungsdatum (%1) sein!")
              .arg(earlierstActivation.toString(Qt::ISODate))};
      QMessageBox::information(getMainWindow(), qsl("Ungültiges Datum"), msg);
      continue;
    }
    break;
  } while (true);
  if (not v->bookActivateInterest(dlg.date()))
  {
    QString msg{qsl("Beim der Buchung ist ein Fehler eingetreten - bitte "
                    "schaue in die LOG Datei für weitere Informationen")};
    QMessageBox::warning(getMainWindow(), qsl("Buchungsfehler"), msg);
  }
}

void changeContractValue(contract *pc)
{
  LOG_CALL;
  if (not pc->initialBookingReceived())
  {
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
  qInfo() << wiz.field(fnPayoutInterest);
  if (wiz.field(qsl("confirmed")).toBool())
  {
    double amount{QLocale().toDouble(wiz.field(qsl("amount")).toString())};
    QDate date{wiz.field(qsl("date")).toDate()};
    if (wiz.field(qsl("deposit_notPayment")).toBool())
    {
      pc->deposit(date, amount, wiz.field(fnPayoutInterest).toBool());
    }
    else
    {
      pc->payout(date, amount, wiz.field(fnPayoutInterest).toBool());
    }
  }
  else
    qInfo() << "contract change was cancled by the user";
}

namespace
{
  void print_as_csv(const QDate &bookingDate,
                    const QVector<contract> &changedContracts,
                    const QVector<QDate> &startOfInterrestCalculation,
                    const QVector<booking> &asBookings)
  {
    csvwriter csv(qsl(";"));
    csv.addColumns(
        qsl("Vorname; Nachname; Email; Strasse; Plz; Stadt; IBAN; Kennung; "
            "Auszahlend;"
            "Beginn; Buchungsdatum; Zinssatz; Kreditbetrag; Zins; Endbetrag"));
    QLocale l;
    for (int i = 0; i < changedContracts.count(); i++)
    {
      const contract &c = changedContracts[i];
      const booking &b = asBookings[i];
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
      csv.appendToRow(interestModelDisplayString(c.iModel()));
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
                            i2s(bookingDate.year()));
    csv.saveAndShowInExplorer(filename);
  }
} // namespace
void annualSettlement()
{
  LOG_CALL;
  QDate bookingDate = dateOfnextSettlement();
  if (not bookingDate.isValid() or bookingDate.isNull())
  {
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

  if (dlg.exec() == QDialog::Rejected || not dlg.confirmed())
    return;

  busycursor bc;
  QVector<QVariant> ids = executeSingleColumnSql(
      dkdbstructur[contract::tnContracts][contract::fnId]);
  qInfo() << "going to try annual settlement for contracts w ids:" << ids;
  QVector<contract> changedContracts;
  QVector<QDate> startOfInterrestCalculation;
  QVector<booking> asBookings;
  // try execute annualSettlement for all contracts
  for (const auto &id : qAsConst(ids))
  {
    contract c(id.toLongLong());
    QDate startDate = c.latestBooking().date;
    if (0 not_eq c.annualSettlement(yearOfSettlement))
    {
      if (dlg.print_csv())
      {
        changedContracts.push_back(c);
        asBookings.push_back(c.latestBooking());
        startOfInterrestCalculation.push_back(startDate);
      }
    }
  }
  if (dlg.print_csv())
    print_as_csv(bookingDate, changedContracts, startOfInterrestCalculation,
                 asBookings);
  return;
}

/*************************/
/*** Ausdrucke Jahresend Briefe *******/
/*************************/
namespace
{
  void createInitialLetterTemplates()
  {
    LOG_CALL;
    QDir outDir(appConfig::Outdir());
    outDir.mkdir(qsl("vorlagen"));
    outDir.mkdir(qsl("html"));
    const QString vorlagenVerzeichnis = appConfig::Outdir() + qsl("/vorlagen/");
    extractTemplateFileFromResource(vorlagenVerzeichnis, qsl("brieflogo.png"));
    extractTemplateFileFromResource(vorlagenVerzeichnis, qsl("zinsbrief.css"));
    extractTemplateFileFromResource(vorlagenVerzeichnis, qsl("zinsbrief.html"));
    extractTemplateFileFromResource(vorlagenVerzeichnis, qsl("zinsliste.html"));
    extractTemplateFileFromResource(vorlagenVerzeichnis, qsl("zinsbuchungen.csv"));
#ifdef Q_OS_WIN
    extractTemplateFileFromResource(vorlagenVerzeichnis, qsl("zinsmails-win.bat"), qsl("zinsmails.bat"));
#else
    extractTemplateFileFromResource(vorlagenVerzeichnis, qsl("zinsmails-linux.bat"), qsl("zinsmails.bat"));
#endif
    extractTemplateFileFromResource(vorlagenVerzeichnis, qsl("dkv2mail.bat"));
    extractTemplateFileFromResource(vorlagenVerzeichnis, qsl("dkv2mail"));
    extractTemplateFileFromResource(vorlagenVerzeichnis,
                                    qsl("endabrechnung.html"));
    extractTemplateFileFromResource(vorlagenVerzeichnis,
                                    qsl("endabr-Zinsnachw.html"));
    extractTemplateFileFromResource(vorlagenVerzeichnis,
                                    qsl("endabrechnung.csv"));
    extractTemplateFileFromResource(appConfig::Outdir() + qsl("/html/"),
                                    qsl("zinsbrief.css"));
  }

  int askUserForYearOfPrintouts()
  {
    LOG_CALL;
    QVector<int> years = yearsWithAnnualBookings();
    if (years.isEmpty())
    {
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
                             QDate endDate, bool isTerminated)
{
  QVariantList vl;
  /* get list of contracts */
  QVector<QVariant> ids = executeSingleColumnSql(
      isTerminated ? dkdbstructur[contract::tnExContracts][contract::fnId]
                   : dkdbstructur[contract::tnContracts][contract::fnId],
      qsl(" %1=%2 GROUP BY id").arg(contract::fnKreditorId, i2s(creditorId)));

  for (const auto &id : qAsConst(ids))
  {
    contract contr(id.toLongLong(), isTerminated);
    /* Forget contracts that don't exist in the period.
       i.e. conclusionDate must be before end of period
       and contract must not have been finalized before start of period */
    bool oldFinalizedContract = isTerminated && (contr.plannedEndDate() < startDate);
    if (contr.conclusionDate() <= endDate && oldFinalizedContract == false)
    {
      QVariantMap contractMap = contr.toVariantMap(startDate, endDate);
      vl.append(contractMap);
    }
  }
  return vl;
}

void annualSettlementLetters()
{
  LOG_CALL;
  int yearOfSettlement = askUserForYearOfPrintouts();
  if (yearOfSettlement <= 0)
  {
    qInfo() << "print out canceled by user";
    return;
  }

  busycursor bc;
#if 0
    QVector<booking> annualBookings =
        getAnnualSettlements(yearOfSettlement);

    if (annualBookings.isEmpty())

    {
        bc.finish();
        QMessageBox::information(
            nullptr, qsl("Fehler"),
            qsl("Im Jahr %1 konnten keine Zinsbuchungen gefunden werden. "
                "Es gibt keine Verträge für die eine Abrechnung gemacht werden "
                "kann.")
                .arg(yearOfSettlement));
        return;
    }
#endif

  createInitialLetterTemplates();

  QList<qlonglong> creditorIds;
  creditorsWithAnnualSettlement(creditorIds, yearOfSettlement);
  if (creditorIds.size() <= 0)
  {
    qInfo() << "no Kreditoren to create letters for";
    return;
  }

  QVariantMap printData = {};
  printData[qsl("Zinsjahr")] = yearOfSettlement;
  printData[qsl("Zinsdatum")] =
      QDate(yearOfSettlement, 12, 31).toString(qsl("dd.MM.yyyy"));
  printData[qsl("gmbhLogo")] =
      QVariant(appConfig::Outdir() + qsl("/vorlagen/brieflogo.png"));
  printData[qsl("meta")] = getMetaTableAsMap();

  /* storage for data of all Kreditoren */
  QVariantList Kreditoren;
  QVariantList Auszahlungen;

  double totalBetrag2 = 0.;
  double annualInterest2 = 0;
  double otherInterest2 = 0;
  double interestForPayout2 = 0.;
  double interestCredit2 = 0.;
  for (const auto &cred : qAsConst(creditorIds))
  {
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

    if (vl.size() > 0)
    {
      double payedInterest = 0;
      for (const QVariant &v : qAsConst(vl))
      {
        QVariantMap vm = v.toMap();
        otherInterest += vm["dSonstigeZinsen"].toDouble();
        annualInterest += vm["dJahresZinsen"].toDouble();
        interestForPayout += vm["dAuszahlung"].toDouble();
        interestCredit += vm["dZinsgutschrift"].toDouble();
        totalBetrag += vm["dEndBetrag"].toDouble();
      }
      payedInterest = otherInterest + interestForPayout;
      printData[qsl("ausbezahlterZins")] =
          payedInterest == 0. ? "" : d2euro(payedInterest);
      printData[qsl("mitAusbezahltemZins")] = payedInterest > 0.;
      printData[qsl("mitZins")] = payedInterest + interestCredit > 0.;
      printData[qsl("SumAuszahlung")] =
          interestForPayout == 0. ? "" : d2euro(interestForPayout);
      printData[qsl("dSumJahresZinsen")] = annualInterest;

      printData[qsl("SumJahresZinsen")] =
          annualInterest == 0. ? "" : d2euro(annualInterest);

      printData[qsl("sonstigerZins")] =
          otherInterest == 0. ? "" : d2euro(otherInterest);

      printData["SumZinsgutschrift"] =
          interestCredit == 0. ? "" : d2euro(interestCredit);

      printData[qsl("Vertraege")] = vl;

      printData[qsl("totalBetrag")] = d2euro(totalBetrag);

      QString fileName = qsl("Jahresinfo %1_%3, %4")
                             .arg(i2s(yearOfSettlement),
                                  credRecord.lastname(),
                                  credRecord.firstname().append(qsl(".pdf")));
      /* save data for eMail batch file */
      currCreditorMap[qsl("Vertraege")] = vl;
      currCreditorMap["SumBetrag"] = d2euro(totalBetrag);
      currCreditorMap[qsl("Attachment")] = fileName;
      currCreditorMap[qsl("SumJahresZinsen")] =
          annualInterest == 0. ? "" : d2euro(annualInterest);

      currCreditorMap[qsl("SumSonstigeZinsen")] =
          otherInterest == 0. ? "" : d2euro(otherInterest);

      currCreditorMap[qsl("SumZinsgutschrift")] =
          interestCredit == 0. ? "" : d2euro(interestCredit);

      if (currCreditorMap[qsl("Email")] == "")
      {
        currCreditorMap.remove(qsl("Email"));
      }

      if (interestForPayout > 0.)
      {
        currCreditorMap[qsl("SumAuszahlung")] = d2euro(interestForPayout);
        Auszahlungen.append(currCreditorMap);
      }
      else
      {
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
  printData[qsl("Sum2Betrag")] = d2euro(totalBetrag2);
  printData[qsl("Sum2JahresZinsen")] = d2euro(annualInterest2);
  printData[qsl("Sum2SonstigeZinsen")] = d2euro(otherInterest2);
  printData[qsl("Sum2Auszahlung")] = d2euro(interestForPayout2);
  printData[qsl("Sum2Zinsgutschrift")] = d2euro(interestCredit2);

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
  showInExplorer(appConfig::Outdir(), showFolder);
  qInfo() << "Alles OK";
}

/*************************/
/*** contract endings   **/
/*************************/

void deleteInactiveContract(contract *c)
{
  LOG_CALL;
  // contracts w/o bookings can be deleted
  // todo: if creditor has no other (active or deleted) contracts: propose
  // delete creditor
  if (QMessageBox::question(getMainWindow(), qsl("Vertrag löschen"),
                            qsl("Soll der inaktive Vertrag ") + c->label() +
                                qsl(" gelöscht werden?")) == QMessageBox::Yes)
  {
    if (not c->deleteInactive())
    {
      QMessageBox::information(getMainWindow(), qsl("Fehler"),
                               qsl("Der Vertrag konnte nicht gelöscht werden"));
    }
  }
  if (QMessageBox::question(
          getMainWindow(), qsl("Kreditor*in löschen?"),
          qsl("Soll die zugehörige Kreditgeber*in gelöscht werden?")) ==
      QMessageBox::Yes)
  {
    creditor::remove(c->creditorId());
  }
}

void terminateContract(contract *pc)
{
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

void terminateContract_Final(contract &c)
{
  LOG_CALL;
  wizTerminateContract wiz(getMainWindow(), c);
  wiz.exec();
  if (not wiz.field(qsl("confirm")).toBool())
    return;
  double interest = 0., finalValue = 0.;
  if (not c.finalize(false, wiz.field(qsl("date")).toDate(), interest,
                     finalValue))
  {
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

void cancelContract(contract &c)
{
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
  c.cancel(wiz.field(qsl("date")).toDate());
}

void finalizeContractLetter(contract *c)
{
  LOG_CALL;

  busycursor bc;
  // copy Templates (if not available)
  createInitialLetterTemplates();

  QVariantMap printData = {};
  printData[qsl("gmbhLogo")] =
      QVariant(appConfig::Outdir() + qsl("/vorlagen/brieflogo.png"));
  printData[qsl("meta")] = getMetaTableAsMap();

  creditor credRecord(c->creditorId());
  printData[qsl("creditor")] = QVariant(credRecord.getVariantMap());
  printData[qsl("Vertrag")] = c->toVariantMap();
  printData[qsl("endBetrag")] = d2euro(c->value());
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
  showInExplorer(appConfig::Outdir(), showFolder);
  qInfo() << "Vertragsabschlussdokument erfolgreich angelegt";
}

/*************************/
/*** investments        **/
/*************************/

qlonglong createInvestment_matchingContract(int &interest, QDate &from,
                                            QDate &to)
{
  LOG_CALL;
  // give the user a UI to create a investment which will match a certain set of
  // contract data
  wizNewInvestment wiz;
  wiz.setField(pnZSatz, QVariant(interest));
  wiz.initStartDate(from);
  wiz.setField(pnBis, QVariant(from.addYears(1).addDays(-1)));
  wiz.exec();
  if (not wiz.field(pnKorrekt).toBool())
  {
    qInfo() << "investment wiz was canceled";
    return 0;
  }
  qlonglong newId =
      saveNewInvestment(wiz.field(pnZSatz).toInt(), wiz.field(pnVon).toDate(),
                        wiz.field(pnBis).toDate(), wiz.field(pnTyp).toString());
  if (0 >= newId)
  {
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
void createInvestment()
{
  LOG_CALL;
  wizNewInvestment wiz;
  wiz.initStartDate(QDate::currentDate());
  wiz.exec();
  if (not wiz.field(pnKorrekt).toBool())
  {
    qInfo() << "investment wiz was canceled";
    return;
  }
  if (0 >=
      saveNewInvestment(wiz.field(pnZSatz).toInt(), wiz.field(pnVon).toDate(),
                        wiz.field(pnBis).toDate(), wiz.field(pnTyp).toString()))
  {
    qCritical() << "Investment could not be saved";
    QMessageBox::warning(nullptr, qsl("Fehler"),
                         qsl("Die Geldanlage konnte nicht gespeichert werden"));
  }
}
