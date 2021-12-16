#ifndef DLGINTERESTLETTERS_H
#define DLGINTERESTLETTERS_H

#include "pch.h"
#include <QSpinBox>

class dlgInterestLetters : public QDialog
{
    Q_OBJECT
public:
    explicit dlgInterestLetters(QWidget *parent = nullptr, int currentYear = 0);
    bool print_csv() { return csv->isChecked();}
    bool confirmed() { return confirm->isChecked();}
    virtual void setYear(int y);
    int getYear() { return yearSelector->value(); }
private slots:
    void confirmChanged(int state);

private:
    int year;
    QDialogButtonBox* buttons;
    QCheckBox* csv;
    QCheckBox* confirm;
    QSpinBox *yearSelector;
};

#endif // DLGINTERESTLETTERS_H
