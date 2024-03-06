#include "helper.h"
#include "dlgaskcontractlabel.h"

dlgAskContractLabel::dlgAskContractLabel(const QString& oldLabel) : oldLabel(oldLabel)
{
    QLabel* header =new QLabel(qsl("Vertragskennung anpassen"));
    QLabel* msg    =new QLabel(qsl("Mit diesem Dialog kannst Du die Kennung eines Vertrages ändern.<p>"
                                   " <b>Du solltest die Kennung jedoch nur aus wichtigem Grund ändern, <br>denn sie ist ein wichtiges Identifikationsmerkmal eines Vertrages.</b>"));
    msg->setTextFormat (Qt::RichText);

    leLabel =new QLineEdit(oldLabel);

    buttons =new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    connect(buttons, &QDialogButtonBox::accepted, this, &dlgAskContractLabel::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &dlgAskContractLabel::reject);

    QGridLayout* g =new QGridLayout();
    g->setColumnMinimumWidth(0, 30);
    g->setColumnMinimumWidth(2, 30);
    g->setRowMinimumHeight(0, 20);

    g->addWidget (header, 1,1);
    g->setRowMinimumHeight (2,20);
    g->addWidget (msg, 3, 1);
    g->setRowMinimumHeight(4, 20);

    g->addWidget (leLabel, 5, 1);
    g->setRowMinimumHeight (6,20);
    g->addWidget (buttons, 7,1,1,2);

    setLayout (g);
}

void dlgAskContractLabel::showEvent(QShowEvent* se)
{   LOG_CALL;
    if( se->spontaneous())
        return;
    centerDlg(qobject_cast<QWidget*>(parent()), this, 200, 300);
}
