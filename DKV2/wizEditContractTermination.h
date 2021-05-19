#ifndef WIZEDITCONTRACTTERMINATION_H
#define WIZEDITCONTRACTTERMINATION_H

#include "pch.h"

extern const QString pnNewEDate;

class wpEditContractTermination : public QWizardPage
{
    Q_OBJECT
public:
    wpEditContractTermination(QWidget* =nullptr);
    void initializePage() override;
    bool validatePage() override;
public slots:
    void onNoticePeriod_currentIndexChanged(int i);
    void onAllowBothChanged(int i);
private:
    bool init =true;
    QDateEdit* deTerminationDate;
    QComboBox* cbNoticePeriod;
    QCheckBox* chkAllowBoth;
};

class wizEditContractTermination : public QWizard{
    Q_OBJECT
public:
    wizEditContractTermination(QWidget* =nullptr);
    QDate minContractTermination;
    int newNoticePeriod;
};


#endif // WIZEDITCONTRACTTERMINATION_H
