#ifndef DLGASKDATA_H
#define DLGASKDATA_H

#include <QDialog>
class QWidget;
class QLabel;
class QDateEdit;
class QDialogButtonBox;
class QDate;

class dlgAskDate : public QDialog
{
    Q_OBJECT
public:
    explicit dlgAskDate(QWidget *parent = nullptr);
    void setDate(QDate d) {dateEdit->setDate(d);}
    QDate date(void) {return dateEdit->date();}
    void setHeader(const QString &s) {header->setText(s);}
    void setMsg(const QString &s) {msg->setText(s);}
// signals:
// private slots:
private:
    QLabel* header;
    QLabel* msg;
    QDateEdit* dateEdit;
    QDialogButtonBox* buttons;
};

#endif // DLGASKDATA_H
