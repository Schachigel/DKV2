#include <QLabel>
#include <QDateEdit>
#include <QLineEdit>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QDebug>

#include "appconfig.h"
#include "helperfin.h"
#include "wizactivatecontract.h"

wizActivateContract_IntroPage::wizActivateContract_IntroPage(QWidget* p) : QWizardPage(p)
{
    setTitle("Aktivierung eines Vertrags");
}

void wizActivateContract_IntroPage::initializePage()
{
    activateContractWiz* wiz = dynamic_cast<activateContractWiz*>(wizard());
    QString subtitle = "Mit dieser Dialogfolge kannst Du den Vertrag <p><b>%1</b> von <b>%2</b> <p>aktivieren, "
                       "so dass die Zinsberechnung beginnt.<br>"
                       "Die Aktivierung muss nach dem Geldeingang durchgeführt werden.<br>";
    setSubTitle(subtitle.arg(wiz->label).arg(wiz->creditorName));
}

wizActiateContract_DatePage::wizActiateContract_DatePage(QWidget* p) : QWizardPage(p)
{
    QDateEdit* de = new QDateEdit;
    de->setDisplayFormat("dd.MM.yyyy");
    registerField("date", de);
    QVBoxLayout*  layout = new QVBoxLayout;
    layout->addWidget(de);
    setLayout(layout);
}

void wizActiateContract_DatePage::initializePage()
{
    setTitle("Aktivierungsdatum");
    setSubTitle("Das Aktivierungsdatum entspricht dem Datum, zu dem das Geld auf unserem Konto eingegangen ist");
}

wizActiateContract_AmountPage::wizActiateContract_AmountPage(QWidget* p) : QWizardPage(p)
{
    setTitle("Eingegangener Kreditbetrag");
    setSubTitle("Gib die Summe ein, die von dem/der DK-Geber*in überwiesen wurde.<br> "
                "Diese entspricht normalerweise dem im Vertrag vereinbarten Kreditbetrag.");
    QVBoxLayout*  layout = new QVBoxLayout;
    QLabel* l = new QLabel("Betrag in Euro");
    QLineEdit* le = new QLineEdit;
    registerField("amount", le);
    l->setBuddy(le);
    layout->addWidget(l);
    layout->addWidget(le);
    setLayout(layout);
}

bool wizActiateContract_AmountPage::validatePage()
{
    activateContractWiz* wiz = dynamic_cast<activateContractWiz*>(wizard());
    double amount = field("amount").toDouble();
    if( amount < getNumMetaInfo(MIN_AMOUNT))
        return false;
    setField("amount", round2(amount));
    if( wiz->expectedAmount != amount) {
        qInfo() << "activation with different amount";
    }
    return true;
}

wizActivateContract_SummaryPage::wizActivateContract_SummaryPage( QWidget* p) : QWizardPage(p)
{
    setTitle("Zusammenfassung");
    QCheckBox* cb = new QCheckBox("Die Eingaben sind korrekt!");
    registerField("confirmed", cb);
    QVBoxLayout* layout = new QVBoxLayout;
    layout-> addWidget(cb);
    setLayout(layout);
}

void wizActivateContract_SummaryPage::initializePage()
{
    QString subt ="Der Vertrag <p><b>%1</b> von <b>%2</b><p> soll mit einem Betrag von <p>"
                  "<b>%3 Euro</b><p> zum %4 aktiviert werden. <br>";
    activateContractWiz* wiz = dynamic_cast<activateContractWiz*>(wizard());
    double amount = field("amount").toDouble();
    subt = subt.arg(wiz->label).arg(wiz->creditorName);
    QLocale locale;
    subt = subt.arg(locale.toCurrencyString(amount)).arg(field("date").toDate().toString("dd.MM.yyyy"));
    if( amount != wiz->expectedAmount)
        subt = subt.arg("<b>Der Überweisungsbetrag stimmt nicht mit dem Kreditbetrag überein.</b>");
    else
        subt = subt.arg("");
    setSubTitle(subt);
}

bool wizActivateContract_SummaryPage::validatePage()
{
    if( field("confirmed").toBool())
        return true;
    return false;
}

activateContractWiz::activateContractWiz(QWidget* p) : QWizard (p)
{
    addPage(new wizActivateContract_IntroPage);
    addPage(new wizActiateContract_DatePage);
    addPage(new wizActiateContract_AmountPage);
    addPage(new wizActivateContract_SummaryPage);
}
