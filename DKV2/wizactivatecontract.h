#ifndef ACTIVATECONTRACTWIZ_H
#define ACTIVATECONTRACTWIZ_H

#include <QDate>
#include <QWizard>

struct wizActivateContract_IntroPage : public QWizardPage {
    wizActivateContract_IntroPage(QWidget* w =nullptr);
    void initializePage() override;
    Q_OBJECT;
};

struct wizActiateContract_DatePage : public QWizardPage {
    wizActiateContract_DatePage(QWidget* w=nullptr);
    void initializePage() override;
    Q_OBJECT;
};

struct wizActiateContract_AmountPage : public QWizardPage {
    wizActiateContract_AmountPage(QWidget* w=nullptr);
    bool validatePage() override;
    Q_OBJECT;
};

class wizActivateContract_SummaryPage : public QWizardPage {
    Q_OBJECT;
public:
    wizActivateContract_SummaryPage(QWidget* w=nullptr);
    void initializePage() override;
    bool validatePage() override;
    bool isComplete() const override;
public slots:
    void onConfirmData_toggled(int );
};


struct activateContractWiz : public QWizard
{
    activateContractWiz(QWidget* p =nullptr);
    QString label;
    QString creditorName;
    double expectedAmount;
    Q_OBJECT;
};

#endif // ACTIVATECONTRACTWIZ_H
