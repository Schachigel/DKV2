
#include "pch.h"
#include "helper.h"
#include "dlgdisplaycolumns.h"

const QString visible {qsl("sichtbar")};
const QString hidden  {qsl("ausgeblendet")};
const QString init    {qsl("init")};


dlgDisplayColumns::dlgDisplayColumns(const QVector<QPair<int, QString>>& colInfo, const QBitArray& status, QWidget* p)
    : QDialog(p), status(status), colInfo(colInfo)
{
    setFontPs(this, 10);
    QVBoxLayout* mainlayout =new QVBoxLayout();

    QLabel* header =new QLabel( qsl("<h3>Wähle die Spalten aus,<br> die angezeigt werden sollen!</h3>"));
    //header->setWordWrap (true);
    mainlayout->addWidget (header);

    QVBoxLayout* layout =new QVBoxLayout();
    for( const QPair<int, QString>& col : qAsConst(colInfo)) {
        if( col.second.isEmpty ()) continue;
        QCheckBox* cb =new QCheckBox(col.second);
        cb->setChecked (status[col.first]);
        checkboxes.push_back (cb);
        layout->addWidget (cb);
    }

    QGroupBox* gb =new QGroupBox();
    gb->setCheckable (false);
    gb->setLayout (layout);

    mainlayout->addWidget (gb);

    QPushButton* selectAll =new QPushButton(qsl("Alle auswählen"));
    connect( selectAll, &QPushButton::pressed, this, &dlgDisplayColumns::selectAll);
    mainlayout->addWidget (selectAll, Qt::AlignLeft);

    setLayout (mainlayout);

    QDialogButtonBox* buttons =new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    connect(buttons, &QDialogButtonBox::accepted, this, &dlgDisplayColumns::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &dlgDisplayColumns::reject);
    mainlayout->addWidget (buttons);
}

void dlgDisplayColumns::selectAll()
{
    for( const auto& box : qAsConst(checkboxes)) {
        box->setCheckState (Qt::CheckState::Checked);
    }
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
