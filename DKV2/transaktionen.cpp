
#include <QString>
#include <QMessageBox>
#include <QDate>

#include "askdatedlg.h"
#include "helper.h"
#include "finhelper.h"
#include "letters.h"
#include "transaktionen.h"


void aktiviereVertrag(int vid)
{   LOG_CALL;
    askDateDlg dlg( nullptr, QDate::currentDate());
    dlg.setMsg("<H3>Mit der Aktivierung des Vertrags beginnt die Zinsberechnung. <br>Bitte geben Sie das Datum des Geldeingangs ein:</H3>");
    dlg.setDateLabel("Die Verzinsung beginnt am");
    if( QDialog::Accepted != dlg.exec())
        return;

    Contract v;
    v.loadContractFromDb(vid);
    if( v.activateContract(dlg.getDate()))
    {
        if( dlg.shouldPrint())
            printThankyouLetter(v);
        QMessageBox::information(nullptr, "Aktivieren des Vertrags",
           "Der Vertrag wurde erfolgreich aktiviert");
        return;
    }
    else
    {
        QMessageBox::critical(nullptr, "Fehler beim Aktivieren des Vertrags",
           "Es ist ein Fehler bei der Aktivierung aufgetreten, bitte überprüfen sie das LOG");
        qCritical() << "Das Aktivieren des Vertrags ist fehlgeschlagen";
    }
}

