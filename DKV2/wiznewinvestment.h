#ifndef WIZNEWINVESTMENT_H
#define WIZNEWINVESTMENT_H

#include <QWizard>
#include <QComboBox>

#include "investment.h"

extern const QString pnVon /*=qsl("von*")*/;
extern const QString pnBis /*=qsl("bis*")*/;
extern const QString pnTyp /*=qsl("typ*")*/;
extern const QString pnZSatz /*=qsl("zs*")*/;
extern const QString pnKorrekt /*=qsl("OK*")*/;

struct wpInvestmentSummary : public QWizardPage
{
    wpInvestmentSummary(QWidget* w =nullptr);
    void initializePage() override;
//    bool validatePage() override;
};

struct wpType : public QWizardPage
{
    wpType(QWidget* p =nullptr);
    void initializePage() override;
//    bool validatePage() override;
private:
    Q_OBJECT;
};

class wpTimeFrame : public QWizardPage
{
    Q_OBJECT;
public:
    wpTimeFrame(QWidget* p =nullptr);
    void initializePage() override;
    bool validatePage() override;
private slots:
    void onStartDate_changed();
};

struct wpNewInvestInterest : public QWizardPage
{
    wpNewInvestInterest(QWidget* p =nullptr);
    // void initializePage() override;
    // bool validatePage() override;
private:
    Q_OBJECT;
};

struct wizNewInvestment : public QWizard
{
public:
    wizNewInvestment(QWidget* w =nullptr);
private:
    Q_OBJECT;
};

#endif // WIZNEWINVESTMENT_H
