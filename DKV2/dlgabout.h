#ifndef DLGABOUT_H
#define DLGABOUT_H


#include "qobjectdefs.h"
class dlgAbout : public QDialog
{
    Q_OBJECT
public:
    explicit dlgAbout(QWidget *p =nullptr);
    void setHeader(const QString &s) {header->setText(s);}
    void setMsg(const QString &s) {msg->setText(s);}
private slots:
    void showEvent(QShowEvent*) override;
private:
    QLabel* header;
    QLabel*msg;
};

#endif // DLGABOUT_H
