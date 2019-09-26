#ifndef ACTIVATECONTRACTDLG_H
#define ACTIVATECONTRACTDLG_H

#include <QDialog>
#include <QDate>

namespace Ui {
class activateContractDlg;
}

class activateContractDlg : public QDialog
{
    Q_OBJECT

public:
    explicit activateContractDlg(QWidget *parent = nullptr, QDate date=QDate::currentDate());
    ~activateContractDlg();

private:
    Ui::activateContractDlg *ui;
};

#endif // ACTIVATECONTRACTDLG_H
