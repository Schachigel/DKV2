#ifndef FRMJAHRESABSCHLUSS_H
#define FRMJAHRESABSCHLUSS_H

#include <QDialog>
#include "jahresabschluss.h"


namespace Ui {
class frmJahresabschluss;
}

class frmJahresabschluss : public QDialog
{
    Q_OBJECT

public:
    explicit frmJahresabschluss(const jahresabschluss& JA, QWidget *parent = nullptr);
    ~frmJahresabschluss();

private slots:
    void on_pbOK_clicked();

private:
    Ui::frmJahresabschluss *ui;
    const jahresabschluss& ja;
    void fillTesaList();
    void fillnTesaList();
};

#endif // FRMJAHRESABSCHLUSS_H
