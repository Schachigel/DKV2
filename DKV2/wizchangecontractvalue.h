#ifndef CHANGECONTRACTVALUEWIZ_H
#define CHANGECONTRACTVALUEWIZ_H

#include <QDate>
#include <QWizard>

class wizChangeContract_IntroPage : public QWizardPage
{
public:
    wizChangeContract_IntroPage(QWidget* parent =nullptr);
    void initializePage() override;
    bool validatePage() override;
};

struct wizChangeContract_AmountPage : public QWizardPage
{
    wizChangeContract_AmountPage(QWidget* parent =nullptr);
    void initializePage() override;
    bool validatePage() override;
};

struct wizChangeContract_DatePage : public QWizardPage
{
    wizChangeContract_DatePage(QWidget* parent =nullptr);
    void initializePage() override;
    bool validatePage() override;
};

struct wizChangeContract_Summary : public QWizardPage{
    wizChangeContract_Summary(QWidget* p =nullptr);
    void initializePage() override;
    bool validatePage() override;
};

struct wizChangeContract : public QWizard
{
    wizChangeContract(QWidget* p =nullptr);
    QString creditorName;
    QString contractLabel;
    double  currentAmount = 0.;
    QDate   earlierstDate;
};

#endif // CHANGECONTRACTVALUEWIZ_H
