#include "dlgchangecontracttermination.h"

#include "helper.h"
#include "helperfin.h"

dlgChangeContractTermination::dlgChangeContractTermination(QWidget *parent) : QDialog(parent)
{
    title =new QLabel(qsl("<h2>Verändern von Vertragsende und Kündigungsfrist<p></h2>"));
    setFontPointSize(title, 14);
    subTitle =new QLabel(qsl("Für das Vertragsende kann eine Kündigungsfrist <b>oder</b> "
                             "ein festes Vertragsende vereinbart werden.<p>"
                             "Bei einem festen Vertragsende wird üblicher Weise keine Kündigungsfrist "
                             "angegeben. Falls nötig kann dies jedoch mit dem Kontrollkästchen "
                             "erzwungen werden.<p>"));
    subTitle->setWordWrap(true);

    buttons =new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    connect(buttons, &QDialogButtonBox::accepted, this, &dlgChangeContractTermination::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &dlgChangeContractTermination::reject);

    QLabel* lKfrist =new QLabel(qsl("Kündigungsfrist"));

    notPeriod =new QComboBox();
    notPeriod->addItem(qsl("festes Vertragsende"), QVariant(-1));
    for (int i=3; i<12; i++)
        notPeriod->addItem(i2s(i) + qsl(" Monate"), QVariant(i));
    notPeriod->addItem(qsl("1 Jahr"), QVariant(12));
    notPeriod->addItem(qsl("1 Jahr und 1 Monat"), QVariant(13));
    for (int i=14; i<24; i++)
        notPeriod->addItem(qsl("1 Jahr und ") + i2s( i-12) + qsl(" Monate"), QVariant(i));
    notPeriod->addItem(qsl("2 Jahre"), QVariant(24));
    connect(notPeriod, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &dlgChangeContractTermination::onNoticePeriod_currentIndexChanged);
    notPeriod->setToolTip(qsl("Wird eine Kündigungsfrist vereinbart, "
                                   "so gibt es kein festes Vertragsende. "
                                   "Um ein festes Vertragsende einzugeben wähle "
                                   "in dieser Liste \"festes Vertragsende\""));

    QLabel* lContractEnd =new QLabel(qsl("Neues Vertragsende"));
    terminationDate =new QDateEdit();

    QLabel* lBoth =new QLabel(qsl("Kündigungsfrist <i>und</i> Vertragsende zulassen"));
    allowBoth =new QCheckBox();
    // TODO Change to checkStateChanged once Qt 6.9 is available on all targets.
    // https://doc.qt.io/qt-6/qcheckbox-obsolete.html
    connect(allowBoth, &QCheckBox::checkStateChanged, this, &dlgChangeContractTermination::onAllowBothChanged);

    QGridLayout* g =new QGridLayout(this);
    g->setColumnMinimumWidth(0, 30);
    g->setColumnMinimumWidth(3, 30);
    g->setRowMinimumHeight(0, 20);
    g->addWidget(title, 1,1, 1, 2);
    g->addWidget(subTitle, 2, 1, 1, 2);

    QHBoxLayout* hl =new QHBoxLayout();
    hl->addWidget(allowBoth);
    hl->addWidget(lBoth);
    g->addLayout(hl, 3, 1);

    g->addWidget(lKfrist, 4, 1);
    g->addWidget(notPeriod, 4, 2);
    g->addWidget(lContractEnd, 5, 1);
    g->addWidget(terminationDate, 5, 2);
    g->setRowMinimumHeight(6, 20);
    g->addWidget(buttons, 7, 2, 1, 2);

    g->setColumnStretch(1,1);
    g->setColumnStretch(2,4);
    setLayout(g);
}

void dlgChangeContractTermination::showEvent(QShowEvent* se)
{   LOG_CALL;
    if( se->spontaneous())
        return;
    centerDlg(qobject_cast<QWidget*>(parent()), this);
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

void dlgChangeContractTermination::onAllowBothChanged(Qt::CheckState i)
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
