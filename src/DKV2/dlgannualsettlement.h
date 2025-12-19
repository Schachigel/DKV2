#ifndef DLGANNUALSETTLEMENT_H
#define DLGANNUALSETTLEMENT_H



class dlgAnnualsettlement : public QDialog
{
    Q_OBJECT
public:
    explicit dlgAnnualsettlement( int year =0, QWidget *parent = nullptr);
    bool print_csv() { return csv->isChecked();}
    bool confirmed() { return confirm->isChecked();}
    void setYear(int y) {year =y;}
private:
    int year =0;
    QDialogButtonBox* buttons;
    QCheckBox* csv;
    QCheckBox* confirm;
signals:
    void confirmChanged(Qt::CheckState state);
};

#endif // DLGANNUALSETTLEMENT_H
