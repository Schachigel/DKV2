
#include <QLabel>
#include <QDateEdit>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QMessageBox>

#include "wizterminatecontract.h"

wpTerminateContract_DatePage::wpTerminateContract_DatePage(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Vertrag beenden"));
    QDateEdit* de = new QDateEdit;
    de->setDisplayFormat(qsl("dd.MM.yyyy"));
    registerField(qsl("date"), de);
    de->setToolTip(qsl("Rückerstattungsdatum"));
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(de);
    setLayout(layout);
}

void wpTerminateContract_DatePage::initializePage()
{
    wizTerminateContract* wiz = qobject_cast<wizTerminateContract*>(wizard());
    QString creditorName =executeSingleValueSql(qsl("Vorname || ' ' || Nachname"), qsl("Kreditoren"), qsl("id=") + QString::number(wiz->cont.creditorId())).toString();
    QString Kennung =wiz->cont.label();
    QString subt {qsl("Mit dieser Dialogfolge kannst Du den Vertrag <p><b>%1</b> von <b>%2</b><p> beenden.<p>"
                "Gib das Datum an, zu dem der Vertrag ausgezahlt wird. "
                "Bis zu diesem Datum werden die Zinsen berechnet. ")};
    setSubTitle(subt.arg(Kennung, creditorName));
    setField(qsl("date"), wiz->cont.plannedEndDate());
}

bool wpTerminateContract_DatePage::validatePage()
{
    wizTerminateContract* wiz = qobject_cast<wizTerminateContract*>(wizard());
    if( field(qsl("date")).toDate() >= wiz->cont.latestBooking().date)
        return true;
    QString msg (qsl("Das Vertragsende muss nach der letzten Buchung des Vertrags am %1 sein"));
    msg = msg.arg(wiz->cont.latestBooking().date.toString(qsl("dd.MM.yyyy")));
    QMessageBox::information(this, qsl("Ungültiges Datum"), msg);
    return false;
}

wpTerminateContract_ConfirmationPage::wpTerminateContract_ConfirmationPage(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Vertrag beenden"));
    QCheckBox* cbPrint = new QCheckBox(qsl("Beleg als CSV Datei speichern"));
    registerField(qsl("print"), cbPrint);
    cbPrint->setToolTip(qsl("Die Datei wird in dem konfigurierten Ausgabeordner gespeichert"));
    QCheckBox* cbConfirm = new QCheckBox(qsl("Die Engaben sind korrekt"));
    registerField(qsl("confirm"), cbConfirm);
    cbConfirm->setToolTip(qsl("Bestätige, dass die Eingaben richtig sind!"));
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(cbPrint);
    layout->addWidget(cbConfirm);
    setLayout(layout);
    connect(cbConfirm, &QCheckBox::stateChanged, this, &wpTerminateContract_ConfirmationPage::onConfirmData_toggled);
}

void wpTerminateContract_ConfirmationPage::initializePage()
{
    wizTerminateContract* wiz = qobject_cast<wizTerminateContract*>(wizard());
    double interest =0., finalValue =0.;
    wiz->cont.finalize(true, field(qsl("date")).toDate(), interest, finalValue);

    QString subtitle = qsl("<table width=100%>"
                       "<tr><td>Bewertung des Vertrags zum Laufzeitende</td><td align=right><b>%1</b> </td></tr>"
                       "<tr><td>Zinsen der letzten Zinsphase</td>            <td align=right><b>%2</b> </td></tr>"
                       "<tr><td>Auszahlungsbetrag </td>                      <td align=right><b>%3</b> </td></tr>"
                       "</table>");
    QLocale locale;
    subtitle = subtitle.arg(locale.toCurrencyString(wiz->cont.value()), locale.toCurrencyString(interest), locale.toCurrencyString(finalValue));
    setSubTitle(subtitle);
}
void wpTerminateContract_ConfirmationPage::onConfirmData_toggled(int)
{
    emit completeChanged();
}
bool wpTerminateContract_ConfirmationPage::isComplete() const
{
    return field(qsl("confirm")).toBool();
}

wizTerminateContract::wizTerminateContract(QWidget* p, contract& c) : QWizard(p), cont(c)
{
    addPage(new wpTerminateContract_DatePage);
    addPage(new wpTerminateContract_ConfirmationPage);
    QFont f = font(); f.setPointSize(10); setFont(f);
}
