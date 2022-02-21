#ifndef WIZNEW_H
#define WIZNEW_H

#include <QStringLiteral>
#include <QRadioButton>
#include <QDateEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QWizard>
#include "contract.h"
#include "helper.h"

enum { page_new_or_existing,
       page_address,
       page_email,
       page_bankaccount,
       page_confirm_creditor,
       // contract related pages:
       page_label_and_amount,
       page_contract_timeframe,
       page_interest_selection_Mode,
       page_interest_from_investment,
       page_interest_value_selection,
       page_interest_payment_mode,
       page_confirm_contract};

extern const QString pnNew;
extern const QString pnCreditor;
/*
 * wpNewOrExisting - ask new or use existing
 */
class wpNewOrExisting : public QWizardPage
{
    Q_OBJECT
public:
    wpNewOrExisting(QWidget* );
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
    QLabel*       subTitleLabel = new QLabel(qsl("Keine Daten!"));
};

/*
 * wpNewCreditorAddress -
 */
extern const QString pnFName;
extern const QString pnLName;
extern const QString pnStreet;
extern const QString pnCity;
extern const QString pnPcode;
extern const QString pnCountry;

struct wpNewCreditorAddress : public QWizardPage{
    wpNewCreditorAddress(QWidget* p);
    void cleanupPage() override {};
    bool validatePage()   override;
    int nextId() const    override;
private:
    Q_OBJECT;
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};

extern const QString pnEMail;
extern const QString pnPhone;
extern const QString pnContact;
extern const QString pnComment;

struct wpEmail : public QWizardPage {
    wpEmail (QWidget* p);
    void cleanupPage() override  {}; // do not change data on back
    bool validatePage()   override;
    int nextId() const    override;
private:
    Q_OBJECT;
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};

extern const QString pnIban;
extern const QString pnBic;
extern const QString pnAccount;

struct wpBankAccount : public QWizardPage{
    wpBankAccount(QWidget* p);
    void cleanupPage() override{}; // do not change data on back
    bool validatePage()   override;
    int nextId() const    override;
private:
    Q_OBJECT;
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};

extern const QString pnConfirmCreditor;

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
    QCheckBox*  cbCreateContract =nullptr;
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};

/********************************
CONTRACT creation wiz starts here
*********************************
*/

/*
 * wpLableAndAmount - ask basic contract data
*/
extern const QString pnLabel;
extern const QString pnAmount;

struct wpLableAndAmount : public QWizardPage{
    wpLableAndAmount(QWidget* p);
    void initializePage() override;
    void cleanupPage() override;
    bool validatePage()   override;
    int nextId() const    override;
private:
    Q_OBJECT;
    QLabel* contractLabel = nullptr;
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};

/*
 * wpContractTimeFrame - we have to ask contract date first,
 * so that we can list proper investments later
 */
extern const QString pnCDate;
extern const QString pnEDate;
extern const QString pnPeriod;
extern const QString pnContractComment;

class wpContractTimeframe : public QWizardPage{
    Q_OBJECT
public:
    wpContractTimeframe(QWidget*);
    void initializePage() override;
    bool validatePage()   override;
    int nextId() const    override;
public slots:
    void onNoticePeriod_currentIndexChanged(int i);

private:
    bool init =true;
    QDateEdit* deTerminationDate; // to enable / disable we need the pointer to the cBox
    QComboBox* cbNoticePeriod;    // to access the itemData
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};

/*
 * wpInterestSelectionMode - ask: investment or interest?
*/
struct wpInterestSelectionMode : public QWizardPage {
    // select between "choose from investment" and select %
    wpInterestSelectionMode(QWidget* p);
    void initializePage() override;
    int nextId() const override;
private:
    Q_OBJECT;
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};

/*
 * wpInterestFromInvestment - deduce the interest from an investment
*/
struct wpInterestFromInvestment : public QWizardPage {
    wpInterestFromInvestment(QWidget* w);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;
public slots:
    void onInvestments_currentIndexChanged(int);
private:
    Q_OBJECT
    QComboBox* cbInvestments =nullptr; // to read itemData on validation
    QLabel* lblInvestmentInfo =nullptr;
    qlonglong rowid_investment =-1;
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};

/*
 * wpInterestSelection select interest from
 */
struct wpInterestSelection : public QWizardPage {
    wpInterestSelection(QWidget* p);
    bool validatePage() override;
    int nextId() const    override;
private:
    Q_OBJECT;
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};

/*
 * wpInterestPayoutMode select payout mode for the interest
 */
extern const QString pnIPaymentDelayed;
struct wpInterestPayoutMode : public QWizardPage {
    wpInterestPayoutMode(QWidget* p);
    bool validatePage() override;
    int nextId() const override;
private:
    Q_OBJECT;
    QComboBox* cbImode =nullptr;
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};

/*
 * wpContractConfirmation get users confirmation
 */
extern const QString pnConfirmContract;

class wpConfirmContract : public QWizardPage
{
    Q_OBJECT;
public:
    wpConfirmContract(QWidget*);
    void initializePage() override;
private:
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};

/*
 * *********************************
 *        the WIZARD
 * *********************************
 */

struct wizNew : public QWizard
{
    wizNew(QWidget* p =nullptr);
    qlonglong creditorId =-1;
    int interest =-1;
    qlonglong investmentId =0;
    int noticePeriod =-1;
    interestModel iPaymentMode =interestModel::maxId;
    bool createCreditor =false; // the corresponding field becomes false on cancel during contract dialog
    creditor c_tor;
    bool selectCreateContract =true; // for edit creditor this should be false
private:
    Q_OBJECT;
    bool updateMode =false;
};


#endif // WIZNEW_H
