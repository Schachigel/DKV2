#include <QDate>
#include "pch.h"

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
//#include "wizannualsettlement.h"
#include "wiznewinvestment.h"
#include "dlgchangecontracttermination.h"
#include "dlgannualsettlement.h"
#include "dlginterestletters.h"
#include "dlgaskdate.h"
#include "wiznew.h"
#include "transaktionen.h"
#include "filewriter.h"

namespace
{
    bool postDB_UpgradeActions(int /*sourceVersion*/, const QString & /*dbName*/)
    {
        //    autoDb db(dbName, qsl("postUpgradeActions"));
        bool ret = true;
        //  do stuff to adapt to new db depending on the source version
        return ret;
    }
}
bool checkSchema_ConvertIfneeded(const QString &origDbFile)
{
    LOG_CALL;

    int version_of_original_file = get_db_version(origDbFile);
    if (version_of_original_file < CURRENT_DB_VERSION)
    {
        qInfo() << "lower version -> converting";
        if (QMessageBox::Yes not_eq QMessageBox::question(getMainWindow(), qsl("Achtung"), qsl("Das Format der Datenbank ist veraltet. Soll die Datenbank konvertiert werden?")))
        {
            qInfo() << "conversion rejected by user";
            return false;
        }
        QString backup = convert_database_inplace(origDbFile);
        if (backup.isEmpty())
        {
            QMessageBox::critical(getMainWindow(), qsl("Fehler"), qsl("Bei der Konvertierung ist ein Fehler aufgetreten. Die Ausführung muss beendet werden."));
            qCritical() << "db converstion of older DB failed";
            return false;
        }
        // actions which depend on the source version of the db
        if (not postDB_UpgradeActions(version_of_original_file, origDbFile))
        {
            QMessageBox::critical(getMainWindow(), qsl("Fehler"), qsl("Bei der Konvertierung ist ein Fehler aufgetreten. Die Ausführung muss beendet werden."));
            qCritical() << "db converstion of older DB failed";
            return false;
        }
        QMessageBox::information(nullptr, qsl("Erfolgsmeldung"), qsl("Die Konvertierung ware erfolgreich. Eine Kopie der ursprünglichen Datei liegt unter \n") + backup);
        return true;
    }
    else if (version_of_original_file == CURRENT_DB_VERSION)
    {
        return validateDbSchema(origDbFile, dkdbstructur);
    }
    else
    {
        qInfo() << "higher version ? there is no way back";
        return false;
    }
}

