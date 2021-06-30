#ifndef DLGCHANGECONTRACTTERMINATION_H
#define DLGCHANGECONTRACTTERMINATION_H

#include "pch.h"

class dlgChangeContractTermination : public QDialog
{
    Q_OBJECT
public:
    explicit dlgChangeContractTermination(QWidget* p =nullptr);
    void setMinContractTerminationDate(QDate d) { minContractTerminationDate =d;}
    void setNoticePeriod(int i);
    void setEndDate(QDate d) { terminationDate->setDate(d);}
    int noticePeriod() { return notPeriod->currentData().toInt();}
    QDate endDate() { return terminationDate->date();}
private slots:
    void showEvent(QShowEvent*) override;
    void onNoticePeriod_currentIndexChanged(int i);
    void onAllowBothChanged(int i);
    void accept() override;
private:
    QDate minContractTerminationDate;
    QLabel* title;
    QLabel* subTitle;
    QDialogButtonBox* buttons;
    QComboBox* notPeriod;
    QDateEdit* terminationDate;
    QCheckBox* allowBoth;
};

#endif // DLGCHANGECONTRACTTERMINATION_H
