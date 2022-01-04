#ifndef WIZNEWINVESTMENT_H
#define WIZNEWINVESTMENT_H

#include "pch.h"
#include "investment.h"

extern const QString pnVon /*=qsl("von*")*/;
extern const QString pnBis /*=qsl("bis*")*/;
extern const QString pnTyp /*=qsl("typ*")*/;
extern const QString pnZSatz /*=qsl("zs*")*/;
extern const QString pnKorrekt /*=qsl("OK*")*/;

class wpInvestmentSummary : public QWizardPage
{
    Q_OBJECT
public:
    wpInvestmentSummary(QWidget* w =nullptr);
    void initializePage() override;
//    bool validatePage() override;
private:
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};

struct wpType : public QWizardPage
{
    Q_OBJECT
public:
    wpType(QWidget* p =nullptr);
    void initializePage() override;
//    bool validatePage() override;
private:
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
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
    void onSwitchFloating(int state);

private:
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
    QDateEdit* deBis;
    QDateEdit* deVon;
    QCheckBox* cbFloating;
};

struct wpNewInvestInterest : public QWizardPage
{
    wpNewInvestInterest(QWidget* p =nullptr);
    // void initializePage() override;
    // bool validatePage() override;
private:
    Q_OBJECT;
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
};

struct wizNewInvestment : public QWizard
{
public:
    wizNewInvestment(QWidget* w =nullptr);
private:
    Q_OBJECT;
};

#endif // WIZNEWINVESTMENT_H
