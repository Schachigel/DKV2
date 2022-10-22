#ifndef WIZCANCELCONTRACT_H
#define WIZCANCELCONTRACT_H



#include "contract.h"

struct wpCancelContract_IntroPage : public QWizardPage {
    wpCancelContract_IntroPage (QWidget* w =nullptr);
    void initializePage() override;
    Q_OBJECT;

private:
    QLabel *subTitleLabel = nullptr;
};

struct wpCancelContract_DatePage : public QWizardPage {
    wpCancelContract_DatePage(QWidget* =nullptr);
    void cleanupPage() override  {};
    void initializePage() override;
    bool validatePage() override;
    Q_OBJECT;

private:
    QLabel *subTitleLabel = nullptr;
};

struct wpCancelContract_SummaryPage : public QWizardPage {
    wpCancelContract_SummaryPage(QWidget* p =nullptr);
    void initializePage() override;
    bool isComplete() const override;
public slots:
    void onConfirmData_toggled(int);
private:
    Q_OBJECT;
    QLabel *subTitleLabel = nullptr;
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
