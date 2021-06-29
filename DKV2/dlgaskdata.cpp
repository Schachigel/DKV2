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

    int nWidth =250, nHeight =400;
    if (parent != NULL) {
        QPoint parentPos = parent->mapToGlobal(parent->pos());
        parentPos.setX(parentPos.x()+parent->width()/2 - nWidth/2);
        parentPos.setY(parentPos.y() + parent->height()/2 -nHeight/2);
        parentPos= parent->mapFromGlobal(parentPos);

        setGeometry(parentPos.x(),
                    parentPos.y(),
                    nWidth, nHeight);
    }
}
