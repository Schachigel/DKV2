#include "pch.h"

#include "helper.h"
#include "helperfin.h"
#include "appconfig.h"

#include "wizchangecontractvalue.h"

enum change_contract_pages {
    intro_page,
    amount_page,
    date_page,
    payout_page,
    summary_page
};

wpChangeContract_IntroPage::wpChangeContract_IntroPage(QWidget* parent) : QWizardPage(parent)
{
    setTitle(qsl("Ein- / Auszahlung"));
    subTitleLabel = new QLabel(qsl("In dieser Dialogfolge kannst Du Ein- oder Auszahlungen zum Vertrag <br><b>DK-PPP-yyyy-nnnnnn</b> <br>von <b>Vorname Nachname</b> verbuchen."));

    QRadioButton *rbDeposit = new QRadioButton(qsl("Einzahlung"));
    QRadioButton* rbPayout = new QRadioButton(qsl("Auszahlung"));
    registerField(fnDeposit_notPayment, rbDeposit);
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(subTitleLabel);
    layout->addWidget(rbDeposit);
    layout->addWidget(rbPayout);
    setLayout(layout);
}

void wpChangeContract_IntroPage::initializePage()
{
    QString subtitle =qsl("In dieser Dialogfolge kannst Du Ein- oder Auszahlungen zum Vertrag <br><b>%1</b> <br>von <b>%2</b> verbuchen.");
    wizChangeContract* wiz= qobject_cast<wizChangeContract*>(wizard());
    subtitle = subtitle.arg(wiz->contractLabel, wiz->creditorName);
    subTitleLabel->setText(subtitle);
}

bool wpChangeContract_IntroPage::validatePage()
{
    wizChangeContract* wiz= qobject_cast<wizChangeContract*>(this->wizard());

    // is there enough money in the contract to do a payout?
    double minContract =dbConfig::readValue(MIN_AMOUNT).toDouble();
    double minPayout   =dbConfig::readValue(MIN_PAYOUT).toDouble();
    double minAmountToMakeA_Payout = minContract + minPayout +1;
    if( not field(fnDeposit_notPayment).toBool() and wiz->currentAmount < minAmountToMakeA_Payout) {
        QString msg(qsl("Die kleinste Einlage beträgt %1. Die kleinste Auszahlung beträgt %2. "
                    "Daher ist im Moment keine Auszahlung möglich.<p>Du kannst einen Einzahlung machen oder "
                    "über den entsprechenden Menüpunkt den Vertrag beenden"));
        msg = msg.arg(s_d2euro(minContract), s_d2euro(minPayout));
        QMessageBox::information(this, qsl("Keine Auszahlung möglich"), msg);
        return false;
    }
    return true;
}

int wpChangeContract_IntroPage::nextId() const
{
    return amount_page;
}

///////////////////////////////////////////
/// wpChangeContract_AmountPage
///////////////////////////////////////////

wpChangeContract_AmountPage::wpChangeContract_AmountPage(QWidget* parent) : QWizardPage(parent)
{
    subTitleLabel = new QLabel("Keine Daten!");
    subTitleLabel->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    QLineEdit* le = new QLineEdit;

    QDoubleValidator* dv =new QDoubleValidator(0., 100000000., 2, le);
    le->setValidator(dv);
    registerField(qsl("amount"), le);

    layout->addWidget(subTitleLabel);
    layout->addWidget(le);
    setLayout(layout);
}

void wpChangeContract_AmountPage::initializePage()
{
    bool isDeposit = field(fnDeposit_notPayment).toBool();
    double minPayout =dbConfig::readValue(MIN_PAYOUT).toDouble();
    double minAmount =dbConfig::readValue(MIN_AMOUNT).toDouble();
    QLocale l;
    if( isDeposit) {
        setTitle(qsl("Einzahlungsbetrag"));
        QString subt =qsl("Der Betrag muss größer als %1 sein.");
        subt =subt.arg(s_d2euro(minPayout));
        subTitleLabel->setText(subt);
        setField(qsl("amount"), l.toString (1000.));
    } else {
        setTitle(qsl("Auszahlungsbetrag"));
        wizChangeContract* wiz= qobject_cast<wizChangeContract*>(this->wizard());
        double currentAmount = wiz->currentAmount;
        // double minPayment = 100., minRemains = 500.;
        double maxPayout = currentAmount - minAmount;
        QString subtitle =qsl("Der Auszahlungsbetrag kann zwischen %1 und %2 liegen.");
        subtitle = subtitle.arg(s_d2euro(minPayout), s_d2euro(maxPayout));
        subTitleLabel->setText(subtitle);
        setField(qsl("amount"), l.toString(minPayout));
    }
}

