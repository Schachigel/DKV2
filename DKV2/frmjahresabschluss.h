#ifndef FRMJAHRESABSCHLUSS_H
#define FRMJAHRESABSCHLUSS_H

#include <QDialog>
#include <QStandardItemModel>

#include "vertrag.h"
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
    void on_btnCsv_clicked();
    void on_pbKontoauszug_clicked();

private:
    Ui::frmJahresabschluss *ui;
    const jahresabschluss& ja;
    QStandardItemModel* getModelFromContracts(const QVector<Contract>& vertraege) const;
};

#endif // FRMJAHRESABSCHLUSS_H
