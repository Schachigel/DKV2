#ifndef DLGINTERESTLETTERS_H
#define DLGINTERESTLETTERS_H



class dlgInterestLetters : public QDialog
{
    Q_OBJECT
public:
    //explicit dlgInterestLetters(QWidget *parent = nullptr, int currentYear = 0);
    explicit dlgInterestLetters(QWidget *parent = nullptr, QVector<int> years =QVector<int>());
    bool print_csv() { return csv->isChecked();}
    bool confirmed() { return confirm->isChecked();}
    virtual void setYear(int y);
    int getYear() { return yearSelector->currentData ().toInt (); }
private slots:
    void confirmChanged(Qt::CheckState state);

private:
    int year;
    QVector<int> years;
    QDialogButtonBox* buttons;
    QCheckBox* csv;
    QCheckBox* confirm;
    QComboBox *yearSelector;
};

#endif // DLGINTERESTLETTERS_H
