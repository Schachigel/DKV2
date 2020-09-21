#ifndef WIZNEW_H
#define WIZNEW_H

#include <QStringLiteral>
#include <QRadioButton>
#include <QDateEdit>
#include <QComboBox>
#include <QWizard>

#include "helper.h"

enum { page_new_or_existing, page_address, page_email, page_bankaccount,
     page_confirm_creditor, page_contract_data, page_contract_term, page_confirm_contract};

class wizNewOrExistingPage : public QWizardPage
{
    Q_OBJECT
public:
    wizNewOrExistingPage(QWidget* );
    ~wizNewOrExistingPage(){}
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;
public slots:
    void onExistingCreditor_toggled(bool );

private:
    bool init =true;
    QRadioButton* rbNew;
    QRadioButton* rbExisting;
    QComboBox*    cbCreditors;
};

struct wizNewCreditorAddressPage : public QWizardPage{
    wizNewCreditorAddressPage(QWidget* p);
    bool validatePage()   override;
    int nextId() const    override;
};

struct wizEmailPage : public QWizardPage {
    wizEmailPage (QWidget* p);
    bool validatePage()   override;
    int nextId() const    override;
};

struct wizBankAccountPage : public QWizardPage{
    wizBankAccountPage(QWidget* p);
    bool validatePage()   override;
    int nextId() const    override;
};

class wizConfirmCreditorPage : public QWizardPage{
    Q_OBJECT
public:
    wizConfirmCreditorPage(QWidget* p);
    void initializePage() override;
    bool validatePage()   override;
    int nextId() const    override;
public slots:
    void onConfirmCreateContract_toggled(int state);
};

struct wizNewContractDataPage : public QWizardPage{
    wizNewContractDataPage(QWidget* p);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const    override;
private:
    bool init =true;
    QComboBox* cbInterest =nullptr;
};

class wizContractTimingPage : public QWizardPage{
    Q_OBJECT
public:
    wizContractTimingPage(QWidget*);
    void initializePage() override;
    bool validatePage()   override;
    int nextId() const    override;
public slots:
    void onNoticePeriod_currentIndexChanged(int i);

private:
    bool init=true;
    QComboBox* cbNoticePeriod;
    QDateEdit* deTerminationDate;
    QDateEdit* deCDate;
};

class wizContractConfirmationPage : public QWizardPage
{
    Q_OBJECT;
public:
    wizContractConfirmationPage(QWidget*);
    void initializePage() override;
    bool validatePage()   override;
    bool isComplete() const override;
public slots:
    void onConfirmData_toggled(int );
};

struct wizNew : public QWizard
{
    wizNew(QWidget* p =nullptr);
    qlonglong creditorId =-1;
    double interest =0.;
    QDate date =QDate::currentDate();
    int noticePeriod =-1;
    QDate termination =EndOfTheFuckingWorld;
    Q_OBJECT;
};

struct wizEditCreditor : public QWizard
{
    wizEditCreditor(QWidget* p =nullptr);
    qlonglong creditorId =-1;
    double interest =0.;
    QDate date =QDate::currentDate();
    int noticePeriod =-1;
    QDate termination =EndOfTheFuckingWorld;
    Q_OBJECT;
};

#endif // WIZNEW_H
