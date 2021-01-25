#ifndef WIZNEW_H
#define WIZNEW_H

#include <QStringLiteral>
#include <QRadioButton>
#include <QDateEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QWizard>
#include "helper.h"

enum { page_new_or_existing, page_address, page_email, page_bankaccount,
     page_confirm_creditor, page_contract_data, page_contract_term, page_confirm_contract};

class wpNewOrExisting : public QWizardPage
{
    Q_OBJECT
public:
    wpNewOrExisting(QWidget* );
    ~wpNewOrExisting(){}
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

struct wpNewCreditorAddress : public QWizardPage{
    wpNewCreditorAddress(QWidget* p);
    void cleanupPage() override  {};
    bool validatePage()   override;
    int nextId() const    override;
};

struct wpEmail : public QWizardPage {
    wpEmail (QWidget* p);
    void cleanupPage() override  {};
    bool validatePage()   override;
    int nextId() const    override;
};

struct wpBankAccount : public QWizardPage{
    wpBankAccount(QWidget* p);
    void cleanupPage() override  {};
    bool validatePage()   override;
    int nextId() const    override;
};

class wpConfirmCreditor : public QWizardPage{
    Q_OBJECT
public:
    wpConfirmCreditor(QWidget* p);
    void initializePage() override;
    bool validatePage()   override;
    int nextId() const    override;
public slots:
    void onConfirmCreateContract_toggled(int state);
private:
    QCheckBox* cbCreateContract =nullptr;
};

struct wpNewContractData : public QWizardPage{
    wpNewContractData(QWidget* p);
    void initializePage() override;
    void cleanupPage() override  {};
    bool validatePage() override;
    int nextId() const    override;
private:
    bool init =true;
    QComboBox* cbInterest =nullptr;
};

class wpContractTiming : public QWizardPage{
    Q_OBJECT
public:
    wpContractTiming(QWidget*);
    void initializePage() override;
    void cleanupPage() override  {};
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

class wpContractConfirmation : public QWizardPage
{
    Q_OBJECT;
public:
    wpContractConfirmation(QWidget*);
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
