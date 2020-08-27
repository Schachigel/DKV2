#include <QLabel>
#include <QDateEdit>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QMessageBox>

#include "wizcancelcontract.h"

wizCancelContract_IntroPage::wizCancelContract_IntroPage(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Vertragskündigung"));
}

void wizCancelContract_IntroPage::initializePage()
{
    wizCancelContract* wiz = qobject_cast<wizCancelContract*>(wizard());
    QString subTitle(qsl("Mit dieser Dialogfolge kannst Du den Vertrag <p><b>%1</b>"
                     "<p>von <p><b>%2</b><p> kündigen. "
                     "Dadurch wird das Vertragsende festgelegt, an dem die Auszahlung erfolgen wird.<p>"));
    subTitle = subTitle.arg(wiz->c.label(), wiz->creditorName);
    setSubTitle(subTitle);
}

wizCancelContract_DatePage::wizCancelContract_DatePage(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Kündigungsdatum"));

    QDateEdit* de = new QDateEdit;
    de->setDisplayFormat(qsl("dd.MM.yyyy"));
    registerField(qsl("date"), de);
    QVBoxLayout*  layout = new QVBoxLayout;
    layout->addWidget(de);
    setLayout(layout);
}

void wizCancelContract_DatePage::initializePage()
{
    wizCancelContract* wiz = qobject_cast<wizCancelContract*>(wizard());
    QString subTitle(qsl("Das durch die Kündigungsfrist vertraglich vereinbarte Vertragsende ist am <b>%1</b>.<br>"
                         "Die letzte Buchung zu dem Vertrag war am <b>%2<b>.<br>"
                     "<p>Gib das geplante Vertragsende ein."));
    QDate latestB = wiz->c.latestBooking().date;
    subTitle =subTitle.arg(wiz->contractualEnd.toString("dd.MM.yyyy"), latestB.toString("dd.MM.yyyy"));
    setSubTitle(subTitle);
    setField(qsl("date"), std::max(wiz->contractualEnd, latestB).addDays(1));
}

bool wizCancelContract_DatePage::validatePage()
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

wizCancelContract_SummaryPage::wizCancelContract_SummaryPage(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Zusammenfassung"));
    QCheckBox* cb = new QCheckBox(qsl("Die Eingaben sind korrekt!"));
    registerField(qsl("confirmed"), cb);
    QVBoxLayout* layout = new QVBoxLayout;
    layout-> addWidget(cb);
    setLayout(layout);
    connect(cb, SIGNAL(stateChanged(int)), this, SLOT(onConfirmData_toggled(int)));
}
void wizCancelContract_SummaryPage::initializePage()
{
    wizCancelContract* wiz = qobject_cast<wizCancelContract*>(wizard());
    QString subt =qsl("Der Vertrag <b>%1</b> <p>von <b>%2</b><p>soll zum <b>%3</b> beendet werden.");
    subt = subt.arg(wiz->c.label(), wiz->creditorName);
    subt = subt.arg(field(qsl("date")).toDate().toString(qsl("dd.MM.yyyy")));
    setSubTitle(subt);
}
void wizCancelContract_SummaryPage::onConfirmData_toggled(int)
{
    completeChanged();
}
bool wizCancelContract_SummaryPage::isComplete() const
{
    return field("confirmed").toBool();
}

wizCancelContract::wizCancelContract(QWidget* p) : QWizard(p)
{
    addPage(new wizCancelContract_IntroPage);
    addPage(new wizCancelContract_DatePage);
    addPage(new wizCancelContract_SummaryPage);
}
