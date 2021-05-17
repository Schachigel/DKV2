#include <iso646.h>

#include "pch.h"

#include "helper.h"
#include "wizEditContractTermination.h"


const QString pnNewEDate {qsl("newEndDate")};
const QString pnNewPeriod{qsl("newNoticePeriod")};
const QString pnAllowBoth{qsl("allowBoth")}; // internal

wpEditContractTermination::wpEditContractTermination(QWidget* p) : QWizardPage(p)
{   LOG_CALL;
    setTitle(qsl("Verändern von Vertragsbeginn und -Ende"));
    setSubTitle(qsl("Für das Vertragsende kann eine Kündigungsfrist <b>und / oder</b> ein festes Vertragsende vereinbart werden."));

    cbNoticePeriod =new QComboBox;
    registerField(pnNewPeriod, cbNoticePeriod);
    cbNoticePeriod->addItem(qsl("festes Vertragsende"), QVariant(-1));
    for (int i=3; i<12; i++)
        cbNoticePeriod->addItem(QString::number(i) + qsl(" Monate"), QVariant(i));
    cbNoticePeriod->addItem(qsl("1 Jahr"), QVariant(12));
    cbNoticePeriod->addItem(qsl("1 Jahr und 1 Monat"), QVariant(13));
    for (int i=14; i<24; i++)
        cbNoticePeriod->addItem(qsl("1 Jahr und ") + QString::number( i-12) + qsl(" Monate"), QVariant(i));
    cbNoticePeriod->addItem(qsl("2 Jahre"), QVariant(24));

    connect(cbNoticePeriod, SIGNAL(currentIndexChanged(int)), this, SLOT(onNoticePeriod_currentIndexChanged(int)));

    cbNoticePeriod->setToolTip(qsl("Wird eine Kündigungsfrist vereinbart, "
                                   "so gibt es kein festes Vertragsende. "
                                   "Um ein festes Vertragsende einzugeben wähle "
                                   "in dieser Liste \"festes Vertragsende\""));
    QLabel* l2 =new QLabel(qsl("Kündigungsfrist"));
    l2->setBuddy(cbNoticePeriod);

    deTerminationDate =new QDateEdit;
    registerField(pnNewEDate, deTerminationDate);
    deTerminationDate->setDisplayFormat(qsl("dd.MM.yyyy"));
    QLabel* l3 =new QLabel(qsl("Neues Vertragsende"));
    l3->setBuddy(deTerminationDate);

    chkAllowBoth = new QCheckBox(qsl("Vertragsende <i>und</i> Kündigungsfrist zulassen"));

    QGridLayout* g =new QGridLayout;
    g->addWidget(l2, 1, 0);
    g->addWidget(cbNoticePeriod, 1, 1);
    g->addWidget(l3, 2, 0);
    g->addWidget(deTerminationDate, 2, 1);
    g->addWidget(chkAllowBoth, 3, 2);
    g->setColumnStretch(0, 1);
    g->setColumnStretch(1, 4);
    setLayout(g);
}
void wpEditContractTermination::initializePage()
{   LOG_CALL;
    setField(pnNewPeriod, 4);
    setField(pnNewEDate, EndOfTheFuckingWorld);
    setField(pnAllowBoth, false);
    deTerminationDate->setEnabled(false);
}

void wpEditContractTermination::onNoticePeriod_currentIndexChanged(int i)
{   LOG_CALL;
    if( field(pnAllowBoth).toBool())
        return;
    if( i == 0) {
        // deTerminationDate->setDate(QDate::currentDate().addYears(5));
        setField(pnNewEDate, QDate::currentDate().addYears(5));
        deTerminationDate->setEnabled(true);
    } else {
        //deTerminationDate->setDate(EndOfTheFuckingWorld);
        setField(pnNewEDate, EndOfTheFuckingWorld);
        deTerminationDate->setEnabled(false);
    }
}
bool wpEditContractTermination::validatePage()
{   LOG_CALL;
    wizEditContractTermination* wiz =qobject_cast<wizEditContractTermination*> (wizard());
    Q_ASSERT(wiz);
    QDate end   =field(pnNewEDate).toDate();
    if( wiz->minContractTermination >= end) {
        qInfo() << "Enddate is before Startdate: " << wiz->minContractTermination.toString(Qt::ISODate) << "/" << field(pnNewEDate).toDate().toString(Qt::ISODate);
        return false;
    }
    wiz->newNoticePeriod =cbNoticePeriod->itemData(field(pnNewPeriod).toInt()).toInt();
    return true;
}

wizEditContractTermination::wizEditContractTermination(QWidget* p) : QWizard(p)
{
    addPage(new wpEditContractTermination);
}
