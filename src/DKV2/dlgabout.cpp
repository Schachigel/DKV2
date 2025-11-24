

#include "helper.h"
#include "dlgabout.h"
#include "qboxlayout.h"

dlgAbout::dlgAbout(QWidget *parent) : QDialog(parent)
{
    header =new QLabel();
    header->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    header->setWordWrap (true);
    msg =new QLabel();
    msg->setTextFormat (Qt::RichText);
    msg->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    msg->setWordWrap (true);
    QVBoxLayout* vl =new QVBoxLayout();
    vl->setSizeConstraint(QLayout::SetMinimumSize);
    vl->addWidget (header);
    vl->addWidget (msg);
    setLayout(vl);
}

void dlgAbout::showEvent(QShowEvent* e)
{
    if( e->spontaneous ())
        return;
    centerDlg(qobject_cast<QWidget*>(parent()), this, 600, 400);
    this->adjustSize ();
}
