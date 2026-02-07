#ifndef WIZNEW_H
#define WIZNEW_H

#include "uihelper.h"

#include "creditor.h"
#include "contract.h"

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


inline const QString pnNew{qsl("create_new")};
inline const QString pnCreditor{qsl("creditor")};
inline const QString pnFName{qsl("firstname")};
inline const QString pnLName{qsl("lastname")};
inline const QString pnStreet{qsl("street")};
inline const QString pnCity{qsl("city")};
inline const QString pnPcode{qsl("pcode")};
inline const QString pnCountry{qsl("country")};
inline const QString pnEMail{qsl("e-mail")};
inline const QString pnPhone{qsl("phone")};
inline const QString pnContact{qsl("contact")};
inline const QString pnComment{qsl("comment")};
inline const QString pnConfirmCreditor{qsl("confirmCreateContract")};
inline const QString pnCreateContract{qsl("createContract")};
inline const QString pnLabel{qsl("label")};
inline const QString pnAmount{qsl("amount")};
inline const QString pnContractComment{qsl("contractComment")};
inline const QString pnCDate{qsl("startD")};
inline const QString pnEDate{qsl("endD")};
inline const QString pnPeriod{qsl("noticePeriod")};
inline const QString pnConfirmContract{qsl("confirmContract")};
inline const QString pnIMode{qsl("imode")};
inline const QString pnIPaymentDelayed{qsl("ipnd")};

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
struct wpNewCreditorAddress : public QWizardPage{
    wpNewCreditorAddress(QWidget* p);
    void cleanupPage() override {};
    bool validatePage()   override;
    int nextId() const    override;
private:
    Q_OBJECT
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};

struct wpEmail : public QWizardPage {
    wpEmail (QWidget* p);
    void cleanupPage() override  {}; // do not change data on back
    bool validatePage()   override;
    int nextId() const    override;
private:
    Q_OBJECT
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};

inline const QString pnIban{qsl("iban")};
inline const QString pnBic{qsl("bic")};
inline const QString pnAccount{qsl("account")};

struct wpBankAccount : public QWizardPage{
    wpBankAccount(QWidget* p);
    void cleanupPage() override{}; // do not change data on back
    bool validatePage()   override;
    int nextId() const    override;
private:
    Q_OBJECT
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};


class wpConfirmCreditor : public QWizardPage{
    Q_OBJECT
public:
    wpConfirmCreditor(QWidget* p);
    void initializePage() override;
    bool validatePage()   override;
    int nextId() const    override;
public slots:
    void onConfirmCreateContract_toggled(Qt::CheckState state);
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
struct wpLableAndAmount : public QWizardPage{
    wpLableAndAmount(QWidget* p);
    void initializePage() override;
    void cleanupPage() override;
    bool validatePage()   override;
    int nextId() const    override;
private:
    Q_OBJECT
    QLabel* contractLabel = nullptr;
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};

/*
 * wpContractTimeFrame - we have to ask contract date first,
 * so that we can list proper investments later
 */
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
    Q_OBJECT
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
    tableindex_t rowid_investment =SQLITE_invalidRowId;
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
    Q_OBJECT
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};

/*
 * wpInterestPayoutMode select payout mode for the interest
 */
struct wpInterestPayoutMode : public QWizardPage {
    wpInterestPayoutMode(QWidget* p);
    bool validatePage() override;
    int nextId() const override;
private:
    Q_OBJECT
    QComboBox* cbImode =nullptr;
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};

/*
 * wpContractConfirmation get users confirmation
 */
class wpConfirmContract : public QWizardPage
{
    Q_OBJECT
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
    wizNew(creditor& c, QWidget* p =nullptr);
    tableindex_t existingCreditorId =SQLITE_invalidRowId;
    int interest =-1;
    tableindex_t investmentId =SQLITE_invalidRowId;
    int noticePeriod =-1;
    interestModel iPaymentMode =interestModel::maxId;
    bool createCreditor =false; // the corresponding field becomes false on cancel during contract dialog
    creditor& cred;
    bool selectCreateContract =true; // for edit creditor this should be false
private:
    Q_OBJECT
    bool updateMode =false;
};


#endif // WIZNEW_H
