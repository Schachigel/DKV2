#ifndef DLGDISPLAYCOLUMNS_H
#define DLGDISPLAYCOLUMNS_H



struct dlgDisplayColumns : public QDialog
{
public:
    explicit dlgDisplayColumns(const QVector<QPair<int, QString>>& colInfo, const QBitArray& status, QWidget *parent =nullptr);
    QBitArray getNewStatus() {return status;};
private slots:
    void accept() override;
    void selectAll();

private:
    QBitArray status;
    QVector<QPair<int, QString>> colInfo;
    QVector<QCheckBox*> checkboxes;
};

#endif // DLGDISPLAYCOLUMNS_H
