#include <QLabel>
#include <QDateEdit>
#include <QLineEdit>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QDebug>

#include "appconfig.h"
#include "helper.h"
#include "helperfin.h"
#include "wizactivatecontract.h"

wizActivateContract_IntroPage::wizActivateContract_IntroPage(QWidget* p) : QWizardPage(p)
{
    setTitle("Aktivierung eines Vertrags");
}

void wizActivateContract_IntroPage::initializePage()
{
    activateContractWiz* wiz = qobject_cast<activateContractWiz*>(wizard());
    QString subtitle = qsl("Mit dieser Dialogfolge kannst Du den Vertrag <p><b>%1</b> von <b>%2</b> <p>aktivieren, "
                       "so dass die Zinsberechnung beginnt.<br>"
                       "Die Aktivierung muss nach dem Geldeingang durchgeführt werden.<br>");
    setSubTitle(subtitle.arg(wiz->label, wiz->creditorName));
}

wizActiateContract_DatePage::wizActiateContract_DatePage(QWidget* p) : QWizardPage(p)
{
    QDateEdit* de = new QDateEdit;
    de->setDisplayFormat(qsl("dd.MM.yyyy"));
    registerField(qsl("date"), de);
    QVBoxLayout*  layout = new QVBoxLayout;
    layout->addWidget(de);
    setLayout(layout);
}

void wizActiateContract_DatePage::initializePage()
{
    setTitle(qsl("Aktivierungsdatum"));
    setSubTitle(qsl("Das Aktivierungsdatum entspricht dem Datum, zu dem das Geld auf unserem Konto eingegangen ist"));
}

wizActiateContract_AmountPage::wizActiateContract_AmountPage(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Eingegangener Kreditbetrag"));
    setSubTitle(qsl("Gib die Summe ein, die von dem/der DK-Geber*in überwiesen wurde.<br> "
                "Diese entspricht normalerweise dem im Vertrag vereinbarten Kreditbetrag."));
    QVBoxLayout*  layout = new QVBoxLayout;
    QLabel* l = new QLabel(qsl("Betrag in Euro"));
    QLineEdit* le = new QLineEdit;
    registerField(qsl("amount"), le);
    le->setValidator(new QIntValidator(this));
    l->setBuddy(le);
    layout->addWidget(l);
    layout->addWidget(le);
    setLayout(layout);
}

bool wizActiateContract_AmountPage::validatePage()
{
    activateContractWiz* wiz = qobject_cast<activateContractWiz*>(wizard());
    double amount = field(qsl("amount")).toDouble();
    if( amount < getNumMetaInfo(MIN_AMOUNT))
        return false;
    setField(qsl("amount"), r2(amount));
    if( wiz->expectedAmount != amount) {
        qInfo() << "activation with different amount";
    }
    return true;
}

wizActivateContract_SummaryPage::wizActivateContract_SummaryPage( QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Zusammenfassung"));
    QCheckBox* cb = new QCheckBox(qsl("Die Eingaben sind korrekt!"));
    registerField(qsl("confirmed"), cb);
    QVBoxLayout* layout = new QVBoxLayout;
    layout-> addWidget(cb);
    setLayout(layout);
    connect(cb, SIGNAL(stateChanged(int)), this, SLOT(onConfirmData_toggled(int)));
}

void wizActivateContract_SummaryPage::initializePage()
{
    QString subt =qsl("Der Vertrag <p><b>%1</b> von <b>%2</b><p> soll mit einem Betrag von <p>"
                  "<b>%3 Euro</b><p> zum %4 aktiviert werden. <br>");
    activateContractWiz* wiz = qobject_cast<activateContractWiz*>(wizard());
    double amount = field(qsl("amount")).toDouble();
    subt = subt.arg(wiz->label, wiz->creditorName);
    QLocale locale;
    subt = subt.arg(locale.toCurrencyString(amount), field(qsl("date")).toDate().toString(qsl("dd.MM.yyyy")));
    if( amount != wiz->expectedAmount)
        subt = subt.arg(qsl("<b>Der Überweisungsbetrag stimmt nicht mit dem Kreditbetrag überein.</b>"));
    else
        subt = subt.arg(QString());
    setSubTitle(subt);
}

bool wizActivateContract_SummaryPage::validatePage()
{
    if( field(qsl("confirmed")).toBool())
        return true;
    return false;
}
void wizActivateContract_SummaryPage::onConfirmData_toggled(int )
{
    completeChanged();
}
bool wizActivateContract_SummaryPage::isComplete() const
{
    return field("confirmed").toBool();
}

activateContractWiz::activateContractWiz(QWidget* p) : QWizard (p)
{
    addPage(new wizActivateContract_IntroPage);
    addPage(new wizActiateContract_DatePage);
    addPage(new wizActiateContract_AmountPage);
    addPage(new wizActivateContract_SummaryPage);
}
