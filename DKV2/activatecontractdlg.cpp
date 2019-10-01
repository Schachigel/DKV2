#include "activatecontractdlg.h"
#include "ui_activatecontractdlg.h"

activateContractDlg::activateContractDlg(QWidget *parent, QDate date) :
    QDialog(parent),
    ui(new Ui::activateContractDlg)
{
    ui->setupUi(this);
    ui->dateEdit->setDate(date);
}

activateContractDlg::~activateContractDlg()
{
    delete ui;
}

QDate activateContractDlg::getActivationDate()
{
    return ui->dateEdit->date();
}
