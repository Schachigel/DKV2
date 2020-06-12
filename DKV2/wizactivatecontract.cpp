#include <QLabel>
#include <QDateEdit>
#include <QLineEdit>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QDebug>

#include "helperfin.h"
#include "wizactivatecontract.h"

wizActivateContract_IntroPage::wizActivateContract_IntroPage(QWidget* p) : QWizardPage(p)
{
    setTitle("Aktivierung eines Vertrags");
}

void wizActivateContract_IntroPage::initializePage()
{
    activateContractWiz* wiz = dynamic_cast<activateContractWiz*>(wizard());
    QString subtitle = "Mit dieser Dialogfolge kannst Du den Vertrag <b>%1</b> von <b>%2</b> aktiviert, "
                       "so dass die Zinsberechnung beginnt.<br>"
                       "Die Aktivierung muss nach dem Geldeingang durchgeführt werden.";
    setSubTitle(subtitle.arg(wiz->label).arg(wiz->creditorName));
}

wizActiateContract_DatePage::wizActiateContract_DatePage(QWidget* p) : QWizardPage(p)
{
    QLabel* l = new QLabel("Aktivierungsdatum");
    QDateEdit* de = new QDateEdit;
    registerField("date", de);
    l->setBuddy(de);
    QVBoxLayout*  layout = new QVBoxLayout;
    layout->addWidget(l);
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
    setSubTitle("Gib die Summe ein, die von dem Kreditor überwiesen wurde. "
                "Diese sollte normalerweise dem Kreditbetrag im Vertrag entsprechen.");
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
    if( amount < 100)
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
    QString subt ="Der Vertrag <b>%1</b> von <b>%2</b> soll mit einem Betrag von <b>%3</b> Euro aktiviert werden. <p>%4";
    activateContractWiz* wiz = dynamic_cast<activateContractWiz*>(wizard());
    double amount = field("amount").toDouble();
    subt = subt.arg(wiz->label).arg(wiz->creditorName).arg(QString::number(amount));
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
