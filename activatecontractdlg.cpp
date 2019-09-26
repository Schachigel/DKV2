#include "activatecontractdlg.h"
#include "ui_activatecontractdlg.h"

activateContractDlg::activateContractDlg(QWidget *parent, QDate date) :
    QDialog(parent),
    ui(new Ui::activateContractDlg)
{
    ui->dateEdit->setDate(date);
    ui->setupUi(this);
}

activateContractDlg::~activateContractDlg()
{
    delete ui;
}