void newCreditorAndContract()
{
    LOG_CALL;
    wizNew wiz(getMainWindow());
    wiz.setField(pnNew, true);
    wiz.setField(pnConfirmContract, false);

    /*auto wizRes =*/wiz.exec();
    if (wiz.field(pnNew).toBool())
    {
        if (not wiz.createCreditor)
        {
            qInfo() << "User decided against creditor creation";
            return;
        }
        // one can only come here, if the users accepted the creation of the creditor
        if (not wiz.c_tor.isValid())
        {
            // the user was checked during validation of the wizard -> very wrong
            qCritical() << "invalid creditor data -> we have to fail";
            return;
        }

        if (wiz.c_tor.save() >= 0)
            qInfo() << "creditor created successfully";
        else
        {
            QMessageBox::critical(getMainWindow(), qsl("Programm Fehler"), qsl("Die Kundeninfo konnte nicht "
                                                                               "gespeichert werden. Details findest Du in der Log Datei."));
            return;
        }
    }
    else
    {
        wiz.c_tor.setId(wiz.creditorId);
        qInfo() << "contract for existing creditor will be created";
    }

    if (not wiz.field(pnConfirmContract).toBool())
    {
        qInfo() << "user decided not to save the contract";
        return;
    }
    contract cont;
    cont.setCreditorId(wiz.c_tor.id());
    cont.setPlannedInvest(wiz.field(pnAmount).toDouble());
    cont.setInterestRate(wiz.interest / 100.);
    cont.setInvestment(wiz.investmentId);
    cont.setLabel(wiz.field(pnLabel).toString());
    cont.setConclusionDate(wiz.field(pnCDate).toDate());
    cont.setNoticePeriod(wiz.noticePeriod);
    cont.setPlannedEndDate(wiz.field(pnEDate).toDate());
    cont.setInterestModel(wiz.iPaymentMode);
    cont.setComment(wiz.field(pnContractComment).toString());
    cont.setInterestActive(not wiz.field(pnIPaymentDelayed).toBool());
    if (-1 == cont.saveNewContract())
    {
        qCritical() << "New contract could not be saved";
        QMessageBox::critical(getMainWindow(), qsl("Fehler"), qsl("Der Vertrag konnte nicht "
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
    wiz.selectCreateContract = false;
    //wiz.setField(pnConfirmContract, false);
    wiz.creditorId = creditorId;
    wiz.setStartId(page_address);

    if (QDialog::Accepted == wiz.exec())
    {
        wiz.c_tor.setId(creditorId);
        if (wiz.c_tor.update())
            qInfo() << "successfully updated creditor";
        else
        {
            QMessageBox::critical(getMainWindow(), qsl("Programm Fehler"), qsl("Die Kundeninfo konnte nicht "
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
    ipd.setLabelText(qsl("Ändere den Kommentar zum Vertrag von ") + cred.firstname() + qsl(" ") + cred.lastname());
    ipd.setOption(QInputDialog::UsePlainTextEditForTextInput, true);
    if (ipd.exec() not_eq QDialog::Accepted)
    {
        qInfo() << "inpud dlg canceled";
        return;
    }
    if (pc->updateComment(ipd.textValue().trimmed()))
        qInfo() << "successfully updated comment";
    else
        qCritical() << "update comment failed";
}
void changeContractTermination(contract *pc)
{
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

void activateContract(contract *v)
{
    LOG_CALL;
    creditor cred(v->creditorId());

    wizInitialPayment wiz(getMainWindow());
    wiz.label = v->label();
    wiz.creditorName = cred.firstname() + qsl(" ") + cred.lastname();
    wiz.expectedAmount = v->plannedInvest();
    wiz.setField(fnAmount, v->plannedInvest());
    wiz.setField(fnDate, v->conclusionDate().addDays(1));
    wiz.minimalActivationDate = v->conclusionDate().addDays(1);
    wiz.delayedInterest = not v->interestActive();
    wiz.exec();
    if (not wiz.field(qsl("confirmed")).toBool())
    {
        qInfo() << "contract activation canceled by the user";
        return;
    }
    if (not v->bookInitialPayment(wiz.field(fnDate).toDate(), wiz.field(fnAmount).toDouble()))
    {
        qCritical() << "activation failed";
        Q_ASSERT(false);
    }
    return;
}
void activateInterest(contract *v)
{
    LOG_CALL;
    Q_ASSERT(v->latestBooking().type != booking::Type::non);
    QDate earlierstActivation = v->latestBooking().date.addDays(1);
    dlgAskDate dlg(getMainWindow());
    dlg.setDate(earlierstActivation);
    dlg.setHeader(qsl("Aktivierung der Zinszahlung"));
    dlg.setMsg(qsl("Gib das Datum an, zu dem die Zinszahlung des Vertrags aktiviert werden soll"));
    do
    {
        if (QDialog::Rejected == dlg.exec())
        {
            qInfo() << "interest activation was canceled by the user";
            return;
        }
        if (dlg.date() < earlierstActivation)
        {
            QString msg{qsl("Das Datum kann nicht vor dem letzten Buchungsdatum (%1) sein!").arg(earlierstActivation.toString(Qt::ISODate))};
            QMessageBox::information(getMainWindow(), qsl("Ungültiges Datum"), msg);
            continue;
        }
        break;
    } while (true);
    if (not v->bookActivateInterest(dlg.date()))
    {
        QString msg{qsl("Beim der Buchung ist ein Fehler eingetreten - bitte schaue in die LOG Datei für weitere Informationen")};
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
    wiz.setField(qsl("deposit_notPayment"), QVariant(true));

    wiz.exec();
    if (wiz.field(qsl("confirmed")).toBool())
    {
        double amount{wiz.field(qsl("amount")).toDouble()};
        QDate date{wiz.field(qsl("date")).toDate()};
        qDebug() << wiz.field(qsl("deposit_notPayment")) << ", " << amount << ", " << date;
        if (wiz.field(qsl("deposit_notPayment")).toBool())
        {
            pc->deposit(date, amount);
        }
        else
        {
            pc->payout(date, amount);
        }
    }
    else
        qInfo() << "contract change was cancled by the user";
}
void annualSettlement()
{
    LOG_CALL;
    QDate bookingDate = bookings::dateOfnextSettlement();
    if (not bookingDate.isValid() or bookingDate.isNull())
    {
        QMessageBox::information(nullptr, qsl("Fehler"),
                                 qsl("Ein Jahr für die nächste Zinsberechnung konnte nicht gefunden werden."
                                     "Es gibt keine Verträge für die eine Abrechnung gemacht werden kann."));
        return;
    }
    int yearOfSettlement = bookingDate.year();
    //    wizAnnualSettlement wiz(getMainWindow());
    dlgAnnualsettlement dlg(getMainWindow(), yearOfSettlement);

    dlg.exec();
    if (not dlg.confirmed())
        return;

    QVector<QVariant> ids = executeSingleColumnSql(dkdbstructur[contract::tnContracts][contract::fnId]);
    qDebug() << "contracts to try execute annual settlement for: " << ids;
    QVector<contract> changedContracts;
    QVector<QDate> startOfInterrestCalculation;
    QVector<booking> asBookings;
    for (const auto &id : qAsConst(ids))
    {
        contract c(id.toLongLong());
        QDate startDate = c.latestBooking().date;
        if (0 not_eq c.annualSettlement(yearOfSettlement))
        {
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
    for (int i = 0; i < changedContracts.count(); i++)
    {
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
    filename = filename.arg(QDate::currentDate().toString(Qt::ISODate), QString::number(yearOfSettlement));
    csv.saveAndShowInExplorer(filename);
    return;
}

void interestLetters()
{
    LOG_CALL;
    QVector<int> years =bookings::yearsWithAnnualBookings();
    if( years.size () == 0) {
        QMessageBox::information (getMainWindow (), qsl("Keine Daten"), qsl("Es liegen keine Abrechnungen zum Ausdruck vor"));
        return;
    }
    dlgInterestLetters dlg(getMainWindow(), years);

    dlg.exec();
    if (not dlg.confirmed())
        return;

    int yearOfSettlement = dlg.getYear();
    QVector<booking> annualBookings = bookings::getAnnualSettelments(yearOfSettlement);

    if (annualBookings.size() == 0) {
        QMessageBox::information(nullptr, qsl("Fehler"),
                                 qsl("Im Jahr %1 konnten keine Zinsbuchungen gefunden werden. "
                                     "Es gibt keine Verträge für die eine Abrechnung gemacht werden kann.")
                                     .arg(yearOfSettlement));
        return;
    }
    extractTemplateFileFromResource(appConfig::Outdir () +qsl("/vorlagen/"), qsl("brieflogo.png"));
    extractTemplateFileFromResource(appConfig::Outdir () +qsl("/vorlagen/"), qsl("zinsbrief.css"));
    extractTemplateFileFromResource(appConfig::Outdir () +qsl("/vorlagen/"), qsl("zinsbrief.html"));

    QVariantMap printData = {};
    printData["Zinsjahr"] = yearOfSettlement;
    printData["Zinsdatum"] = QDate(yearOfSettlement, 12, 31).toString("dd.MM.yyyy");
    QVariantMap mm = getMetaMap();
    QString logo = (appConfig::Outdir() + "/vorlagen/brieflogo.png");
    mm["gmbhLogo"] = logo;
    printData["meta"] = mm;

    QList<QPair<int, QString>> creditors;
    KreditorenListeMitId(creditors, yearOfSettlement);
    if (creditors.size() > 0)
    {
        for (auto &cred : qAsConst(creditors))
        {
            creditor credRecord(cred.first);
            printData["creditor"] = credRecord.getVariant();

            QVector<QVariant> ids = executeSingleColumnSql(
                dkdbstructur[contract::tnContracts][contract::fnId],
                qsl(" %1=%2 GROUP BY id").arg(contract::fnKreditorId, QString::number(cred.first)));

            QVariantList vl;
            for (const auto &id : qAsConst(ids))
            {
                contract contr(id.toLongLong());
                vl.append(contr.toVariantMap_4annualBooking(yearOfSettlement));
            }
            printData["Vertraege"] = vl;
            QString fileName = QDate::currentDate().toString(qsl("yyyy-MM-dd")).append("-Zinsen").append(QString::number(yearOfSettlement)).append("_").append(QString::number(credRecord.id())).append("_").append(credRecord.lastname()).append(".pdf");
            pdfWrite(qsl("Zinsbrief"), fileName, printData);
        }
        qInfo() << "Alles OK";
    }
}

void deleteInactiveContract(contract *c)
{
    LOG_CALL;
    // contracts w/o bookings can be deleted
    // todo: wiz ui with confirmation?
    // if creditor has no other contracts: delete creditor
    contract::remove(c->id());
}
void terminateContract(contract *pc)
{
    LOG_CALL;
    if (pc->hasEndDate())
    {
        terminateContract_Final(*pc);
    }
    else
    {
        cancelContract(*pc);
    }
}
void terminateContract_Final(contract &c)
{
    LOG_CALL;
    wizTerminateContract wiz(getMainWindow(), c);
    wiz.exec();
    if (not wiz.field(qsl("confirm")).toBool())
        return;
    double interest = 0., finalValue = 0.;
    if (not c.finalize(false, wiz.field(qsl("date")).toDate(), interest, finalValue))
    {
        qDebug() << "failed to terminate contract";
    }
    return;
}
void cancelContract(contract &c)
{
    LOG_CALL;
    wizCancelContract wiz(getMainWindow());
    wiz.c = c;
    wiz.creditorName = executeSingleValueSql(qsl("Vorname || ' ' || Nachname"), qsl("Kreditoren"), qsl("id=") + QString::number(c.creditorId())).toString();
    wiz.contractualEnd = QDate::currentDate().addMonths(c.noticePeriod());
    wiz.exec();
    if (not wiz.field(qsl("confirmed")).toBool())
    {
        qInfo() << "cancel wizard canceled by user";
        return;
    }
    c.cancel(wiz.field(qsl("date")).toDate());
}

qlonglong createInvestment(int &interest, QDate &from, QDate &to)
{
    LOG_CALL;
    // give the user a UI to create a investment which will match a certain set of contract data
    wizNewInvestment wiz;
    wiz.setField(pnZSatz, QVariant(interest));
    wiz.setField(pnVon, QVariant(from));
    wiz.setField(pnBis, QVariant(from.addYears(1).addDays(-1)));
    wiz.exec();
    if (not wiz.field(pnKorrekt).toBool())
    {
        qInfo() << "investment wiz was canceled";
        return 0;
    }
    qlonglong newId = saveNewInvestment(wiz.field(pnZSatz).toInt(),
                                        wiz.field(pnVon).toDate(),
                                        wiz.field(pnBis).toDate(),
                                        wiz.field(pnTyp).toString());
    if (0 >= newId)
    {
        qCritical() << "Investment could not be saved";
        QMessageBox::warning(nullptr, qsl("Fehler"), qsl("Die Geldanlage konnte nicht gespeichert werden"));
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
    wiz.setField(pnVon, QDate::currentDate());
    wiz.exec();
    if (not wiz.field(pnKorrekt).toBool())
    {
        qInfo() << "investment wiz was canceled";
        return;
    }
    if (0 >= saveNewInvestment(wiz.field(pnZSatz).toInt(),
                               wiz.field(pnVon).toDate(),
                               wiz.field(pnBis).toDate(),
                               wiz.field(pnTyp).toString()))
    {
        qCritical() << "Investment could not be saved";
        QMessageBox::warning(nullptr, qsl("Fehler"), qsl("Die Geldanlage konnte nicht gespeichert werden"));
    }
}
