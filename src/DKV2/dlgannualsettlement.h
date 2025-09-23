#ifndef DLGANNUALSETTLEMENT_H
#define DLGANNUALSETTLEMENT_H



class dlgAnnualsettlement : public QDialog
{
    Q_OBJECT
public:
    explicit dlgAnnualsettlement(QWidget *parent = nullptr, int year =0);
    bool print_csv() { return csv->isChecked();}
    bool confirmed() { return confirm->isChecked();}
    void setYear(int y) {year =y;}
private slots:
    void confirmChanged(int state);
private:
    int year =0;
    QDialogButtonBox* buttons;
    QCheckBox* csv;
    QCheckBox* confirm;
};

#endif // DLGANNUALSETTLEMENT_H
