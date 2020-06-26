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
    QString subTitle("Mit dieser Dialogfolge kannst Du den Vertrag <br><b>%1</b> von <b>%2</b><br> kündigen. "
                "Dadurch wird das Vertragende festgelegt, an dem die Auszahlung erfolgen wird.");
    subTitle = subTitle.arg(wiz->c.label()).arg(wiz->creditorName);
    setSubTitle(subTitle);
}

wizCancelContract_DatePage::wizCancelContract_DatePage(QWidget* p) : QWizardPage(p)
{
    QLabel* l = new QLabel("Vertragsende");
    QDateEdit* de = new QDateEdit;
    de->setDisplayFormat("dd.MM.yyyy");
    registerField("date", de);
    l->setBuddy(de);
    QVBoxLayout*  layout = new QVBoxLayout;
    layout->addWidget(l);
    layout->addWidget(de);
    setLayout(layout);
}

void wizCancelContract_DatePage::initializePage()
{
    wizCancelContract* wiz = dynamic_cast<wizCancelContract*>(wizard());
    QString subTitle("Das vertraglich vorgesehene Vertragende ist frühestens am %1.");
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
    QString subt = "Der Vertrag %1 von %2 soll zum %3 beendet werden.";
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
