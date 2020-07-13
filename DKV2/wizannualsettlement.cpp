#include <QVariant>
#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>

#include "wizannualsettlement.h"

wizAnnualSettlement_IntroPage::wizAnnualSettlement_IntroPage(QWidget* p)  : QWizardPage(p)
{
    setTitle("Jahresabrechnung");
    setSubTitle("Die Abrechnung für das Folgende Jahr kann gemacht werden:");
    QLabel* l =new QLabel;
    registerField("year", l, "text");
    QCheckBox* csv = new QCheckBox("Buchungen als csv Datei ausgeben.");
    registerField("printCsv", csv);
    QCheckBox* confirm =new QCheckBox("Jahresabrechnung jetzt durchführen.");
    registerField("confirm", confirm);
    QVBoxLayout* bl = new QVBoxLayout;
    bl->addWidget(l);
    bl->addWidget(csv);
    bl->addWidget(confirm);
    setLayout(bl);
}

void wizAnnualSettlement_IntroPage::initializePage()
{
    setField("printCsv", true);
    setField("confirm", false);
}

bool wizAnnualSettlement_IntroPage::validatePage()
{
    return this->field("confirm").toBool();
}

wizAnnualSettlement::wizAnnualSettlement(QWidget* p) : QWizard(p)
{
    addPage(new wizAnnualSettlement_IntroPage);
}