void beendeVertrag(int vid)
{   LOG_CALL;

    Contract v;
    v.loadContractFromDb(vid);
    if( v.isActive())
    {
        if( v.Kuendigungsfrist() == -1)
        {   // festes Vertragsende
            VertragsEnde_LaufzeitEnde(v);
        }else
        {   // Vertragsende auf ende der kFrist setzten
            VertragsEnde_MitKFrist(v);
        }
    }
    else
    {
        // passiven Vertrag löschen
        VertragsEnde_PassiverV(v);
    }
    Q_ASSERT(true);
}
void VertragsEnde_LaufzeitEnde( Contract& v)
{   LOG_CALL;

    // Vertragsende muss >= letzteZinsberechnung sein, sonst wäre zuviel Zins ausbezahlt worden
    QDate moeglichesVertragsende = (v.LaufzeitEnde() > v.StartZinsberechnung()) ? v.LaufzeitEnde() : v.StartZinsberechnung();
    double aktuellerWert = v.Thesaurierend() ? v.Wert() : v.Betrag();

    double WertBisLzE = aktuellerWert + ZinsesZins(v.Zinsfuss(), aktuellerWert, v.StartZinsberechnung(), moeglichesVertragsende, v.Thesaurierend());
    QString getDateMsg("<h2>Wenn Sie einen Vertrag beenden wird der Zins abschließend berechnet und der Auszahlungsbetrag ermittelt.<br></h2>"
                "Zum vereinbarten Laufzeitende des Vertrags von %1 %2 hat dieser den aktuellen Wert %3 Euro.<br>"
                "Sie können auch ein anderes Datum wählen - aber beachten Sie, dass damit unter Umständen nicht der geschlossene Vertrag erfüllt wird."
                "Klicken Sie OK um den Vertrag zu beenden");
    QLocale locale;
    getDateMsg = getDateMsg.arg(v.Vorname(), v.Nachname(), locale.toCurrencyString(WertBisLzE));

    askDateDlg dlg( nullptr, moeglichesVertragsende);
    dlg.setMsg(getDateMsg);
    dlg.setDateLabel("Ende der Zinsberechnung (=Auszahlungstermin) am:");
    do {
        if( QDialog::Accepted != dlg.exec())
        {
            qDebug() << "Delete contract was aborted by the user";
            return;
        }
        if( dlg.getDate() < v.StartZinsberechnung())
        {
            dlg.setMsg( getDateMsg + "<br><b>Das Datum muss nach der letzten Zinsausschüttung liegen</b>");
            dlg.setDate(v.StartZinsberechnung().addDays(1));
            continue;
        }
        break;
    } while(true);

    QDate Vertragsende = dlg.getDate();
    double davonZins =ZinsesZins(v.Zinsfuss(), aktuellerWert, v.StartZinsberechnung(), Vertragsende, v.Thesaurierend());
    double neuerWert =aktuellerWert +davonZins;

    QString confirmDeleteMsg("<h3>Vertragsabschluß</h3><br>Wert zum Vertragsende: %1 Euro<br>Zins der letzten Zinsphase: %2 Euro<br>"\
                             "Soll der Vertrag gelöscht werden?");
    confirmDeleteMsg = confirmDeleteMsg.arg(locale.toCurrencyString(neuerWert), locale.toCurrencyString(davonZins));
    if( QMessageBox::Yes != QMessageBox::question(nullptr, "Vertrag löschen?", confirmDeleteMsg))
        return;
    if( !v.terminateActiveContract(Vertragsende))
    {
        QMessageBox::critical( nullptr, "Vertrag beenden", "Der Vertrag konnte nicht gelöscht werden");
        qCritical() << "Deleting the contract failed";
        return;
    }
    if( dlg.shouldPrint())
        printFinalLetter(v, Vertragsende);
    QMessageBox::information( nullptr, "Vertrag beenden", "Der Vertrag wurde erfolgreich gelöscht");
    return;
}
void VertragsEnde_MitKFrist( Contract& v)
{   LOG_CALL;

    int KuendigungsMonate = v.Kuendigungsfrist();
    QString getDateMsg("<h2>Kündigung eines Vertrages mit Kündigungsfrist<h2><p>");
    getDateMsg += "Geben Sie den <b>Kündigungszeitpunkt</b> ein. Das Vertragsende wird dann auf das Datum "
            + QString::number(KuendigungsMonate) + " Monate <b>nach diesem Datum<b> gesetzt.";
    askDateDlg dlg(nullptr, QDate::currentDate());
    dlg.setMsg(getDateMsg);
    dlg.setDateLabel("Zeitpunkt der Kündigung");
    QDate Vertragsende;
    do {
        if( QDialog::Accepted != dlg.exec())
        {
            qDebug() << "Delete contract was aborted by the user";
            return; // not an error
        }
        Vertragsende = dlg.getDate().addMonths(KuendigungsMonate).addDays(1);
        if( Vertragsende < v.StartZinsberechnung())
        {
            dlg.setMsg( getDateMsg + "<br>Das Vertragsende muss <b>nach</b> der letzten Zinsausschüttung liegen");
            dlg.setDate( v.StartZinsberechnung().addMonths(-1*KuendigungsMonate));
            continue;
        }
        break;
    } while(true);

    if( v.cancelActiveContract( Vertragsende))
    {
        printTerminationLetter(v, dlg.getDate(), KuendigungsMonate);
        QMessageBox::information(nullptr, "Vertragskündigung", QString("Der Vertrag wurde erfolgreich zum ") + dlg.getDate().toString("dd.MM.yyyy") + "gekündigt");
        return;
    }
    else
    {
        QMessageBox::critical(nullptr, "Vertragskündigung", QString("Die Vertragskündigung konnte nicht gespeichert werden"));
        qCritical() << "Fehler beim speichern der Kündigung";
        return;
    }
}
void VertragsEnde_PassiverV(Contract& v)
{   LOG_CALL;
    QString Vorname = v.Vorname();
    QString Nachname = v.Nachname();
    int index = v.getVid();

    QString msg( "Soll der Vertrag von ");
    msg += Vorname + " " + Nachname + " (id " + QString::number(index) + ") gelöscht werden?";
    if( QMessageBox::Yes != QMessageBox::question(nullptr, "Kreditvertrag löschen", msg))
        return;  // valid decision, no error
    if( v.deleteInactiveContract())
    {
        QMessageBox::information(nullptr, "Vertrag löschen", "Der inaktive Vertrag wurde erfolgreich gelöscht");
        return;
    }
    else
    {
        QMessageBox::critical(nullptr, "Vertrag löschen", "Der inaktive Vertrag konnte nicht gelöscht werden");
        return;
    }
}

