#include "askdatedlg.h"
#include "ui_askDateDlg.h"

askDateDlg::askDateDlg(QWidget *parent, QDate date) :
    QDialog(parent),
    ui(new Ui::askDateDlg)
{
    ui->setupUi(this);
    ui->lblMsg->setText("<H3>Mit der Aktivierung des Vertrags beginnt die Zinsberechnung. "
                       "<br>Bitte geben Sie das Datum des Geldeingangs ein:");
    ui->lblDate->setText("Die Verzinsung beginnt am ");
    ui->dateEdit->setDate(date);
}
void askDateDlg::setDate(QDate d)
{
    ui->dateEdit->setDate(d);
}

askDateDlg::~askDateDlg()
{
    delete ui;
}

void askDateDlg::setMsg(const QString& msg)
{
    ui->lblMsg->setText(msg);
}

void askDateDlg::setDateLabel(const QString& msg)
{
    ui->lblDate->setText(msg);
}

QDate askDateDlg::getDate() const
{
    return ui->dateEdit->date();
}

bool askDateDlg::shouldPrint() const
{
    return ui->cbPrint->checkState() == Qt::CheckState::Checked;
}
