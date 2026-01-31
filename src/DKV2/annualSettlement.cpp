#include "annualSettlement.h"

#include "helperfile.h"
#include "appconfig.h"
#include "dkdbviews.h"
#include "creditor.h"
#include "csvwriter.h"


// NON MEMBER FUNCTIONS
namespace {
QString print_as_csv(const QDate &bookingDate,
                  const QVector<contract> &changedContracts,
                  const QVector<QDate> &startOfInterrestCalculation,
                  const QVector<booking> &asBookings) {

    csvWriter csv;
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
    return csv.toString();
    // QString filename{qsl("%1_Jahresabrechnung-%2.csv")};
    // filename = filename.arg(QDate::currentDate().toString(Qt::ISODate),
    //                         i2s(bookingDate.year()));
    // csv.saveAndShowInExplorer(filename);
}
} // Eo local namespace

QDate dateOfnextSettlement()
{
    /*
     * Man sollte eine Jahresendbuchung auch mehrmals machen können, für den Fall, dass es nachträglich
     * gebuchte Geldeingänge für Neuverträge (=Aktivierungen) gab
    */
    QVariant ret =executeSingleValueSql(qsl("SELECT date FROM (%1)").arg(sqlNextAnnualSettlement));

    if( ret.canConvert<QDate>()) {
        RETURN_OK(ret.toDate(),
                  qsl("dateOfNextSettlement: Date of next Settlement was found to be %1").arg(ret.toDate().toString (Qt::ISODate)));
    }
    RETURN_OK(QDate(), qsl("dateOfNextSettlement: Es wurde kein Datum für eine Jahresabrechnung gefunden"));
}

// for all contracts
int executeCompleteAS( year AS_year/*, QString &csv*/)
{
    QVector<QSqlRecord> contractData;
    QVector<QPair<QString, QVariant>>parameterYear{{qsl(":YEAR"), QVariant(AS_year)}};
    if( not executeSql(sqlContractDataForAnnualSettlement, parameterYear, contractData)) {
        qCritical() << "executeCompleteAS: execution of sql failed";
        return 0;
    }
    if( contractData.isEmpty()) {
        qCritical() << "nothing to do: There are no contracts for AS";
        return 0;
    }

    int nbrOfSuccessfull_AS {0};
    for (const auto &C : std::as_const(contractData))
    {
        contract c(C);
        qInfo() << c.toString();
        QDate startDate = c.latestBookingDate();
        if( not startDate.isValid()) {
            qCritical() << qsl("contract %1 seems to have no bookings").arg(c.id_aS());
            continue; // keine Buchung, keine (erst-)Einzahlung. ToDo: unnötig, wenn vorab die passenden Verträge ausgesucht wurden
        }

        /////////////////////////////////////////////////////
        int aSResult {c.annualSettlement(AS_year)};
        ////////////////////////////////////////////////////
        if( aSResult == AS_year) {
            qInfo() << qsl("Jährl. Zinsabrechnung für Vertrag %1 (%2) erfolgreich").arg( c.id_aS(),  c.label ());
            nbrOfSuccessfull_AS +=1;
        }
        else if( aSResult == 0)
            qCritical() << qsl("Keine jährl. Zinsabrechnung für Vertrag %1 (%2) möglich").arg( c.id_aS(),  c.label ());
        else
            qCritical() << qsl("Zinsabrechnung nur erfolgreich für Vertrag %1 (%2) bis %3").arg(c.id_aS(), c.label (), QString::number(aSResult));
    }
    return  nbrOfSuccessfull_AS;
}

QString formulate_AS_as_CSV(year y)
{
    return QString();
}

QString AS_filename(year y)
{
    QString projectName {dbConfig::readValue(projectConfiguration::GMBH_PROJECT).toString()};
    projectName =makeSafeFileName(projectName, 9);

    QString filename{qsl("%1_JA-%2_%3.csv")};
    filename = filename.arg(QDate::currentDate().toString(Qt::ISODate),
                             i2s(y), projectName);
    return filename;
}

void writeAnnualSettlementCsv(year y)
{
    QString content =formulate_AS_as_CSV(y);
}
