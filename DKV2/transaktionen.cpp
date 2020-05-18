
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

    contract v;
Q_ASSERT(!"repair");
}

void beendeVertrag(int vid)
{   LOG_CALL;
Q_ASSERT(!"repair");
    contract v;
}
void VertragsEnde_LaufzeitEnde( contract& v)
{   LOG_CALL;
Q_ASSERT(!"repair");
    return;
}
void VertragsEnde_MitKFrist( contract& v)
{   LOG_CALL;
Q_ASSERT(!"repair");
}
void VertragsEnde_PassiverV(contract& v)
{   LOG_CALL;
Q_ASSERT(!"repair");
}

