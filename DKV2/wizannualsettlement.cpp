
#include <QVariant>
#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>

#include "helper.h"
#include "wizannualsettlement.h"

wizAnnualSettlement_IntroPage::wizAnnualSettlement_IntroPage(QWidget* p)  : QWizardPage(p)
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
    connect(confirm, SIGNAL(stateChanged(int)), this, SLOT(onConfirmData_toggled(int)));
}

void wizAnnualSettlement_IntroPage::initializePage()
{
    setField(qsl("printCsv"), true);
    setField(qsl("confirm"), false);
}
bool wizAnnualSettlement_IntroPage::isComplete() const
{
    return field("confirm").toBool();
}
void wizAnnualSettlement_IntroPage::onConfirmData_toggled(int)
{
    completeChanged();
}

wizAnnualSettlement::wizAnnualSettlement(QWidget* p) : QWizard(p)
{
    addPage(new wizAnnualSettlement_IntroPage);
}
