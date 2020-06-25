#ifndef WIZCANCELCONTRACT_H
#define WIZCANCELCONTRACT_H

#include <QWizard>

#include "contract.h"

struct wizCancelContract_IntroPage : public QWizardPage {
    wizCancelContract_IntroPage (QWidget* w =nullptr);
    void initializePage() override;
};

struct wizCancelContract_DatePage : public QWizardPage {
    wizCancelContract_DatePage(QWidget* =nullptr);
    void initializePage() override;
    bool validatePage() override;
};

struct wizCancelContract_SummaryPage : public QWizardPage {
    wizCancelContract_SummaryPage(QWidget* p =nullptr);
    void initializePage() override;
    bool validatePage() override;
};

struct wizCancelContract : public QWizard
{
    wizCancelContract(QWidget* w =nullptr);
    // data
    contract c;
    QString creditorName;
    QDate contractualEnd;
};

#endif // WIZCANCELCONTRACT_H
