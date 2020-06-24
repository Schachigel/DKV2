
#include <QLabel>
#include <QDateEdit>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QMessageBox>

#include "wizterminatecontract.h"

wizTerminateContract_DatePage::wizTerminateContract_DatePage(QWidget* p) : QWizardPage(p)
{
    setTitle("Vertrag beenden");
    setSubTitle("Mit dieser Dialogfolge kannst Du einen Vertrag beenden.<br>"
                "Gib das Datum an, zu dem der Vertrag ausgezahlt wird. <br>"
                "Bis zu diesem Datum werden die Zinsen berechnet. ");
    QLabel* l = new QLabel("Vertragsende");
    l->setWordWrap(true);
    QDateEdit* de = new QDateEdit;
    de->setDisplayFormat("dd.MM.yyyy");
    registerField("date", de);
    l->setBuddy(de);
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(l);
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
    if( field("date").toDate() >= wiz->c.latestBooking())
        return true;
    QString msg ("Das Vertragsende muss nach der letzten Buchung des Vertrags am %1 sein");
    msg = msg.arg(wiz->c.latestBooking().toString("dd.MM.yyyy"));
    QMessageBox::information(this, "Ungültiges Datum", msg);
    return false;
}

wizTerminateContract_ConfirmationPage::wizTerminateContract_ConfirmationPage(QWidget* p) : QWizardPage(p)
{
    setTitle("Vertrag beenden");
    QCheckBox* cbPrint = new QCheckBox("Beleg als PDF speichern");
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

    QString subtitle = "Bewertung des Vertrags zum Laufzeit Ende: <b>%1 Euro</b><br>"
                       "Zinsen seit letzter Berechnung: <b>%2 Euro</b>"
                       "Auszahlungsbetrag: <b>%3</b>";
    subtitle = subtitle.arg(wiz->c.value()).arg(interest).arg(finalValue);
    setSubTitle(subtitle);
}

bool wizTerminateContract_ConfirmationPage::validatePage()
{
    return field("confirm").toBool();
}

wizTerminateContract::wizTerminateContract(QWidget* p, const contract& c) : QWizard(p), c(c)
{
    addPage(new wizTerminateContract_DatePage);
    addPage(new wizTerminateContract_ConfirmationPage);
}
