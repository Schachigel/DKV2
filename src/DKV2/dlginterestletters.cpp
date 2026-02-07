#include "dlginterestletters.h"
#include "uihelper.h"
#include "helperfin.h"

void dlgInterestLetters::setYear(int y)
{
    year = y;
}

dlgInterestLetters::dlgInterestLetters(QWidget *parent, QVector<int> years) : QDialog(parent), years(years)
{
    QGridLayout* g =new QGridLayout();
    // get some space left and right
    g->setColumnMinimumWidth(0, 30);
    g->setColumnMinimumWidth(2, 30); // last col

    int row =0;
    g->setRowMinimumHeight(row++, 20);

    QLabel* title =new QLabel(qsl("<h2>Ausgabe Zinsbriefe</h2>"));
    g->addWidget(title, row++, 1);

    QString msgtxt =qsl("Bitte das Jahr für die Zinsbriefe wählen.");
    QLabel* msg =new QLabel(msgtxt);
    msg->setWordWrap(true);
    g->addWidget(msg, row++, 1);

    if (year == 0) {
        QDate::currentDate().getDate(&year, nullptr, nullptr);
    }

    QLabel *yearLabel = new QLabel(qsl("Jahr"));
    yearSelector = new QComboBox;
    for( const auto& aYear: std::as_const(years)) {
        yearSelector->addItem (i2s(aYear), aYear);
    }

    g->addWidget(yearLabel, row, 1);
    g->addWidget(yearSelector, row++, 2);

    csv = new QCheckBox(qsl("Zinsen auch als csv Datei ausgeben."));

    confirm =new QCheckBox(qsl("Briefe als PDF Dateien ausgeben."));
    g->addWidget(confirm, row++, 1);
    connect(confirm, &QCheckBox::checkStateChanged, this, &dlgInterestLetters::confirmChanged);

    buttons =new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
    connect(buttons, &QDialogButtonBox::accepted, this, &dlgInterestLetters::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &dlgInterestLetters::reject);
    g->setRowMinimumHeight(row++, 20);
    g->addWidget(buttons, row++, 1, 1, 2);

    setLayout(g);
}

void dlgInterestLetters::confirmChanged(Qt::CheckState state)
{
    if( state == Qt::Checked)
        buttons->button(QDialogButtonBox::Ok)->setEnabled(true);
    else
        buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
}
