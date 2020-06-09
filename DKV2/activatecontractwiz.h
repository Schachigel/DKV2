#ifndef ACTIVATECONTRACTWIZ_H
#define ACTIVATECONTRACTWIZ_H

#include <QDate>
#include <QWizard>

struct wizActivateContract_IntroPage : public QWizardPage {
    wizActivateContract_IntroPage(QWidget* w =nullptr);
    void initializePage() override;
};

struct wizActiateContract_DatePage : public QWizardPage {
    wizActiateContract_DatePage(QWidget* w=nullptr);
    void initializePage() override;
//    bool validatePage() override;
};

struct wizActiateContract_AmountPage : public QWizardPage {
    wizActiateContract_AmountPage(QWidget* w=nullptr);
//    void initializePage() override;
    bool validatePage() override;
};

struct wizActivateContract_SummaryPage : public QWizardPage {
    wizActivateContract_SummaryPage(QWidget* w=nullptr);
    void initializePage() override;
    bool validatePage() override;
};


struct activateContractWiz : public QWizard
{
    activateContractWiz(QWidget* p =nullptr);
    QString label;
    QString creditorName;
    double expectedAmount;
};

#endif // ACTIVATECONTRACTWIZ_H
