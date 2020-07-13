
#include <QLabel>
#include <QDateEdit>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QMessageBox>

#include "wizterminatecontract.h"

wizTerminateContract_DatePage::wizTerminateContract_DatePage(QWidget* p) : QWizardPage(p)
{
    setTitle("Vertrag beenden");
    setSubTitle("Mit dieser Dialogfolge kannst Du einen Vertrag beenden.<p>"
                "Gib das Datum an, zu dem der Vertrag ausgezahlt wird. "
                "Bis zu diesem Datum werden die Zinsen berechnet. ");
    QDateEdit* de = new QDateEdit;
    de->setDisplayFormat("dd.MM.yyyy");
    registerField("date", de);
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(de);
    setLayout(layout);
}

void wizTerminateContract_DatePage::initializePage()
{
    wizTerminateContract* wiz = dynamic_cast<wizTerminateContract*>(wizard());
    setField("date", wiz->c.plannedEndDate());
}

bool wizTerminateContract_DatePage::validatePage()
{
    wizTerminateContract* wiz = dynamic_cast<wizTerminateContract*>(wizard());
    if( field("date").toDate() >= wiz->c.latestBooking().date)
        return true;
    QString msg ("Das Vertragsende muss nach der letzten Buchung des Vertrags am %1 sein");
    msg = msg.arg(wiz->c.latestBooking().date.toString("dd.MM.yyyy"));
    QMessageBox::information(this, "Ungültiges Datum", msg);
    return false;
}

wizTerminateContract_ConfirmationPage::wizTerminateContract_ConfirmationPage(QWidget* p) : QWizardPage(p)
{
    setTitle("Vertrag beenden");
    QCheckBox* cbPrint = new QCheckBox("Beleg als CSV Datei speichern");
    registerField("print", cbPrint);
    QCheckBox* cbConfirm = new QCheckBox("Daten bestätigen");
    registerField("confirm", cbConfirm);
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(cbPrint);
    layout->addWidget(cbConfirm);
    setLayout(layout);
}

void wizTerminateContract_ConfirmationPage::initializePage()
{
    wizTerminateContract* wiz = dynamic_cast<wizTerminateContract*>(wizard());
    double interest =0., finalValue =0.;
    wiz->c.finalize(true, field("date").toDate(), interest, finalValue);

    QString subtitle = "<table width=100%>"
                       "<tr><td>Bewertung des Vertrags zum Laufzeitende</td><td align=right><b>%1</b> </td></tr>"
                       "<tr><td>Zinsen der letzten Zinsphase</td>            <td align=right><b>%2</b> </td></tr>"
                       "<tr><td>Auszahlungsbetrag </td>                      <td align=right><b>%3</b> </td></tr>"
                       "</table>";
    QLocale locale;
    subtitle = subtitle.arg(locale.toCurrencyString(wiz->c.value())).arg(locale.toCurrencyString(interest)).arg(locale.toCurrencyString(finalValue));
    setSubTitle(subtitle);
}

bool wizTerminateContract_ConfirmationPage::validatePage()
{
    return field("confirm").toBool();
}

wizTerminateContract::wizTerminateContract(QWidget* p, contract c) : QWizard(p), c(c)
{
    addPage(new wizTerminateContract_DatePage);
    addPage(new wizTerminateContract_ConfirmationPage);
}
