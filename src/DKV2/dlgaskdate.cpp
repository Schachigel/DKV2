
#include "helper.h"
#include "dlgaskdate.h"

dlgAskDate::dlgAskDate(QWidget *parent) : QDialog(parent)
{
    header =new QLabel( qsl("Abgelaufene Geldanlagen beenden"));

    msg =new QLabel(qsl("Wähle das Datum bis zu dem Geldanlagen geschlossen werden sollen.<p>Alle Geldanlagen, deren Endedatum vor diesem Datum liegt, werden geschlossen."));
    msg->setWordWrap(false);
    msg->setTextFormat (Qt::RichText);

    dateEdit =new QDateEdit(QDate::currentDate(), this);
    dateEdit->setDisplayFormat(qsl("dd.MM.yyyy"));

    buttons =new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    connect(buttons, &QDialogButtonBox::accepted, this, &dlgAskDate::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &dlgAskDate::reject);

    QGridLayout* g =new QGridLayout();
    g->setColumnMinimumWidth(0, 30);
    g->setColumnMinimumWidth(2, 30);
    g->setRowMinimumHeight(0, 20);

    g->addWidget(header, 1, 1);
    g->setRowMinimumHeight(2, 20);
    g->addWidget(msg, 3, 1);
    g->setRowMinimumHeight(4, 20);
    g->addWidget(dateEdit, 5, 1);
    g->setRowMinimumHeight(6, 20);
    g->addWidget(buttons, 7, 1, 1, 2);

    setLayout(g);
}

void dlgAskDate::showEvent(QShowEvent* se)
{   LOG_CALL;
    if( se->spontaneous())
        return;
    centerDlg(qobject_cast<QWidget*>(parent()), this, 200, 300);
}