bool wpChangeContract_AmountPage::validatePage()
{
    QLocale l;
    bool isDeposit = field(fnDeposit_notPayment).toBool();
    // cave! QLocale l ist notwendig, damit Werte mit Dezimalkomma (d) richtig "verstanden" werden
    double amount = r2(l.toDouble(field(qsl("amount")).toString()));

    if( isDeposit) {
        if( amount <= 0)
            return false;
        else
            return true;
    } else {
        wizChangeContract* wiz= qobject_cast<wizChangeContract*>(this->wizard());
        double currentAmount = wiz->currentAmount;
        double minPayout =dbConfig::readValue(MIN_PAYOUT).toDouble();
        double minAmount =dbConfig::readValue(MIN_AMOUNT).toDouble();
        if( amount < minPayout) {
            QMessageBox::information(this, qsl("Auszahlung nicht möglich"), qsl("Der Auszahlungsbetrag muss mindestens %1 betragen").arg(s_d2euro(minPayout)));
            return false;
        }
        if( currentAmount-amount < minAmount) {
            QMessageBox::information(this, qsl("Auszahlung nicht möglich"), qsl("Der Restbetrag muss mindestens %1 betragen").arg(s_d2euro(minAmount)));
            return false;
        }
    }
    return true;
}

int wpChangeContract_AmountPage::nextId() const
{
    return date_page;
}

wpChangeContract_DatePage::wpChangeContract_DatePage(QWidget* parent) : QWizardPage(parent)
{
    subTitleLabel = new QLabel(qsl("Keine Daten!"));
    subTitleLabel->setWordWrap(true);

    QDateEdit *de = new QDateEdit;
    de->setDisplayFormat(qsl("dd.MM.yyyy"));
    registerField(qsl("date"), de);

    QVBoxLayout*  layout = new QVBoxLayout;
    layout->addWidget(subTitleLabel);
    layout->addWidget(de);
    setLayout(layout);
}

void wpChangeContract_DatePage::initializePage()
{
    wizChangeContract* wiz= qobject_cast<wizChangeContract*>(this->wizard());
    QString subt=QString(qsl("Das Datum muss nach der letzten Buchung zu diesem Vertrag (%1) liegen. ")).arg(wiz->earlierstDate.toString(qsl("dd.MM.yyyy")));

    bool deposit = field(fnDeposit_notPayment).toBool();
    if( deposit) {
        setTitle(qsl("Datum des Geldeingangs"));
        subTitleLabel->setText(subt + qsl("<p>Gib das Datum an, an dem das Geld auf unserem Konto gutgeschrieben wurde."));
    } else {
        setTitle(qsl("Überweisungsdatum"));
        subTitleLabel->setText(subt + qsl("<p>Gib das Datum ein, zu dem die Überweisung durchgeführt wird."));
    }
    setField(qsl("date"), wiz->earlierstDate);
}

bool wpChangeContract_DatePage::validatePage()
{
    wizChangeContract* wiz= qobject_cast<wizChangeContract*>(this->wizard());
    QDate d {field(qsl("date")).toDate()};
    QString msg;

    if( d < wiz->earlierstDate)
        msg =qsl("Das Vertragsdatum muss nach der letzten Buchung liegen");
    if( d.month() == 12 and d.day() == 31)
        // avoid interest bookings on the date of anual settlements.
        // it is a holiday anyways
        msg += qsl("Eine Ein- oder Auszahlung darf nicht am 31.12. sein");
    if( not msg.isEmpty ()){
        QMessageBox::information (this, "Info", msg);
        return false;
    }
    return true;
}

