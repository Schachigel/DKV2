#include "pch.h"

#include "wizcancelcontract.h"

wpCancelContract_IntroPage::wpCancelContract_IntroPage(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Vertragskündigung"));
    subTitleLabel = new QLabel(qsl("Keine Daten!"));
    subTitleLabel->setWordWrap(true);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(subTitleLabel);
    setLayout(layout);
}

void wpCancelContract_IntroPage::initializePage()
{
    wizCancelContract* wiz = qobject_cast<wizCancelContract*>(wizard());
    QString subTitle(qsl("Mit dieser Dialogfolge kannst Du den Vertrag <p><b>%1</b>"
                     "<p>von <p><b>%2</b><p> kündigen. "
                     "Dadurch wird das Vertragsende festgelegt, an dem die Auszahlung erfolgen wird.<p>"));
    subTitle = subTitle.arg(wiz->c.label(), wiz->creditorName);
    subTitleLabel->setText(subTitle);
}

wpCancelContract_DatePage::wpCancelContract_DatePage(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Kündigungsdatum"));
    subTitleLabel = new QLabel(qsl("Keine Daten!"));
    subTitleLabel->setWordWrap(true);

    QLabel *lblVEnde =new QLabel(qsl("Datum, zu dem der Vertrag beendet wird"));
    QDateEdit *de = new QDateEdit;
    de->setDisplayFormat(qsl("dd.MM.yyyy"));
    registerField(qsl("date"), de);

    QLabel *lblK = new QLabel( qsl("Datum, zu dem die Kündigung ausgesprochen wurde"));
    QDateEdit *deK = new QDateEdit;
    deK->setDisplayFormat(qsl("dd.MM.yyyy"));
    registerField(qsl("KüDatum"), deK);

    QVBoxLayout*  layout = new QVBoxLayout;
    layout->addWidget(subTitleLabel);
    layout->addWidget (lblVEnde);
    layout->addWidget(de);
    layout->addWidget (new QLabel());
    layout->addWidget (lblK);
    layout->addWidget (deK);

    setLayout(layout);
}

void wpCancelContract_DatePage::initializePage()
{
    wizCancelContract* wiz = qobject_cast<wizCancelContract*>(wizard());
    QString subTitle(qsl("Das durch die Kündigungsfrist vertraglich vereinbarte Vertragsende ist am <b>%1</b>.<p>"
                         "Die letzte Buchung zu dem Vertrag war am <b>%2<b>.<p>"
                     "Gib das geplante Vertragsende ein."));
    QDate latestB = wiz->c.latestBooking().date;
    subTitle =subTitle.arg(wiz->contractualEnd.toString(qsl("dd.MM.yyyy")), latestB.toString(qsl("dd.MM.yyyy")));
    subTitleLabel->setText(subTitle);
    setField(qsl("date"), qMax(wiz->contractualEnd, latestB));
    setField(qsl("KüDatum"), QDate::currentDate ());
}

bool wpCancelContract_DatePage::validatePage()
{
    wizCancelContract* wiz = qobject_cast<wizCancelContract*>(wizard());
    QDate lastB =wiz->c.latestBooking().date;
    if( field(qsl("date")).toDate() >= lastB)
        return true;
    else {
        QString msg (qsl("Das Vertragsende muss nach der letzten Buchung des Vertrags am %1 sein"));
        msg = msg.arg(lastB.toString(qsl("dd.MM.yyyy")));
        QMessageBox::information(this, qsl("Ungültiges Datum"), msg);
        return false;
    }
    return true;
}

wpCancelContract_SummaryPage::wpCancelContract_SummaryPage(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Zusammenfassung"));
    subTitleLabel = new QLabel(qsl("Keine Daten!"));
    subTitleLabel->setWordWrap(true);

    QCheckBox *cb = new QCheckBox(qsl("Die Eingaben sind korrekt!"));
    registerField(qsl("confirmed"), cb);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(subTitleLabel);
    layout->addWidget(cb);
    setLayout(layout);
    connect(cb, &QCheckBox::checkStateChanged, this, &wpCancelContract_SummaryPage::onConfirmData_toggled);
}
void wpCancelContract_SummaryPage::initializePage()
{
    wizCancelContract* wiz = qobject_cast<wizCancelContract*>(wizard());
    QString subt =qsl("Der Vertrag <b>%1</b> <p>von <b>%2</b><p>soll zum <b>%3</b> beendet werden.\nDie Kündigung wurde am %4 ausgesprochen.");
    subt = subt.arg(wiz->c.label(), wiz->creditorName);
    subt = subt.arg(field(qsl("date")).toDate().toString(qsl("dd.MM.yyyy")));
    subt = subt.arg(field(qsl("KüDatum")).toDate().toString(qsl("dd.MM.yyyy")));
    subTitleLabel->setText(subt);
}
void wpCancelContract_SummaryPage::onConfirmData_toggled(int)
{
    emit completeChanged();
}
bool wpCancelContract_SummaryPage::isComplete() const
{
    return field(qsl("confirmed")).toBool();
}

wizCancelContract::wizCancelContract(QWidget* p) : QWizard(p)
{
    addPage(new wpCancelContract_IntroPage);
    addPage(new wpCancelContract_DatePage);
    addPage(new wpCancelContract_SummaryPage);
}
