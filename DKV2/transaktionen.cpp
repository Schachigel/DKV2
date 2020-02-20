
#include <QString>
#include <QMessageBox>
#include <QDate>

#include "askdatedlg.h"
#include "helper.h"
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
    return true;
}

bool VertragsEnde_MitKFrist( Vertrag& v)
{LOG_ENTRY_and_EXIT;

    QString getDateMsg("<h2>Kündigung eines Vertrages mit Kündigungsfrist<h2><p>");
    getDateMsg += "Geben Sie den <b>Kündigungszeitpunkt</b> ein. Das Vertragsende wird dann auf das Datum "
            + QString::number(v.Kuendigungsfrist()) + " Monate <b>nach diesem Datum<b> gesetzt";
    askDateDlg dlg(nullptr, QDate::currentDate());
    dlg.showPrintOption( false);
    dlg.setMsg(getDateMsg);
    dlg.setDateLabel("Zeitpunkt der Kündigung");
    do {
        if( QDialog::Accepted != dlg.exec())
        {
            qDebug() << "Delete contract was aborted by the user";
            return true; // not an error
        }
        if( dlg.getDate() < v.StartZinsberechnung())
        {
            dlg.setMsg( getDateMsg + "<br><b>Das Datum muss nach der letzten Zinsausschüttung liegen</b>");
            dlg.setDate(v.StartZinsberechnung().addDays(1));
            continue;
        }
        break;
    } while(true);
    // set kfrist = -1, Laufzeitende
    return v.kuendigeAktivenVertrag( dlg.getDate());
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
