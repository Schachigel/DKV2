#include <QDebug>
#include <QVariant>

#include <QLabel>
#include <QLineEdit>
#include <QDateEdit>
#include <QRadioButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QMessageBox>

#include "helperfin.h"
#include "wizchangecontractvalue.h"

wizChangeContract_IntroPage::wizChangeContract_IntroPage(QWidget* parent) : QWizardPage(parent)
{
    setTitle("Ein- / Auszahlung");
    QRadioButton* rbDeposit = new QRadioButton("Einzahlung");
    QRadioButton* rbPayout = new QRadioButton("Auszahlung");
    registerField("deposit_notPayment", rbDeposit);
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(rbDeposit);
    layout->addWidget(rbPayout);
    setLayout(layout);
}

void wizChangeContract_IntroPage::initializePage()
{
    QString subtitle = "In dieser Dialogfolge kannst Du Ein- oder Auszahlungen zum Vertrag <b>%1</b> von <b>%2</b> verbuchen";
    wizChangeContract* wiz= dynamic_cast<wizChangeContract*>(wizard());
    subtitle = subtitle.arg(wiz->contractLabel).arg(wiz->creditorName);
    setSubTitle(subtitle);
}

bool wizChangeContract_IntroPage::validatePage()
{
    wizChangeContract* wiz= dynamic_cast<wizChangeContract*>(this->wizard());

    if( ! field("deposit_notPayment").toBool() && wiz->currentAmount < 601.) {
        QMessageBox::information(this, "Keine Auszahlung möglich",
            "Die kleinste Einlage beträgt 500 Euro. Die kleinste Auszahlung beträgt 100 Euro. "
            "Daher ist im Moment keine Auszahlung möglich.<p>Du kannst einen Einzahlung machen oder "
            "über den entsprechenden Menüpunkt den Vertrag beenden");
        return false;
    }
    return true;
}

wizChangeContract_AmountPage::wizChangeContract_AmountPage(QWidget* parent) : QWizardPage(parent)
{
    QVBoxLayout*  layout = new QVBoxLayout;
    QLineEdit* le = new QLineEdit;
    registerField("amount", le);
    layout->addWidget(le);
    setLayout(layout);
}

void wizChangeContract_AmountPage::initializePage()
{
    bool deposit = field("deposit_notPayment").toBool();
    if( deposit) {
        setTitle("Einzahlungsbetrag");
        setSubTitle("Gib den eingezahlten Betrag in ganzen Euro an. Der Betrag muß größer als 100 Euro sein.");
        setField("amount", 10000.);
    } else {
        setTitle("Auszahlungsbetrag");
        wizChangeContract* wiz= dynamic_cast<wizChangeContract*>(this->wizard());
        double currentAmount = wiz->currentAmount;
        // double minPayment = 100., minRemains = 500.;
        double maxPayout = currentAmount - 500;
        QLocale locale;
        QString subtitle ="Der Auszahlungsbetrag kann zwischen %1 und %2 liegen";
        subtitle = subtitle.arg(locale.toCurrencyString(100)).arg(locale.toCurrencyString(maxPayout));
        setSubTitle(subtitle);
        setField("amount", 100.);
    }
}

bool wizChangeContract_AmountPage::validatePage()
{
    bool deposit = field("deposit_notPayment").toBool();
    double amount = round2(field("amount").toDouble());
    if( amount < 100)
        return false;
    setField("amount", amount);

    if( ! deposit) {
        wizChangeContract* wiz= dynamic_cast<wizChangeContract*>(this->wizard());
        double currentAmount = wiz->currentAmount;
        double amount = field("amount").toDouble();
        // double minPayout = 100., minRemains = 500.
        if( amount <100 || (currentAmount-amount) <500)
            return false;
    }
    return true;
}

wizChangeContract_DatePage::wizChangeContract_DatePage(QWidget* parent) : QWizardPage(parent)
{
    QDateEdit* de = new QDateEdit;
    registerField("date", de);

    QVBoxLayout*  layout = new QVBoxLayout;
    layout->addWidget(de);
    setLayout(layout);
}

void wizChangeContract_DatePage::initializePage()
{
    wizChangeContract* wiz= dynamic_cast<wizChangeContract*>(this->wizard());
    QString subt=QString("Das Datum muß nach der letzten Buchung zu diesem Vertrag (%1) liegen. ").arg(wiz->earlierstDate.toString("dd.MM.yyyy"));

    bool deposit = field("deposit_notPayment").toBool();
    if( deposit) {
        setTitle("Datum des Geldeingangs");
        setSubTitle(subt + "<p>Gib das Datum an, an dem das Geld auf unserem Konto gutgeschrieben wurde.");
    } else {
        setTitle("Überweisungsdatum");
        setSubTitle(subt + "<p>Gib das Datum ein, zu dem die Überweisung durchgeführt wird.");
    }
    setField("date", wiz->earlierstDate);
}

bool wizChangeContract_DatePage::validatePage()
{
    wizChangeContract* wiz= dynamic_cast<wizChangeContract*>(this->wizard());
    if( field("date").toDate() < wiz->earlierstDate)
        return false;
    return true;
}

wizChangeContract_Summary::wizChangeContract_Summary(QWidget* p) : QWizardPage(p)
{
    setTitle("Zusammenfassung");
    QCheckBox* cb = new QCheckBox("Die Eingaben sind korrekt!");
    registerField("confirmed", cb);
    QVBoxLayout* layout = new QVBoxLayout;
    layout-> addWidget(cb);
    setLayout(layout);
}
void wizChangeContract_Summary::initializePage()
{
    wizChangeContract* wiz= dynamic_cast<wizChangeContract*>(this->wizard());

    QString subtitle ="zum Vertrag <b>%1</b><p>von <b>%2</b>:<p>"
                      "<table width=100%><tr><td align=center>Vorheriger Wert</td><td align=center>Änderungsbetrag</td><td align=center>neuer Wert</td></tr>"
                      "<tr><td align=center>%3</td><td align=center>%4%5</td><td align=center>%6</td></tr></table>"
                      "<p>Datum: %7</b>";
    bool deposit = field("deposit_notPayment").toBool();
    double oldValue = wiz->currentAmount, newValue =0;
    double change = field("amount").toDouble();
    if( deposit) {
        setTitle("Zusammenfassung der Einzahlung");
        subtitle = "Einzahlung " +subtitle;
        newValue = wiz->currentAmount + change;
    } else {
        setTitle("Zusammenfassung der Auszahlung");
        subtitle = "Auszahlung " +subtitle;
        newValue = wiz->currentAmount - change;
    }
    QLocale locale;
    setSubTitle(subtitle.arg(wiz->contractLabel)
                .arg(wiz->creditorName)
                .arg(locale.toCurrencyString(oldValue))
                .arg(deposit? "+" : "-")
                .arg(locale.toCurrencyString(change))
                .arg(locale.toCurrencyString(newValue))
                .arg(field("date").toDate().toString("dd.MM.yyyy")));
}

bool wizChangeContract_Summary::validatePage()
{
    return field("confirmed").toBool();
}

wizChangeContract::wizChangeContract(QWidget* p) : QWizard(p)
{
    addPage(new wizChangeContract_IntroPage);
    addPage(new wizChangeContract_AmountPage);
    addPage(new wizChangeContract_DatePage);
    addPage(new wizChangeContract_Summary);
}
