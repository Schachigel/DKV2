#include <iso646.h>

#include "pch.h"

#include "appconfig.h"
#include "helper.h"
#include "helperfin.h"
#include "wizactivatecontract.h"

wpInitialPayment_IntroPage::wpInitialPayment_IntroPage(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Aktivierung eines Vertrags"));
}

void wpInitialPayment_IntroPage::initializePage()
{
    wizInitialPayment* wiz = qobject_cast<wizInitialPayment*>(wizard());
    QString subtitle = qsl("Mit dieser Dialogfolge kannst Du den Geldeingang zu Vertrag <p><b>%1</b> von <b>%2</b> <p>verbuchen. "
                       "Wenn keine verzögerte Zinsanrechnung vereinbart wurde beginnt damit die Zinsanrechnung. Anderenfalls muss "
                       "das Datum für den Beginn der Zinsanrechnung nachträglich eingegeben werden.<br>");
    setSubTitle(subtitle.arg(wiz->label, wiz->creditorName));
}

QString fnDate {qsl("date")};
QString fnActivateNow {qsl("activateNow")};
wpInitialPayment_DatePage::wpInitialPayment_DatePage(QWidget* p) : QWizardPage(p)
{
    QDateEdit* de = new QDateEdit;
    de->setDisplayFormat(qsl("dd.MM.yyyy"));
    registerField(fnDate, de);
    QRadioButton* rbNow =new QRadioButton(qsl("Die Zinszahlung beginnt mit der Aktivierung"));
    QRadioButton* rbLater =new QRadioButton(qsl("Die Zinszahlung wird nachträglich begonnen"));
    rbNow->setChecked(true);
    registerField(fnActivateNow, rbNow);

    QVBoxLayout*  layout = new QVBoxLayout;
    layout->addWidget(de);
    layout->addWidget(rbNow);
    layout->addWidget(rbLater);
    setLayout(layout);
}

void wpInitialPayment_DatePage::initializePage()
{
    setTitle(qsl("Aktivierungsdatum"));
    setSubTitle(qsl("Das Aktivierungsdatum entspricht dem Datum, zu dem das Geld auf unserem Konto eingegangen ist"));
    wizInitialPayment* wiz = qobject_cast<wizInitialPayment*>(wizard());
    minDate = wiz->minimalActivationDate;
}

bool wpInitialPayment_DatePage::validatePage()
{
    if( field(fnDate).toDate() < minDate) {
        QMessageBox::information(this, qsl("Fehlerhaftes Datum"), qsl("Das Datum muss nach dem Vertragsdatum liegen"));
        setField(fnDate, minDate);
        return false;
    }
    return true;
}

QString fnAmount {qsl("amount")};
wpInitialPayment_AmountPage::wpInitialPayment_AmountPage(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Eingegangener Kreditbetrag"));
    setSubTitle(qsl("Gib die Summe ein, die von dem/der DK-Geber*in überwiesen wurde.<br> "
                "Diese entspricht normalerweise dem im Vertrag vereinbarten Kreditbetrag."));
    QVBoxLayout*  layout = new QVBoxLayout;
    QLabel* l = new QLabel(qsl("Betrag in Euro"));
    QLineEdit* le = new QLineEdit;
    registerField(fnAmount, le);
    le->setValidator(new QIntValidator(this));
    l->setBuddy(le);
    layout->addWidget(l);
    layout->addWidget(le);
    setLayout(layout);
}

bool wpInitialPayment_AmountPage::validatePage()
{
    wizInitialPayment* wiz = qobject_cast<wizInitialPayment*>(wizard());
    double amount = field(fnAmount).toDouble();
    if( amount < dbConfig::readValue(MIN_AMOUNT).toDouble())
        return false;
    setField(fnAmount, r2(amount));
    if( wiz->expectedAmount not_eq amount) {
        qInfo() << "activation with different amount";
    }
    return true;
}

wpInitialPayment_SummaryPage::wpInitialPayment_SummaryPage( QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Zusammenfassung"));
    QCheckBox* cb = new QCheckBox(qsl("Die Eingaben sind korrekt!"));
    registerField(qsl("confirmed"), cb);
    QVBoxLayout* layout = new QVBoxLayout;
    layout-> addWidget(cb);
    setLayout(layout);
    connect(cb, &QCheckBox::stateChanged, this, &wpInitialPayment_SummaryPage::onConfirmData_toggled);
}

void wpInitialPayment_SummaryPage::initializePage()
{
    QString subt =qsl("Der Vertrag <p><b>%1</b> von <b>%2</b><p> soll mit einem Betrag von <p>"
                  "<b>%3 </b><p> zum %4 aktiviert werden. <br>");
    wizInitialPayment* wiz = qobject_cast<wizInitialPayment*>(wizard());
    double amount = field(fnAmount).toDouble();
    QLocale locale;
    subt = subt.arg(wiz->label, wiz->creditorName, locale.toCurrencyString(amount), field(fnDate).toDate().toString(qsl("dd.MM.yyyy")));
    if( amount not_eq wiz->expectedAmount)
        subt += qsl(" <b><small>Der Überweisungsbetrag stimmt nicht mit dem Kreditbetrag überein.</small></b>");
    setSubTitle(subt);
}

bool wpInitialPayment_SummaryPage::validatePage()
{
    if( field(qsl("confirmed")).toBool())
        return true;
    return false;
}
void wpInitialPayment_SummaryPage::onConfirmData_toggled(int )
{
    emit completeChanged();
}

bool wpInitialPayment_SummaryPage::isComplete() const
{
    return field(qsl("confirmed")).toBool();
}

wizInitialPayment::wizInitialPayment(QWidget* p) : QWizard (p)
{
    QFont f = font(); f.setPointSize(10); setFont(f);
    addPage(new wpInitialPayment_IntroPage);
    addPage(new wpInitialPayment_DatePage);
    addPage(new wpInitialPayment_AmountPage);
    addPage(new wpInitialPayment_SummaryPage);
}
