#ifndef WIZTERMINATECONTRACT_H
#define WIZTERMINATECONTRACT_H

#include <QWizard>
#include <QDate>

#include "contract.h"

struct wizTerminateContract_DatePage : QWizardPage
{
    wizTerminateContract_DatePage(QWidget* p=nullptr);
    void initializePage() override;
    bool validatePage() override;
};

struct wizTerminateContract_ConfirmationPage : QWizardPage{
    wizTerminateContract_ConfirmationPage(QWidget* p=nullptr);
    void initializePage() override;
    bool validatePage() override;
};

struct wizTerminateContract : QWizard
{
    wizTerminateContract(QWidget* p, const contract& c);
    const contract& c;
};

#endif // WIZTERMINATECONTRACT_H
