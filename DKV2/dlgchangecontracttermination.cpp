

#include "helper.h"
#include "dlgchangecontracttermination.h"

dlgChangeContractTermination::dlgChangeContractTermination(QWidget *p) : QDialog(p)
{
    title =new QLabel(qsl("Verändern von Vertragsende und Kündigungsfrist<p>"));
    setFontPs(title, 14);

    subTitle =new QLabel(qsl("Für das Vertragsende kann eine Kündigungsfrist <b>oder</b> "
                             "ein festes Vertragsende vereinbart werden.<p>"
                             "Bei einem festen Vertragsende wird üblicher Weise keine Kündigungsfrist "
                             "angegeben. Falls nötig kann dies jedoch mit dem Kontrollkästchen "
                             "erzwungen werden.<p>"));
    subTitle->setWordWrap(true);
    setFontPs(subTitle, 10);

    buttons =new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    connect(buttons, &QDialogButtonBox::accepted, this, &dlgChangeContractTermination::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &dlgChangeContractTermination::reject);

    QLabel* lKfrist =new QLabel(qsl("Kündigungsfrist"));
    setFontPs(lKfrist, 10);

    notPeriod =new QComboBox();
    setFontPs(notPeriod, 10);
    notPeriod->addItem(qsl("festes Vertragsende"), QVariant(-1));
    for (int i=3; i<12; i++)
        notPeriod->addItem(QString::number(i) + qsl(" Monate"), QVariant(i));
    notPeriod->addItem(qsl("1 Jahr"), QVariant(12));
    notPeriod->addItem(qsl("1 Jahr und 1 Monat"), QVariant(13));
    for (int i=14; i<24; i++)
        notPeriod->addItem(qsl("1 Jahr und ") + QString::number( i-12) + qsl(" Monate"), QVariant(i));
    notPeriod->addItem(qsl("2 Jahre"), QVariant(24));
    connect(notPeriod, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &dlgChangeContractTermination::onNoticePeriod_currentIndexChanged);
    notPeriod->setToolTip(qsl("Wird eine Kündigungsfrist vereinbart, "
                                   "so gibt es kein festes Vertragsende. "
                                   "Um ein festes Vertragsende einzugeben wähle "
                                   "in dieser Liste \"festes Vertragsende\""));

    QLabel* lContractEnd =new QLabel(qsl("Neues Vertragsende"));
    setFontPs(lContractEnd, 10);

    terminationDate =new QDateEdit();
    setFontPs(terminationDate, 10);


    QLabel* lBoth =new QLabel(qsl("Kündigungsfrist <i>und</i> Vertragsende zulassen"));
    allowBoth =new QCheckBox();
    connect(allowBoth, &QCheckBox::stateChanged, this, &dlgChangeContractTermination::onAllowBothChanged);

    QGridLayout* g =new QGridLayout(this);
    g->addWidget(title, 1,0, 1, 2);
    g->addWidget(subTitle, 2, 0, 1, 2);

    QHBoxLayout* hl =new QHBoxLayout();
    hl->addWidget(allowBoth);
    hl->addWidget(lBoth);
    g->addLayout(hl, 3, 0);

    g->addWidget(lKfrist, 4, 0);
    g->addWidget(notPeriod, 4, 1);
    g->addWidget(lContractEnd, 5, 0);
    g->addWidget(terminationDate, 5, 1);
    g->addWidget(buttons, 6, 0);

    g->setColumnStretch(0,1);
    g->setColumnStretch(1,4);
    setLayout(g);
}

void dlgChangeContractTermination::setNoticePeriod(int i)
{
    if( i == -1) {
        notPeriod->setCurrentIndex(0);
        terminationDate->setEnabled(true);
    } else {
        notPeriod->setCurrentIndex(notPeriod->findData(i));
        terminationDate->setEnabled(false);
    }
}

void dlgChangeContractTermination::onNoticePeriod_currentIndexChanged(int i)
{
    if( allowBoth->isChecked())
        return;

    if(i == 0) {
        if( terminationDate->date() == EndOfTheFuckingWorld)
            terminationDate->setDate(QDate::currentDate().addYears(5));
        terminationDate->setEnabled(true);
    } else {
        terminationDate->setDate(EndOfTheFuckingWorld);
        terminationDate->setEnabled(false);
    }
}

void dlgChangeContractTermination::onAllowBothChanged(int i)
{
    if( i == Qt::Checked) {
        allowBoth->setChecked(true);
        if( terminationDate->date() == EndOfTheFuckingWorld)
            terminationDate->setDate(QDate::currentDate().addYears(5));
        terminationDate->setEnabled(true);
    } else {
        onNoticePeriod_currentIndexChanged(notPeriod->currentIndex());
    }
}

void dlgChangeContractTermination::accept()
{
    QDate end =terminationDate->date();
    if( minContractTerminationDate >= end) {
        QMessageBox::information(this, qsl("Ungültiges Datum"),
                     qsl("Das Vertragsende darf nicht vor der letzten Buchung bzw. dem Vertragsabschluß sein."));
        qInfo() << "Enddate is before Startdate: " << end.toString(Qt::ISODate)
                << "/" << minContractTerminationDate.toString(Qt::ISODate);
        return;
    }
    done(Accepted);
}
