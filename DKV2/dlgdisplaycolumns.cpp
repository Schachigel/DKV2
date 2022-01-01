
#include "pch.h"
#include "helper.h"
#include "dlgdisplaycolumns.h"

const QString visible {qsl("sichtbar")};
const QString hidden  {qsl("ausgeblendet")};
const QString init    {qsl("init")};


dlgDisplayColumns::dlgDisplayColumns(const QVector<QPair<int, QString>>& colInfo, const QBitArray& status, QWidget* p)
    : QDialog(p), status(status), colInfo(colInfo)
{
    QLabel* header =new QLabel( qsl("Wähle die Spalten, die angezeigt werden sollen!"));
    setFontPs(header, 14);
    QVBoxLayout* layout =new QVBoxLayout();
    layout->addWidget (header);
    layout->addWidget (new QLabel());

    for( const QPair<int, QString>& col : qAsConst(colInfo)) {
        if( col.second.isEmpty ()) continue;
        QCheckBox* cb =new QCheckBox(col.second);
        cb->setChecked (status[col.first]);
        checkboxes.push_back (cb);
        layout->addWidget (cb);
    }

    layout->addWidget (new QLabel());

    QDialogButtonBox* buttons =new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    layout->addWidget (buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &dlgDisplayColumns::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &dlgDisplayColumns::reject);

    setLayout (layout);
}

void dlgDisplayColumns::accept ()
{
    int checkboxIndex =0;
    for( const QPair<int, QString>& col : qAsConst(colInfo)) {
        if( col.second.isEmpty ()) continue;
        if( checkboxes[checkboxIndex]->isChecked ())
            status[col.first] =true;
        else
            status[col.first] =false;
        checkboxIndex++;
    }
    QDialog::accept ();
}
