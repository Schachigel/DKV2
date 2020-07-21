#ifndef WIZNEW_H
#define WIZNEW_H

#include <QStringLiteral>
#define qsl(x) QStringLiteral(x)
#include <QRadioButton>
#include <QComboBox>
#include <QWizard>

enum { page_new_or_existing, page_address, page_email, page_bankaccount,
     page_confirm_creditor, page_contract_data, page_contract_term, page_confirm_contract};

class wizNewOrExistingPage : public QWizardPage
{
    Q_OBJECT
public:
    wizNewOrExistingPage(QWidget* );
    ~wizNewOrExistingPage(){}
    void initializePage() override;
public slots:
    void onExistingCreditor_toggled(bool );

    bool validatePage() override;
    int nextId() const override;
private:
    QRadioButton* rbNew;
    QRadioButton* rbExisting;
    QComboBox*    cbCreditors;
};

struct wizNewCreditorAddress : public QWizardPage{
    wizNewCreditorAddress(QWidget* p);
    void initializePage() override;
};

struct wizNewContractData : public QWizardPage{
    wizNewContractData(QWidget* p);
};

struct wizNew : public QWizard
{
    wizNew(QWidget* p);
    qlonglong creditorId =0;
};

#endif // WIZNEW_H
