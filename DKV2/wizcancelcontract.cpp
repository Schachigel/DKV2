#include <QLabel>
#include <QDateEdit>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QMessageBox>

#include "wizcancelcontract.h"

wpCancelContract_IntroPage::wpCancelContract_IntroPage(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Vertragskündigung"));
}

void wpCancelContract_IntroPage::initializePage()
{
    wizCancelContract* wiz = qobject_cast<wizCancelContract*>(wizard());
    QString subTitle(qsl("Mit dieser Dialogfolge kannst Du den Vertrag <p><b>%1</b>"
                     "<p>von <p><b>%2</b><p> kündigen. "
                     "Dadurch wird das Vertragsende festgelegt, an dem die Auszahlung erfolgen wird.<p>"));
    subTitle = subTitle.arg(wiz->c.label(), wiz->creditorName);
    setSubTitle(subTitle);
}

wpCancelContract_DatePage::wpCancelContract_DatePage(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Kündigungsdatum"));

    QDateEdit* de = new QDateEdit;
    de->setDisplayFormat(qsl("dd.MM.yyyy"));
    registerField(qsl("date"), de);
    QVBoxLayout*  layout = new QVBoxLayout;
    layout->addWidget(de);
    setLayout(layout);
}

void wpCancelContract_DatePage::initializePage()
{
    wizCancelContract* wiz = qobject_cast<wizCancelContract*>(wizard());
    QString subTitle(qsl("Das durch die Kündigungsfrist vertraglich vereinbarte Vertragsende ist am <b>%1</b>.<br>"
                         "Die letzte Buchung zu dem Vertrag war am <b>%2<b>.<br>"
                     "<p>Gib das geplante Vertragsende ein."));
    QDate latestB = wiz->c.latestBooking().date;
    subTitle =subTitle.arg(wiz->contractualEnd.toString(qsl("dd.MM.yyyy")), latestB.toString(qsl("dd.MM.yyyy")));
    setSubTitle(subTitle);
    setField(qsl("date"), std::max(wiz->contractualEnd, latestB).addDays(1));
}

bool wpCancelContract_DatePage::validatePage()
{
    wizCancelContract* wiz = qobject_cast<wizCancelContract*>(wizard());
    QDate lastB =wiz->c.latestBooking().date;
    if( field(qsl("date")).toDate() > lastB)
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
    QCheckBox* cb = new QCheckBox(qsl("Die Eingaben sind korrekt!"));
    registerField(qsl("confirmed"), cb);
    QVBoxLayout* layout = new QVBoxLayout;
    layout-> addWidget(cb);
    setLayout(layout);
    connect(cb, &QCheckBox::stateChanged, this, &wpCancelContract_SummaryPage::onConfirmData_toggled);
}
void wpCancelContract_SummaryPage::initializePage()
{
    wizCancelContract* wiz = qobject_cast<wizCancelContract*>(wizard());
    QString subt =qsl("Der Vertrag <b>%1</b> <p>von <b>%2</b><p>soll zum <b>%3</b> beendet werden.");
    subt = subt.arg(wiz->c.label(), wiz->creditorName);
    subt = subt.arg(field(qsl("date")).toDate().toString(qsl("dd.MM.yyyy")));
    setSubTitle(subt);
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
    QFont f = font(); f.setPointSize(10); setFont(f);
    addPage(new wpCancelContract_IntroPage);
    addPage(new wpCancelContract_DatePage);
    addPage(new wpCancelContract_SummaryPage);
}
