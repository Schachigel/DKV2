
#include "helper.h"
#include "dlginterestletters.h"

void dlgInterestLetters::setYear(int y)
{
    year = y;
}

dlgInterestLetters::dlgInterestLetters(QWidget *parent, int currentYear) : QDialog(parent), year(currentYear)
{
    setFontPs(this, 10);

    QGridLayout* g =new QGridLayout();
    // get some space left and right
    g->setColumnMinimumWidth(0, 30);
    g->setColumnMinimumWidth(2, 30); // last col

    int row =0;
    g->setRowMinimumHeight(row++, 20);

    QLabel* title =new QLabel(qsl("Ausgabe Zinsbriefe"));
    setFontPs(title, 14);
    g->addWidget(title, row++, 1);

    QString msgtxt =qsl("Bitte das Jahr für die Zinsbriefe wählen.");
    QLabel* msg =new QLabel(msgtxt);
    msg->setWordWrap(true);
    g->addWidget(msg, row++, 1);

    if (year == 0) {
        QDate::currentDate().getDate(&year, nullptr, nullptr);
    }
    
    QLabel *yearLabel = new QLabel(qsl("Jahr"));
    yearSelector = new QSpinBox;
    yearSelector->setRange(year - 10, year + 3);
    yearSelector->setSingleStep(1);
    yearSelector->setValue(year);

    g->addWidget(yearLabel, row, 1);
    g->addWidget(yearSelector, row++, 2);

    csv = new QCheckBox(qsl("Zinsen auch als csv Datei ausgeben."));
    g->addWidget(csv, row++, 1);

    confirm =new QCheckBox(qsl("PDFs ausgeben."));
    g->addWidget(confirm, row++, 1);
    connect(confirm, &QCheckBox::stateChanged, this, &dlgInterestLetters::confirmChanged);

    buttons =new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
    connect(buttons, &QDialogButtonBox::accepted, this, &dlgInterestLetters::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &dlgInterestLetters::reject);
    g->setRowMinimumHeight(row++, 20);
    g->addWidget(buttons, row++, 1, 1, 2);

    setLayout(g);
}

void dlgInterestLetters::confirmChanged(int state)
{
    if( state == Qt::Checked)
        buttons->button(QDialogButtonBox::Ok)->setEnabled(true);
    else
        buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
}

