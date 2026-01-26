#include "annualSettlement.h"
#include "dkdbviews.h"
#include "creditor.h"
#include "contract.h"
#include "csvwriter.h"
#include "dbstructure.h"


// NON MEMBER FUNCTIONS
namespace {
void print_as_csv(const QDate &bookingDate,
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
    QString filename{qsl("%1_Jahresabrechnung-%2.csv")};
    filename = filename.arg(QDate::currentDate().toString(Qt::ISODate),
                            i2s(bookingDate.year()));
    csv.saveAndShowInExplorer(filename);
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
int executeAnnualSettlement( int year)
{
    // TEST new vs old creatrion of CSV Fiels
    QVector<contract> changedContracts;
    QVector<QDate> startOfInterrestCalculation;
    QVector<booking> asBookings;
    // OLD version FOR TESTING ... ToDo
    QVector<QVariant> ids = executeSingleColumnSql(
        dkdbstructur[contract::tnContracts][contract::fnId]);
    // todo: beschränken auf Verträge, für die eine AS gemacht werden kann durch passendes SQL

    // try execute annualSettlement for all contracts
    for (const auto &id : std::as_const(ids))
    {
        contract c(id.toLongLong());
        QDate startDate = c.latestBookingDate();
        if( not startDate.isValid())
            continue; // keine Buchung, keine (erst-)Einzahlung. ToDo: unnötig, wenn vorab die passenden Verträge ausgesucht wurden

        /////////////////////////////////////////////////////
        if (0 == c.annualSettlement(year))
        ////////////////////////////////////////////////////
        {
            qInfo() << "Keine jährl. Zinsabrechnung für Vertrag " << c.id () << ": " << c.label ();
        }
        else
        {   // for testing new vs. old csv creatrion
            //       TEMPORARY
            changedContracts.push_back(c);
            asBookings.push_back(c.latestBooking());
            startOfInterrestCalculation.push_back(startDate);
            // TEMPRARY TODO: REMOVE
        }
    }
    // print_as_csv(QDate(year, 12, 31), changedContracts, startOfInterrestCalculation, asBookings);

    return changedContracts.size();
}

void writeAnnualSettlementCsv(int year) {

}
