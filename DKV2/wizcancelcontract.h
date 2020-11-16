#ifndef WIZCANCELCONTRACT_H
#define WIZCANCELCONTRACT_H

#include <QWizard>

#include "contract.h"

struct wizCancelContract_IntroPage : public QWizardPage {
    wizCancelContract_IntroPage (QWidget* w =nullptr);
    void initializePage() override;
    Q_OBJECT;
};

struct wizCancelContract_DatePage : public QWizardPage {
    wizCancelContract_DatePage(QWidget* =nullptr);
    void cleanupPage() override  {};
    void initializePage() override;
    bool validatePage() override;
    Q_OBJECT;
};

class wizCancelContract_SummaryPage : public QWizardPage {
    Q_OBJECT;
public:
    wizCancelContract_SummaryPage(QWidget* p =nullptr);
    void initializePage() override;
    bool isComplete() const override;
public slots:
    void onConfirmData_toggled(int);
};

struct wizCancelContract : public QWizard
{
    wizCancelContract(QWidget* w =nullptr);
    // data
    contract c;
    QString creditorName;
    QDate contractualEnd;
    Q_OBJECT;
};

#endif // WIZCANCELCONTRACT_H