wpChangeContract_Summary::wpChangeContract_Summary(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Zusammenfassung"));
    subTitleLabel = new QLabel("Keine Daten!");
    subTitleLabel->setWordWrap(true);

    QCheckBox* cb = new QCheckBox(qsl("Die Eingaben sind korrekt!"));
    registerField(qsl("confirmed"), cb);
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(subTitleLabel);
    layout->addWidget(cb);
    setLayout(layout);
    // TODO Change to checkStateChanged once Qt 6.9 is available on all targets.
    // https://doc.qt.io/qt-6/qcheckbox-obsolete.html
    connect(cb, &QCheckBox::stateChanged, this, &wpChangeContract_Summary::onConfirmData_toggled);
}

int wpChangeContract_DatePage::nextId() const
{
    bool askForPayout =qobject_cast<wizChangeContract*>(wizard())->interestPayoutPossible;
    if( askForPayout)
        return payout_page;
    else
        return summary_page;
}

wpChangeContract_PayoutInterestPage::wpChangeContract_PayoutInterestPage(QWidget* p) : QWizardPage (p)
{
    QLabel* subTitleLabel =new QLabel(qsl("Sollen die Zinsen als ausgezahlt gebucht werden?"));
    subTitleLabel->setWordWrap (true);

    QCheckBox* chk =new QCheckBox(qsl("Zinsen auszahlen"));
    chk->setChecked (false);
    registerField (fnPayoutInterest, chk);

    QVBoxLayout* l =new QVBoxLayout;
    l->addWidget (subTitleLabel);
    l->addWidget (chk);

    setLayout (l);
}

int wpChangeContract_PayoutInterestPage::nextId() const
{
    return summary_page;
}

void wpChangeContract_Summary::initializePage()
{
    wizChangeContract* wiz= qobject_cast<wizChangeContract*>(this->wizard());

    QString subtitle =qsl("zum Vertrag <b>%1</b><p>von <b>%2</b>:<p>"
                      "<table width=100%><tr><td align=center>Vorheriger Wert</td><td align=center>Änderungsbetrag</td><td align=center>neuer Wert</td></tr>"
                      "<tr><td align=center>%3</td><td align=center>%4%5</td><td align=center>%6</td></tr></table>"
                      "<p>Datum: %7</b>");
    if( field (fnPayoutInterest).toBool ()) {
        subtitle.append (qsl("<br>Die Zinsen werden <b>ausgezahlt.</b>"));
    }
    bool deposit = field(fnDeposit_notPayment).toBool();
    double oldValue = wiz->currentAmount, newValue =0;
    double change = QLocale().toDouble(field(qsl("amount")).toString());
    if( deposit) {
        setTitle(qsl("Zusammenfassung der Einzahlung"));
        subtitle = qsl("Einzahlung ") +subtitle;
        newValue = wiz->currentAmount + change;
    } else {
        setTitle(qsl("Zusammenfassung der Auszahlung"));
        subtitle = qsl("Auszahlung ") +subtitle;
        newValue = wiz->currentAmount - change;
    }
    subTitleLabel->setText(subtitle.arg(wiz->contractLabel, wiz->creditorName, s_d2euro(oldValue),
                   deposit? qsl("+") : qsl("-"), s_d2euro(change),
                   s_d2euro(newValue), field(qsl("date")).toDate().toString(qsl("dd.MM.yyyy"))));
}
bool wpChangeContract_Summary::isComplete() const
{
    return field(qsl("confirmed")).toBool();
}
void wpChangeContract_Summary::onConfirmData_toggled(int)
{
    emit completeChanged();
}


wizChangeContract::wizChangeContract(QWidget* p) : QWizard(p)
{   LOG_CALL;
    setWizardStyle(QWizard::ModernStyle);
    setPage(intro_page, new wpChangeContract_IntroPage);
    setPage(amount_page, new wpChangeContract_AmountPage);
    setPage(date_page, new wpChangeContract_DatePage);
    setPage(payout_page, new wpChangeContract_PayoutInterestPage);
    setPage(summary_page, new wpChangeContract_Summary);
}
