
#include "helper.h"
#include "helperfin.h"
#include "dlgannualsettlement.h"

dlgAnnualsettlement::dlgAnnualsettlement(QWidget *parent, int year) : QDialog(parent), year(year)
{
    setFontPs(this, 10);

    QGridLayout* g =new QGridLayout();
    // get some space left and right
    g->setColumnMinimumWidth(0, 30);
    g->setColumnMinimumWidth(2, 30); // last col

    int row =0;
    g->setRowMinimumHeight(row++, 20);

    QLabel* title =new QLabel(qsl("Jahresabrechnung"));
    setFontPs(title, 14);
    g->addWidget(title, row++, 1);

    Q_ASSERT(year);
    QString msgtxt =qsl("Die Abrechnung für das folgende Jahr kann gemacht werden:<p><b>%1<b>");
    QLabel* msg =new QLabel(msgtxt.arg(i2s(year)));
    msg->setWordWrap(true);
    g->addWidget(msg, row++, 1);

    csv =new QCheckBox(qsl("Buchungen als csv Datei ausgeben."));
    g->addWidget(csv, row++, 1);

    confirm =new QCheckBox(qsl("Jahresabrechnung jetzt durchführen."));
    g->addWidget(confirm, row++, 1);
    connect(confirm, &QCheckBox::stateChanged, this, &dlgAnnualsettlement::confirmChanged);

    buttons =new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
    connect(buttons, &QDialogButtonBox::accepted, this, &dlgAnnualsettlement::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &dlgAnnualsettlement::reject);
    g->setRowMinimumHeight(row++, 20);
    g->addWidget(buttons, row++, 1, 1, 2);

    setLayout(g);
}

void dlgAnnualsettlement::confirmChanged(int state)
{
    if( state == Qt::Checked)
        buttons->button(QDialogButtonBox::Ok)->setEnabled(true);
    else
        buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
}
