
#include <QLabel>
#include <QDateEdit>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QMessageBox>

#include "wizterminatecontract.h"

wizTerminateContract_DatePage::wizTerminateContract_DatePage(QWidget* p) : QWizardPage(p)
{
    setTitle("Vertrag beenden");
    setSubTitle("Mit dieser Dialogfolge kannst Du einen Vertrag beenden.");
    QLabel* l = new QLabel("Gib das Datum des Vertragsende ein. Bis zu diesem Datum werden die Zinsen berechnet."
                           " Es sollte auch das Auszahlungsdatum sein.");
    l->setWordWrap(true);
    QDateEdit* de = new QDateEdit;
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
    QString msg ("Das Vertragsende muss nach der letzten Buchung des Vertrags am %1 liegen");
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
    QString subtitle = "Der Wert des Vertrags beläuft sich auf %1 Euro. "
                       "Inclusive Zins müssen %2 Euro ausbezahlt werden.";
    QString finalV =QString::number(wiz->c.futureValue(field("date").toDate()));
    QString currentV =QString::number(wiz->c.currentValue());
    subtitle = subtitle.arg(currentV).arg(finalV);
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
