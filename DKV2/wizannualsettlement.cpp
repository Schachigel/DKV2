#include <QLabel>
#include <QVBoxLayout>

#include "wizannualsettlement.h"

wizAnnualSettlement_IntroPage::wizAnnualSettlement_IntroPage(QWidget* p)  : QWizardPage(p)
{
    setTitle("Jahresabrechnung");
    setSubTitle("Die Abrechnung für das Folgende Jahr kann gemacht werden:");
    QLabel* l = new QLabel;
    registerField("year", l, "text");
    QVBoxLayout* bl = new QVBoxLayout;
    bl->addWidget(l);
    setLayout(bl);
}

void wizAnnualSettlement_IntroPage::initializePage()
{

}

wizAnnualSettlement::wizAnnualSettlement(QWidget* p) : QWizard(p)
{
    addPage(new wizAnnualSettlement_IntroPage);

}
