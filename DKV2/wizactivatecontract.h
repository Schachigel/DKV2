#ifndef ACTIVATECONTRACTWIZ_H
#define ACTIVATECONTRACTWIZ_H

#include <QDate>
#include <QWizard>

struct wpActivateContract_IntroPage : public QWizardPage {
    wpActivateContract_IntroPage(QWidget* w =nullptr);
    void initializePage() override;
    Q_OBJECT;
};

extern QString fnDate;
struct wpActiateContract_DatePage : public QWizardPage {
    wpActiateContract_DatePage(QWidget* w=nullptr);
    void cleanupPage() override  {};
    void initializePage() override;
    bool validatePage() override;
    QDate minDate;
    Q_OBJECT;
};

extern QString fnAmount;
struct wpActiateContract_AmountPage : public QWizardPage {
    wpActiateContract_AmountPage(QWidget* w=nullptr);
    void cleanupPage() override  {};
    bool validatePage() override;
    Q_OBJECT;
};

class wpActivateContract_SummaryPage : public QWizardPage {
    Q_OBJECT;
public:
    wpActivateContract_SummaryPage(QWidget* w=nullptr);
    void initializePage() override;
    bool validatePage() override;
    bool isComplete() const override;
public slots:
    void onConfirmData_toggled(int );
};


class wpActivateContract : public QWizard
{
    Q_OBJECT
public:
    wpActivateContract(QWidget* p =nullptr);
    QString label;
    QString creditorName;
    double expectedAmount =0.;
    //  the contract should not be activated before the contract date
    QDate minimalActivationDate;
};

#endif // ACTIVATECONTRACTWIZ_H
