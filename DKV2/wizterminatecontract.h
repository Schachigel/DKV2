#ifndef WIZTERMINATECONTRACT_H
#define WIZTERMINATECONTRACT_H



#include "contract.h"

struct wpTerminateContract_DatePage : public QWizardPage
{
    wpTerminateContract_DatePage(QWidget* p=nullptr);
    void initializePage() override;
    bool validatePage() override;
    Q_OBJECT;
private:
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
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

private:
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};

struct wizTerminateContract : public QWizard
{
    wizTerminateContract(QWidget* p, contract& c);
    contract& cont;
    Q_OBJECT;

private:
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};

#endif // WIZTERMINATECONTRACT_H
