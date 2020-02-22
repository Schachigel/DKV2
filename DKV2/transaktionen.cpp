
#include <QString>
#include <QMessageBox>
#include <QDate>

#include "askdatedlg.h"
#include "helper.h"
#include "finhelper.h"
#include "transaktionen.h"


bool vertragBeenden(int vid)
{LOG_ENTRY_and_EXIT;

    Vertrag v;
    v.ausDb(vid, true);
    if( v.isActive())
    {
        if( v.Kuendigungsfrist() == -1)
        {   // festes Vertragsende
            return VertragsEnde_LaufzeitEnde(v);
        }else
        {   // Vertragsende auf ende der kFrist setzten
            return VertragsEnde_MitKFrist(v);
        }
    }
    else
    {
        // passiven Vertrag löschen
        return VertragsEnde_PassiverV(v);
    }
    return false;
}

bool VertragsEnde_LaufzeitEnde( Vertrag& v)
{LOG_ENTRY_and_EXIT;

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
            return true;
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
        return true;
    if( !v.beendeAktivenVertrag(Vertragsende))
        qCritical() << "Deleting the contract failed";

    return true;
}

bool VertragsEnde_MitKFrist( Vertrag& v)
{LOG_ENTRY_and_EXIT;

    int KuendigungsMonate = v.Kuendigungsfrist();
    QString getDateMsg("<h2>Kündigung eines Vertrages mit Kündigungsfrist<h2><p>");
    getDateMsg += "Geben Sie den <b>Kündigungszeitpunkt</b> ein. Das Vertragsende wird dann auf das Datum "
            + QString::number(KuendigungsMonate) + " Monate <b>nach diesem Datum<b> gesetzt.";
    askDateDlg dlg(nullptr, QDate::currentDate());
    dlg.showPrintOption( false);
    dlg.setMsg(getDateMsg);
    dlg.setDateLabel("Zeitpunkt der Kündigung");
    QDate Vertragsende;
    do {
        if( QDialog::Accepted != dlg.exec())
        {
            qDebug() << "Delete contract was aborted by the user";
            return true; // not an error
        }
        Vertragsende = dlg.getDate().addMonths(KuendigungsMonate);
        if( Vertragsende < v.StartZinsberechnung())
        {
            dlg.setMsg( getDateMsg + "<br>Das Vertragsende muss <b>nach</b> der letzten Zinsausschüttung liegen");
            dlg.setDate( v.StartZinsberechnung().addMonths(-1*KuendigungsMonate));
            continue;
        }
        break;
    } while(true);

    return v.kuendigeAktivenVertrag( Vertragsende);
}

bool VertragsEnde_PassiverV(Vertrag& v)
{LOG_ENTRY_and_EXIT;
    QString Vorname = v.Vorname();
    QString Nachname = v.Nachname();
    int index = v.getVid();

    QString msg( "Soll der Vertrag von ");
    msg += Vorname + " " + Nachname + " (id " + index + ") gelöscht werden?";
    if( QMessageBox::Yes != QMessageBox::question(nullptr, "Kreditvertrag löschen", msg))
        return true;  // valid decision, no error
    return v.loeschePassivenVertrag();
}
