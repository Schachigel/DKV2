#ifndef ACTIVATECONTRACTWIZ_H
#define ACTIVATECONTRACTWIZ_H

#include <QDate>
#include <QWizard>

struct wpInitialPayment_IntroPage : public QWizardPage {
    wpInitialPayment_IntroPage(QWidget* w =nullptr);
    void initializePage() override;
    Q_OBJECT;
};

extern QString fnDate;
extern QString fnActivateNow;
struct wpInitialPayment_DatePage : public QWizardPage {
    wpInitialPayment_DatePage(QWidget* w=nullptr);
    void cleanupPage() override  {};
    void initializePage() override;
    bool validatePage() override;
    QDate minDate;
    Q_OBJECT;
};

extern QString fnAmount;
struct wpInitialPayment_AmountPage : public QWizardPage {
    wpInitialPayment_AmountPage(QWidget* w=nullptr);
    void cleanupPage() override  {};
    bool validatePage() override;
    Q_OBJECT;
};

class wpInitialPayment_SummaryPage : public QWizardPage {
    Q_OBJECT;
public:
    wpInitialPayment_SummaryPage(QWidget* w=nullptr);
    void initializePage() override;
    bool validatePage() override;
    bool isComplete() const override;
public slots:
    void onConfirmData_toggled(int );
};


class wizInitialPayment : public QWizard
{
    Q_OBJECT
public:
    wizInitialPayment(QWidget* p =nullptr);
    QString label;
    QString creditorName;
    double expectedAmount =0.;
    //  the contract should not be activated before the contract date
    QDate minimalActivationDate;
};

#endif // ACTIVATECONTRACTWIZ_H
