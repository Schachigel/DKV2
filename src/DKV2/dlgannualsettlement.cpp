#include "dlgannualsettlement.h"

#include "uihelper.h"
#include "helper_core.h"
#include "helperfin.h"

dlgAnnualSettlement::dlgAnnualSettlement(int year, QWidget *parent)
    : year(year), QDialog(parent)
{
    QGridLayout* g =new QGridLayout();
    // get some space left and right
    g->setColumnMinimumWidth(0, 30);
    g->setColumnMinimumWidth(2, 30); // last col

    int row =0;
    g->setRowMinimumHeight(row++, 20);

    QLabel* title =new QLabel(qsl("<h2>Jahresabrechnung</h2>"));
    g->addWidget(title, row++, 1);

    Q_ASSERT(year);
    QString msgtxt =qsl("Die Abrechnung für das folgende Jahr kann ausgeführt werden:<p><b>%1<b>");
    QLabel* msg =new QLabel(msgtxt.arg(i2s(year)));
    msg->setWordWrap(true);
    g->addWidget(msg, row++, 1);

    confirm =new QCheckBox(qsl("Jahresabrechnung jetzt durchführen."));
    g->addWidget(confirm, row++, 1);

    connect(confirm, &QCheckBox::checkStateChanged, this, &dlgAnnualSettlement::confirmChanged);

    // default dlg buttons - disable OK until "confirm" was checked
    buttons =new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
    connect(buttons, &QDialogButtonBox::accepted, this, &dlgAnnualSettlement::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &dlgAnnualSettlement::reject);

    g->setRowMinimumHeight(row++, 20);
    g->addWidget(buttons, row++, 1, 1, 2);

    setLayout(g);
}

void dlgAnnualSettlement::confirmChanged(Qt::CheckState state)
{
    if( state == Qt::Checked)
        buttons->button(QDialogButtonBox::Ok)->setEnabled(true);
    else
        buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
}
