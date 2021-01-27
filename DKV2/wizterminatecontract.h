#ifndef WIZTERMINATECONTRACT_H
#define WIZTERMINATECONTRACT_H

#include <QWizard>
#include <QDate>

#include "contract.h"

struct wpTerminateContract_DatePage : public QWizardPage
{
    wpTerminateContract_DatePage(QWidget* p=nullptr);
    void initializePage() override;
    bool validatePage() override;
    Q_OBJECT;
};

class wpTerminateContract_ConfirmationPage : public QWizardPage
{
    Q_OBJECT;
public:
    wpTerminateContract_ConfirmationPage(QWidget* p=nullptr);
    void initializePage() override;
    bool isComplete() const override;
public slots:
    void onConfirmData_toggled(int);
};

struct wizTerminateContract : public QWizard
{
    wizTerminateContract(QWidget* p, contract c);
    contract c;
    Q_OBJECT;
};

#endif // WIZTERMINATECONTRACT_H
