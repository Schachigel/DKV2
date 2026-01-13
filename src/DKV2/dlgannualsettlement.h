#ifndef DLGANNUALSETTLEMENT_H
#define DLGANNUALSETTLEMENT_H



class dlgAnnualSettlement : public QDialog
{
    Q_OBJECT
public:
    explicit dlgAnnualSettlement( int year =0, QWidget *parent = nullptr);
    bool confirmed() { return confirm->isChecked();}
    void setYear(int y) {year =y;}
private:
    int year =0;
    QDialogButtonBox* buttons;
    QCheckBox* confirm;
private slots:
    void confirmChanged(Qt::CheckState state);
};

#endif // DLGANNUALSETTLEMENT_H
