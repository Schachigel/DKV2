#ifndef DLGASKCONTRACTLABEL_H
#define DLGASKCONTRACTLABEL_H

#include <QDialog>
#include <QObject>
#include <QWidget>

class dlgAskContractLabel : public QDialog
{
public:
    dlgAskContractLabel(const QString& oldLabel);
    QString newLabel() {return leLabel->text ();};
private slots:
    void showEvent(QShowEvent*) override;

private:
    QString oldLabel;
    QLineEdit* leLabel;
    QDialogButtonBox* buttons;
};

#endif // DLGASKCONTRACTLABEL_H
