#ifndef ACTIVATECONTRACTWIZ_H
#define ACTIVATECONTRACTWIZ_H

#include <QDate>
#include <QWizard>

struct wpActivateContract_IntroPage : public QWizardPage {
    wpActivateContract_IntroPage(QWidget* w =nullptr);
    void initializePage() override;
    Q_OBJECT;
};

struct wpActiateContract_DatePage : public QWizardPage {
    wpActiateContract_DatePage(QWidget* w=nullptr);
    void cleanupPage() override  {};
    void initializePage() override;
    Q_OBJECT;
};

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


struct wpActivateContract : public QWizard
{
    wpActivateContract(QWidget* p =nullptr);
    QString label;
    QString creditorName;
    double expectedAmount =0.;
    Q_OBJECT;
};

#endif // ACTIVATECONTRACTWIZ_H
