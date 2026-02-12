#include "annual_letters.h"

#include "appconfig.h"
#include "dkdbhelper.h"
#include "filewriter.h"
#include "helperfin.h"
#include "helpersql.h"
#include "uihelper.h"
#include "dbstructure.h"

#include "contract.h"
#include "creditor.h"

#include "busycursor.h"
#include "dlginterestletters.h"

#include <QRegularExpression>

void ensureLetterTemplates() {
    LOG_CALL;
    QDir outDir(appconfig::Outdir());
    outDir.mkpath(qsl("."));
    outDir.mkpath(qsl("vorlagen"));
    outDir.mkpath(qsl("html"));
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

namespace {
QString sanitizeFilename(QString name) {
    static const QRegularExpression invalidChars(R"([\\/:*?"<>|])");
    return name.replace(invalidChars, "#");
}

QVariantMap buildLetterBaseData(int yearOfSettlement) {
    QVariantMap basePrintData = {};
    basePrintData[qsl("Zinsjahr")] = yearOfSettlement;
    basePrintData[qsl("Zinsdatum")] =
            QDate(yearOfSettlement, 12, 31).toString(qsl("dd.MM.yyyy"));
    basePrintData[qsl("gmbhLogo")] =
            QVariant(appconfig::Outdir() + qsl("/vorlagen/brieflogo.png"));
    basePrintData[qsl("meta")] = getMetaTableAsMap();
    return basePrintData;
}

struct CreditorLetterData {
    QVariantMap printData;
    QVariantMap creditorMap;
    QString fileName;
    double annualInterest = 0.;
    double otherInterest = 0.;
    double interestForPayout = 0.;
    double interestCredit = 0.;
    double totalBetrag = 0.;
    bool hasContracts = false;
};

struct ContractSummary {
    double annualInterest = 0.;
    double otherInterest = 0.;
    double interestForPayout = 0.;
    double interestCredit = 0.;
    double totalBetrag = 0.;
};

ContractSummary summarizeContracts(const QVariantList &contracts)
{
    ContractSummary summary;
    for (const QVariant &v : std::as_const(contracts)) {
        const QVariantMap vm = v.toMap();
        summary.otherInterest += vm["dSonstigeZinsen"].toDouble();
        summary.annualInterest += vm["dJahresZinsen"].toDouble();
        summary.interestForPayout += vm["dAuszahlung"].toDouble();
        summary.interestCredit += vm["dZinsgutschrift"].toDouble();
        summary.totalBetrag += vm["dEndBetrag"].toDouble();
    }
    return summary;
}

void writeCreditorLetterFiles(const CreditorLetterData &data)
{
    savePdfFromHtmlTemplate(qsl("zinsbrief.html"), data.fileName, data.printData);
}

QVariantMap buildSummaryData(const QVariantMap &basePrintData,
                             const QVariantList &creditors,
                             const QVariantList &payouts,
                             double totalBetrag,
                             double annualInterest,
                             double otherInterest,
                             double interestForPayout,
                             double interestCredit)
{
    QVariantMap summaryData = basePrintData;
    summaryData[qsl("Kreditoren")] = creditors;
    summaryData[qsl("Auszahlungen")] = payouts;
    summaryData[qsl("Sum2Betrag")] = s_d2euro(totalBetrag);
    summaryData[qsl("Sum2JahresZinsen")] = s_d2euro(annualInterest);
    summaryData[qsl("Sum2SonstigeZinsen")] = s_d2euro(otherInterest);
    summaryData[qsl("Sum2Auszahlung")] = s_d2euro(interestForPayout);
    summaryData[qsl("Sum2Zinsgutschrift")] = s_d2euro(interestCredit);
    return summaryData;
}

void writeYearSummaryFiles(const QVariantMap &summaryData, int yearOfSettlement)
{
    writeRenderedTemplate(
                qsl("zinsmails.bat"),
                qsl("zinsmails").append(i2s(yearOfSettlement)).append(qsl(".bat")),
                summaryData);

    savePdfFromHtmlTemplate(
                qsl("zinsliste.html"),
                qsl("Zinsliste-").append(i2s(yearOfSettlement)).append(qsl(".pdf")),
                summaryData);

    writeRenderedTemplate(
                qsl("zinsbuchungen.csv"),
                qsl("zinsbuchungen").append(i2s(yearOfSettlement)).append(qsl(".csv")),
                summaryData);
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

QVector<contract> collectContractsForYear(qlonglong creditorId,
                                          const QDate &startDate,
                                          const QDate &endDate,
                                          bool isTerminated)
{
    QVector<contract> contracts;
    QVector<QVariant> ids = executeSingleColumnSql(
                                isTerminated ? dkdbstructur[contract::tnExContracts][contract::fnId]
                            : dkdbstructur[contract::tnContracts][contract::fnId],
            qsl(" %1=%2 GROUP BY id").arg(contract::fnKreditorId, i2s(creditorId)));

    for (const auto &id : std::as_const(ids)) {
        contract contr(contractId_t {id.toLongLong()}, isTerminated);
        bool finalizedInPeriod = true;
        if (isTerminated) {
            const QDate endDateContract = contr.plannedEndDate();
            finalizedInPeriod = (endDateContract >= startDate && endDateContract <= endDate);
        }
        if (contr.conclusionDate() <= endDate && finalizedInPeriod) {
            contracts.append(contr);
        }
    }
    return contracts;
}

QVariantList getContractList(qlonglong creditorId, QDate startDate,
                             QDate endDate, bool isTerminated) {
    QVariantList vl;
    const QVector<contract> contracts =
            collectContractsForYear(creditorId, startDate, endDate, isTerminated);
    for (const auto &contr : std::as_const(contracts)) {
        QVariantMap contractMap = contr.toVariantMap(startDate, endDate);
        vl.append(contractMap);
    }
    return vl;
}

CreditorLetterData buildCreditorLetterData(qlonglong creditorId,
                                           int yearOfSettlement,
                                           const QVariantMap &basePrintData)
{
    CreditorLetterData data;
    creditor credRecord{creditorId_t {creditorId}};
    QVariantMap currCreditorMap = credRecord.getVariantMap();
    QVariantMap printData = basePrintData;
    printData["creditor"] = currCreditorMap;

    QVariantList vl;
    /* get active contracts */
    vl = getContractList(creditorId, QDate(yearOfSettlement, 1, 1),
                         QDate(yearOfSettlement, 12, 31), false);
    /* get terminated contracts */
    vl.append(getContractList(creditorId, QDate(yearOfSettlement, 1, 1),
                              QDate(yearOfSettlement, 12, 31), true));

    if (vl.size() > 0) {
        const ContractSummary summary = summarizeContracts(vl);
        data.annualInterest = summary.annualInterest;
        data.otherInterest = summary.otherInterest;
        data.interestForPayout = summary.interestForPayout;
        data.interestCredit = summary.interestCredit;
        data.totalBetrag = summary.totalBetrag;

        const double payedInterest = summary.otherInterest + summary.interestForPayout;
        printData[qsl("ausbezahlterZins")] =
                payedInterest == 0. ? "" : s_d2euro(payedInterest);
        printData[qsl("mitAusbezahltemZins")] = payedInterest > 0.;
        printData[qsl("mitZins")] = payedInterest + summary.interestCredit > 0.;
        printData[qsl("SumAuszahlung")] =
                summary.interestForPayout == 0. ? "" : s_d2euro(summary.interestForPayout);
        printData[qsl("dSumJahresZinsen")] = summary.annualInterest;

        printData[qsl("SumJahresZinsen")] =
                summary.annualInterest == 0. ? "" : s_d2euro(summary.annualInterest);

        printData[qsl("sonstigerZins")] =
                summary.otherInterest == 0. ? "" : s_d2euro(summary.otherInterest);

        printData["SumZinsgutschrift"] =
                summary.interestCredit == 0. ? "" : s_d2euro(summary.interestCredit);

        printData[qsl("Vertraege")] = vl;
        printData[qsl("totalBetrag")] = s_d2euro(summary.totalBetrag);

        data.fileName = qsl("Jahresinfo %1_%2, %3.pdf")
                           .arg(i2s(yearOfSettlement),
                                credRecord.lastname(),
                                credRecord.firstname());
        data.fileName = sanitizeFilename(data.fileName);

        /* save data for eMail batch file */
        currCreditorMap[qsl("Vertraege")] = vl;
        currCreditorMap["SumBetrag"] = s_d2euro(summary.totalBetrag);
        currCreditorMap[qsl("Attachment")] = data.fileName;
        currCreditorMap[qsl("SumJahresZinsen")] =
                summary.annualInterest == 0. ? "" : s_d2euro(summary.annualInterest);

        currCreditorMap[qsl("SumSonstigeZinsen")] =
                summary.otherInterest == 0. ? "" : s_d2euro(summary.otherInterest);

        currCreditorMap[qsl("SumZinsgutschrift")] =
                summary.interestCredit == 0. ? "" : s_d2euro(summary.interestCredit);

        if (currCreditorMap[qsl("Email")] == "") {
            currCreditorMap.remove(qsl("Email"));
        }

        if (summary.interestForPayout > 0.) {
            currCreditorMap[qsl("SumAuszahlung")] = s_d2euro(summary.interestForPayout);
        } else {
            currCreditorMap[qsl("SumAuszahlung")] = "";
        }

        data.hasContracts = true;
    }

    data.printData = printData;
    data.creditorMap = currCreditorMap;
    return data;
}
} // namespace

void annualSettlementLetters() {
    LOG_CALL;
    int yearOfSettlement = askUserForYearOfPrintouts();
    if (yearOfSettlement <= 0) {
        qInfo() << "print out canceled by user";
        return;
    }

    busyCursor bc;
    ensureLetterTemplates();

    QList<qlonglong> creditorIds;
    creditorsWithAnnualSettlement(creditorIds, yearOfSettlement);
    if (creditorIds.size() <= 0) {
        qInfo() << "no Kreditoren to create letters for";
        return;
    }

    QVariantMap basePrintData = buildLetterBaseData(yearOfSettlement);

    /* storage for data of all Kreditoren */
    QVariantList Kreditoren;
    QVariantList Auszahlungen;

    double totalBetrag2 = 0.;
    double annualInterest2 = 0;
    double otherInterest2 = 0;
    double interestForPayout2 = 0.;
    double interestCredit2 = 0.;
    for (const auto &cred : std::as_const(creditorIds)) {
        const CreditorLetterData data =
                buildCreditorLetterData(cred, yearOfSettlement, basePrintData);
        if (!data.hasContracts)
            continue;

        if (data.interestForPayout > 0.)
            Auszahlungen.append(data.creditorMap);

        annualInterest2 += data.annualInterest;
        otherInterest2 += data.otherInterest;
        interestForPayout2 += data.interestForPayout;
        interestCredit2 += data.interestCredit;
        totalBetrag2 += data.totalBetrag;
        Kreditoren.append(data.creditorMap);
        /////////////////////////////////////////////////
        writeCreditorLetterFiles(data);
        /////////////////////////////////////////////////
    }

    // Create the eMail Batch file.
    const QVariantMap summaryData = buildSummaryData(
                basePrintData, Kreditoren, Auszahlungen,
                totalBetrag2, annualInterest2, otherInterest2,
                interestForPayout2, interestCredit2);
    writeYearSummaryFiles(summaryData, yearOfSettlement);

    bc.finish();
    showInExplorer(appconfig::Outdir(), showObject::folder);
    qInfo() << "Alles OK";
}
