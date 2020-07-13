#ifndef WIZANNUALSETTLEMENT_H
#define WIZANNUALSETTLEMENT_H

#include <QDate>
#include <QWizard>

struct wizAnnualSettlement_IntroPage : public QWizardPage
{
    wizAnnualSettlement_IntroPage(QWidget* p =nullptr);
    void initializePage() override;
    bool validatePage() override;
};

struct wizAnnualSettlement : public QWizard
{
    wizAnnualSettlement(QWidget* p =nullptr);
    int year;
};

#endif // WIZANNUALSETTLEMENT_H
