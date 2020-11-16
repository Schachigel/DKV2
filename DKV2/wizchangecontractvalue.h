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
    Q_OBJECT;
};

struct wizChangeContract_AmountPage : public QWizardPage
{
    wizChangeContract_AmountPage(QWidget* parent =nullptr);
    void cleanupPage() override  {};
    void initializePage() override;
    bool validatePage() override;
    Q_OBJECT;
};

struct wizChangeContract_DatePage : public QWizardPage
{
    wizChangeContract_DatePage(QWidget* parent =nullptr);
    void cleanupPage() override  {};
    void initializePage() override;
    bool validatePage() override;
    Q_OBJECT;
};

class wizChangeContract_Summary : public QWizardPage{
    Q_OBJECT;
public:
    wizChangeContract_Summary(QWidget* p =nullptr);
    void initializePage() override;
    bool isComplete() const override;
public slots:
    void onConfirmData_toggled(int);
};

struct wizChangeContract : public QWizard
{
    wizChangeContract(QWidget* p =nullptr);
    QString creditorName;
    QString contractLabel;
    double  currentAmount = 0.;
    QDate   earlierstDate;
    Q_OBJECT;
};

#endif // CHANGECONTRACTVALUEWIZ_H
