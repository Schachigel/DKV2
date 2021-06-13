#ifndef ASKDATEDLG_H
#define ASKDATEDLG_H

#include <pch.h>
//#include <QDialog>

namespace Ui {
class AskDateDlg;
}

class AskDateDlg : public QDialog
{
    Q_OBJECT

public:
    explicit AskDateDlg(QWidget *parent = nullptr);
    ~AskDateDlg();
    void setDate(QDate d);
    QDate date (){ return _date; }

private:
    Ui::AskDateDlg *ui;
    QDate _date;
};

#endif // ASKDATEDLG_H
