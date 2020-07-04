#include <QLabel>
#include <QDateEdit>
#include <QCheckBox>
#include <QVBoxLayout>

#include "wizcancelcontract.h"

wizCancelContract_IntroPage::wizCancelContract_IntroPage(QWidget* p) : QWizardPage(p)
{
    setTitle("Vertragskündigung");
}

void wizCancelContract_IntroPage::initializePage()
{
    wizCancelContract* wiz = dynamic_cast<wizCancelContract*>(wizard());
    QString subTitle("Mit dieser Dialogfolge kannst Du den Vertrag <p><b>%1</b>"
                     "<p>von <p><b>%2</b><p> kündigen. "
                     "Dadurch wird das Vertragsende festgelegt, an dem die Auszahlung erfolgen wird.<p>");
    subTitle = subTitle.arg(wiz->c.label()).arg(wiz->creditorName);
    setSubTitle(subTitle);
}

wizCancelContract_DatePage::wizCancelContract_DatePage(QWidget* p) : QWizardPage(p)
{
    setTitle("Kündigungsdatum");

    QDateEdit* de = new QDateEdit;
    de->setDisplayFormat("dd.MM.yyyy");
    registerField("date", de);
    QVBoxLayout*  layout = new QVBoxLayout;
    layout->addWidget(de);
    setLayout(layout);
}

void wizCancelContract_DatePage::initializePage()
{
    wizCancelContract* wiz = dynamic_cast<wizCancelContract*>(wizard());
    QString subTitle("Das durch die Kündigungsfrist vertraglich vereinbarte Vertragsende ist am <b>%1</b>."
                     "<p>Gib das geplante Vertragsende ein.");
    subTitle =subTitle.arg(wiz->contractualEnd.toString("dd.MM.yyyy"));
    setSubTitle(subTitle);
    setField("date", wiz->contractualEnd);
}

bool wizCancelContract_DatePage::validatePage()
{
    wizCancelContract* wiz = dynamic_cast<wizCancelContract*>(wizard());
    return field("date").toDate() > wiz->c.latestBooking();
}

wizCancelContract_SummaryPage::wizCancelContract_SummaryPage(QWidget* p) : QWizardPage(p)
{
    setTitle("Zusammenfassung");
    QCheckBox* cb = new QCheckBox("Die Eingaben sind korrekt!");
    registerField("confirmed", cb);
    QVBoxLayout* layout = new QVBoxLayout;
    layout-> addWidget(cb);

    setLayout(layout);
}

void wizCancelContract_SummaryPage::initializePage()
{
    wizCancelContract* wiz = dynamic_cast<wizCancelContract*>(wizard());
    QString subt = "Der Vertrag <b>%1</b> <p>von <b>%2</b><p>soll zum <b>%3</b> beendet werden.";
    subt = subt.arg(wiz->c.label()).arg(wiz->creditorName);
    subt = subt.arg(field("date").toDate().toString("dd.MM.yyyy"));
    setSubTitle(subt);
}

bool wizCancelContract_SummaryPage::validatePage()
{
    if( field("confirmed").toBool())
        return true;
    return false;
}

wizCancelContract::wizCancelContract(QWidget* p) : QWizard(p)
{
    addPage(new wizCancelContract_IntroPage);
    addPage(new wizCancelContract_DatePage);
    addPage(new wizCancelContract_SummaryPage);
}
