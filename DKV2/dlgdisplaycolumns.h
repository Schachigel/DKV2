#ifndef DLGDISPLAYCOLUMNS_H
#define DLGDISPLAYCOLUMNS_H

#include <QDialog>

struct dlgDisplayColumns : public QDialog
{
public:
    explicit dlgDisplayColumns(const QVector<QPair<int, QString>>& colInfo, const QBitArray& status, QWidget *parent =nullptr);
    QBitArray getNewStatus() {return status;};
private slots:
    void accept() override;

private:
    QBitArray status;
    QVector<QPair<int, QString>> colInfo;
    QVector<QCheckBox*> checkboxes;
};

#endif // DLGDISPLAYCOLUMNS_H
