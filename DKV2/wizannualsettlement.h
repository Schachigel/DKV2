#ifndef WIZANNUALSETTLEMENT_H
#define WIZANNUALSETTLEMENT_H

#include <QDate>
#include <QStringLiteral>
#include <QWizard>

class wpAnnualSettlement_IntroPage : public QWizardPage
{
    Q_OBJECT;
public:
    wpAnnualSettlement_IntroPage(QWidget* p =nullptr);
    void initializePage() override;
    bool isComplete() const override;
public slots:
    void onConfirmData_toggled(int);
};

struct wizAnnualSettlement : public QWizard
{
    wizAnnualSettlement(QWidget* p =nullptr);
    int year;
    Q_OBJECT;
};

#endif // WIZANNUALSETTLEMENT_H
