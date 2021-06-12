#include "askdatedlg.h"
#include "ui_askdatedlg.h"

AskDateDlg::AskDateDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AskDateDlg)
{
    ui->setupUi(this);
}

AskDateDlg::~AskDateDlg()
{
    delete ui;
}

void AskDateDlg::setDate(QDate d)
{
    _date =d;
    ui->dateE->setDate(_date);
}
