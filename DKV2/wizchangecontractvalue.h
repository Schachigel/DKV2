#ifndef CHANGECONTRACTVALUEWIZ_H
#define CHANGECONTRACTVALUEWIZ_H


#include <QDate>
#include <QWizard>
#include <QLabel>
#include "helper.h"

const QString fnDeposit_notPayment {qsl("deposit_notPayment")};

class wpChangeContract_IntroPage : public QWizardPage
{
public:
    wpChangeContract_IntroPage(QWidget* parent =nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;
    Q_OBJECT;

private:
    QLabel *subTitleLabel = nullptr;
};

struct wpChangeContract_AmountPage : public QWizardPage
{
    wpChangeContract_AmountPage(QWidget* parent =nullptr);
    void cleanupPage() override  {};
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;
    Q_OBJECT;

private:
    QLabel *subTitleLabel = nullptr;
};

struct wpChangeContract_DatePage : public QWizardPage
{
    wpChangeContract_DatePage(QWidget* parent =nullptr);
    void cleanupPage() override  {};
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;
    Q_OBJECT;

private:
    QLabel *subTitleLabel = nullptr;
};

const QString fnPayoutInterest {qsl("payout_interest")};

struct wpChangeContract_PayoutInterestPage : public QWizardPage
{
    wpChangeContract_PayoutInterestPage(QWidget* parent =nullptr);
//    void cleanupPage() override  {};
//    void initializePage() override;
//    bool validatePage() override;
    int nextId() const override;
    Q_OBJECT;
};

class wpChangeContract_Summary : public QWizardPage{
    Q_OBJECT;
public:
    wpChangeContract_Summary(QWidget* p =nullptr);
    void initializePage() override;
    bool isComplete() const override;
public slots:
    void onConfirmData_toggled(int);

private:
    QLabel *subTitleLabel = nullptr;
};

struct wizChangeContract : public QWizard
{
    wizChangeContract(QWidget* p =nullptr);
    QString creditorName;
    QString contractLabel;
    double  currentAmount = 0.;
    QDate   earlierstDate;
    bool interestPayoutPossible =false;
    Q_OBJECT;
};

#endif // CHANGECONTRACTVALUEWIZ_H
