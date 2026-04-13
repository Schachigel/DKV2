#ifndef CHANGECONTRACTVALUEWIZ_H
#define CHANGECONTRACTVALUEWIZ_H

#include "uihelper.h"

struct contract;

const QString fnDeposit_notPayment {qsl("deposit_notPayment")};
const QString fnDeferredMidYearInterest {qsl("deferred_mid_year_interest")};

class wpChangeContract_IntroPage : public QWizardPage
{
public:
    wpChangeContract_IntroPage(QWidget* parent =nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;
    Q_OBJECT

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
    Q_OBJECT

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
    Q_OBJECT

private:
    QLabel *subTitleLabel = nullptr;
    QLabel *modeInfoLabel = nullptr;
    QCheckBox *deferredInterestCheckBox = nullptr;
};

const QString fnPayoutInterest {qsl("payout_interest")};

struct wpChangeContract_PayoutInterestPage : public QWizardPage
{
    wpChangeContract_PayoutInterestPage(QWidget* parent =nullptr);
//    void cleanupPage() override  {};
//    void initializePage() override;
//    bool validatePage() override;
    int nextId() const override;
    Q_OBJECT
};

class wpChangeContract_Summary : public QWizardPage{
    Q_OBJECT
public:
    wpChangeContract_Summary(QWidget* p =nullptr);
    void initializePage() override;
    bool isComplete() const override;
public slots:
    void onConfirmData_toggled(Qt::CheckState);

private:
    QLabel *subTitleLabel = nullptr;
};

struct wizChangeContract : public QWizard
{
    wizChangeContract(QWidget* p =nullptr);
    contract* cont = nullptr;
    QString creditorName;
    QString contractLabel;
    double  currentAmount = 0.;
    QDate   earlierstDate;
    bool interestPayoutPossible =false;
    Q_OBJECT
};

#endif // CHANGECONTRACTVALUEWIZ_H
