#ifndef ACTIVATECONTRACTDLG_H
#define ACTIVATECONTRACTDLG_H

#include <QDialog>
#include <QDate>

namespace Ui {
class askDateDlg;
}

class askDateDlg : public QDialog
{
    Q_OBJECT

public:
    explicit askDateDlg(QWidget *parent = nullptr, QDate date=QDate::currentDate());
    ~askDateDlg();
    QDate getDate() const;
    void setDate(QDate d);
    void setMsg(const QString& msg);
    void setDateLabel(const QString& msg);
    bool shouldPrint() const;
    void showPrintOption(bool show);
private:
    Ui::askDateDlg *ui;
};

#endif // ACTIVATECONTRACTDLG_H
