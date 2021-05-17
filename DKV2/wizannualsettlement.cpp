
#include <QVariant>
#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>

#include "helper.h"
#include "wizannualsettlement.h"

wpAnnualSettlement_IntroPage::wpAnnualSettlement_IntroPage(QWidget* p)  : QWizardPage(p)
{
    setTitle(qsl("Jahresabrechnung"));
    setSubTitle(qsl("Die Abrechnung für das Folgende Jahr kann gemacht werden:"));
    QLabel* l =new QLabel;
    registerField(qsl("year"), l, "text");
    QCheckBox* csv = new QCheckBox(qsl("Buchungen als csv Datei ausgeben."));
    registerField(qsl("printCsv"), csv);
    QCheckBox* confirm =new QCheckBox(qsl("Jahresabrechnung jetzt durchführen."));
    registerField(qsl("confirm"), confirm);
    QVBoxLayout* bl = new QVBoxLayout;
    bl->addWidget(l);
    bl->addWidget(csv);
    bl->addWidget(confirm);
    setLayout(bl);
    connect(confirm, &QCheckBox::stateChanged, this, &wpAnnualSettlement_IntroPage::onConfirmData_toggled);
}

void wpAnnualSettlement_IntroPage::initializePage()
{
    setField(qsl("printCsv"), true);
    setField(qsl("confirm"), false);
}
bool wpAnnualSettlement_IntroPage::isComplete() const
{
    return field(qsl("confirm")).toBool();
}
void wpAnnualSettlement_IntroPage::onConfirmData_toggled(int)
{
    emit completeChanged();
}

wizAnnualSettlement::wizAnnualSettlement(QWidget* p) : QWizard(p)
{
    addPage(new wpAnnualSettlement_IntroPage);
    QFont f = font(); f.setPointSize(10); setFont(f);

}
