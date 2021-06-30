#include "dlgaskdata.h"
#include "helper.h"
#include "pch.h"

dlgAskDate::dlgAskDate(QWidget *parent) : QDialog(parent)
{
    header =new QLabel( qsl("Abgelaufene Geldanlagen beenden"), this);
    setFontPs(header, 14);

    msg =new QLabel(qsl("Wähle das Datum bis zu dem Geldanlagen geschlossen werden sollen.<p>Alle Geldanlagen, deren Endedatum vor diesem Datum liegt, werden geschlossen."), this);
    msg->setWordWrap(true);
    setFontPs(msg, 10);

    dateEdit =new QDateEdit(QDate::currentDate(), this);
    setFontPs(dateEdit, 10);

    buttons =new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    connect(buttons, &QDialogButtonBox::accepted, this, &dlgAskDate::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &dlgAskDate::reject);

    QVBoxLayout* layout =new QVBoxLayout();
    layout->addWidget(header);
    layout->addStretch(1);
    layout->addWidget(msg);
    layout->addStretch(1);
    layout->addWidget(dateEdit);
    layout->addStretch(1);
    layout->addWidget(buttons);
    setLayout(layout);
}

void dlgAskDate::showEvent(QShowEvent* se)
{   LOG_CALL;
    if( se->spontaneous())
        return;
    centerDlg(qobject_cast<QWidget*>(parent()), this);
}
